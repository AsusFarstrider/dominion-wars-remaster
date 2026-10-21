"""Read-only version policy and selected-baseline metadata checks (standard library)."""
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STAGES = ("experimental", "alpha", "beta", "release-candidate", "stable")


def require(condition, message):
    if not condition:
        raise ValueError(message)


def remaster_number(value):
    require(isinstance(value, str) and re.fullmatch(r"(?:0|[1-9][0-9]*)\.[0-9]{2}", value),
            "Remaster version must be a major.two-digit-milestone string")
    number = tuple(map(int, value.split(".")))
    require(number > (1, 4), "Do not conflate remaster versions with official 1.04 or earlier")
    return number


def sha(value, label):
    require(isinstance(value, str) and re.fullmatch(r"[a-f0-9]{64}", value), label + " must be SHA256")


def validate_identity(identity, manifest=None):
    fields = {"schema", "product", "original_game_version", "remaster_version", "release_stage",
              "integration_build_id", "source_revision", "flavor", "version_decision_id", "in_game_label"}
    require(isinstance(identity, dict) and set(identity) == fields, "Identity fields missing or unexpected")
    require(type(identity["schema"]) is int and identity["schema"] == 1 and identity["product"] == "Dominion Wars Remastered", "Unsupported identity schema/product")
    require(identity["original_game_version"] == "1.04", "Identity original-game version mismatch")
    require(identity["release_stage"] in STAGES, "Unsupported identity stage")
    require(isinstance(identity["integration_build_id"], str) and re.fullmatch(r"int-[0-9]{8}\.[1-9][0-9]*(?:-[a-z0-9-]+)?", identity["integration_build_id"]),
            "Identity integration build ID invalid")
    require(isinstance(identity["source_revision"], str) and re.fullmatch(r"[a-f0-9]{40}", identity["source_revision"]), "Identity source revision invalid")
    for field in ("flavor", "version_decision_id"):
        require(isinstance(identity[field], str) and bool(identity[field].strip()), "Identity requires " + field)
    version = identity["remaster_version"]
    if version is not None:
        remaster_number(version)
    expected = "Version 1.04" if version is None else "Remastered " + version
    require(identity["in_game_label"] == expected and len(expected) <= 80, "Identity display/version disagreement")
    if manifest is not None:
        for field, key in (("integration_build_id", "build_id"), ("source_revision", "source_revision"), ("flavor", "flavor")):
            require(identity[field] == manifest[key], "Identity/manifest disagreement: " + field)
        for field in ("remaster_version", "release_stage", "in_game_label", "version_decision_id"):
            require(field in manifest and identity[field] == manifest[field], "Identity/manifest disagreement: " + field)
    return True


def validate(registry, manifest=None, current_record=None, identity=None):
    require(type(registry.get("schema")) is int and registry.get("schema") == 1, "Unsupported registry schema")
    require(registry.get("owner") == "Master Integration", "Master owns version decisions")
    official = registry["official_original"]
    require(official["version"] == "1.04", "Official base must remain distinct at 1.04")
    sha(official["unmodified_executable_sha256"], "Original executable hash")
    goal = registry["standing_goal"]
    require(goal["version"] == "2.00" and goal["decision_owner"] == "Master Integration",
            "Standing 2.00 milestone is Master-owned")
    require(goal["status"] in ("not-achieved", "achieved"), "Unknown goal status")
    require(registry["numbering"]["automatic_advancement"] is False, "Automatic advancement is prohibited")
    require(registry["numbering"]["stage_is_separate"] is True, "Stage must be separate from version")
    current = registry["current_baseline"]
    require(current["release_stage"] in STAGES, "Unsupported release stage")
    for field in ("package_sha256", "manifest_sha256", "generated_executable_sha256"):
        sha(current[field], field)
    require(re.fullmatch(r"[a-f0-9]{40}", current["source_revision"]), "Source revision must be identified")
    version = current["remaster_version"]
    if version is None:
        require(current["version_identity_status"] == "legacy-release-with-no-separate-numeric-remaster-version",
                "Only explicit legacy releases may omit the remaster version")
        require(current["display_binding"] == "legacy-original-label" and current["in_game_label"] == "Version 1.04",
                "Do not invent a numeric remaster label for the historical package")
    else:
        remaster_number(version)
        require(current["version_identity_status"] == "master-approved-remaster-version", "Version must be Master-approved")
        require(current["display_binding"] == "validated-shared-version-record", "Display binding must be validated")
        require(current["in_game_label"] == "Remastered " + version, "Game label and approved version disagree")
        decisions = [d for d in registry["milestone_reviews"]
                     if d.get("decision") == "advance" and d.get("assigned_remaster_version") == version
                     and d.get("reviewed_build_id") == current["build_id"]]
        require(len(decisions) == 1 and decisions[0].get("evidence") and decisions[0].get("rationale"),
                "Version requires one evidence-backed milestone decision for this build")
        require(decisions[0].get("significance") == "validated-integrated-milestone", "Routine or research work does not advance versions")
        require(decisions[0].get("approved_by") == "Master Integration", "Only Master approves version advancement")
    if goal["status"] == "achieved":
        earned = [d for d in registry["milestone_reviews"] if d.get("decision") == "advance"
                  and d.get("assigned_remaster_version") == "2.00" and d.get("goal_assessment")]
        require(len(earned) == 1 and earned[0].get("evidence"), "2.00 needs an explicit evidence-backed goal assessment")
        require(earned[0].get("approved_by") == "Master Integration" and earned[0].get("significance") == "validated-integrated-milestone",
                "The2.00 goal requires Master's validated milestone decision")
        assessment = earned[0]["goal_assessment"]
        require(isinstance(assessment, dict) and set(assessment) == set(goal["evidence_categories"])
                and all(isinstance(v, str) and v.strip() for v in assessment.values()),
                "The2.00 assessment must address every evidence category without requiring perfection")
    elif version is not None:
        require(remaster_number(version) < (2, 0), "2.00 or later requires an earned goal record")
    if manifest is not None:
        require(manifest["build_id"] == current["build_id"], "Manifest build ID mismatch")
        require(manifest["source_revision"] == current["source_revision"], "Manifest source revision mismatch")
        require(manifest["flavor"] == current["flavor"], "Manifest flavor mismatch")
        require(manifest["engine"]["input_sha256"] == official["unmodified_executable_sha256"], "Original input hash mismatch")
        require(manifest["engine"]["output_sha256"] == current["generated_executable_sha256"], "Generated executable hash mismatch")
        require(manifest.get("remaster_version") == version, "Manifest remaster version mismatch")
        if version is not None:
            require(manifest.get("release_stage") == current["release_stage"], "Manifest stage mismatch")
            require(manifest.get("in_game_label") == current["in_game_label"], "Manifest display label mismatch")
    if current_record is not None:
        require(current_record["status"] == "approved-internal-baseline", "Current is not an approved baseline")
        for field in ("build_id", "source_revision", "package_sha256", "manifest_sha256"):
            require(current_record[field] == current[field], "Current record mismatch: " + field)
    if identity is not None:
        validate_identity(identity, manifest)
        for field, key in (("integration_build_id", "build_id"), ("source_revision", "source_revision"), ("flavor", "flavor"),
                           ("remaster_version", "remaster_version"), ("release_stage", "release_stage"), ("in_game_label", "in_game_label")):
            require(identity[field] == current[key], "Identity/registry disagreement: " + field)
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--registry", type=Path, default=ROOT / "versioning/registry.json")
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--current-record", type=Path)
    parser.add_argument("--identity", type=Path)
    args = parser.parse_args()
    def read(path):
        return json.loads(path.read_text(encoding="utf-8-sig")) if path else None
    try:
        validate(read(args.registry), read(args.manifest), read(args.current_record), read(args.identity))
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(1, "Version policy check failed: " + str(error) + "\n")
    print("Version policy and supplied metadata agree; no version or runtime was changed.")


if __name__ == "__main__":
    main()
