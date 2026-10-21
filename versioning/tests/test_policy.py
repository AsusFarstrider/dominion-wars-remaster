import copy
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from check_policy import remaster_number, validate, validate_identity

REGISTRY = json.loads((Path(__file__).resolve().parents[1] / "registry.json").read_text())


class VersionPolicyTests(unittest.TestCase):
    def test_current_legacy_registry(self):
        self.assertTrue(validate(REGISTRY))

    def test_two_digit_string_not_float(self):
        self.assertEqual(remaster_number("1.10"), (1, 10))
        for value in (1.1, "1.1", "1.2", "01.10", "1.100", "1.10-alpha", None):
            with self.assertRaises(ValueError):
                remaster_number(value)

    def test_clarified_continuation_and_skipped_milestones(self):
        labels = ["1.05", "1.06", "1.07", "1.08", "1.09", "1.10", "1.11", "1.25", "1.54", "1.75", "1.90", "2.00"]
        numbers = [remaster_number(value) for value in labels]
        self.assertEqual(numbers[0], (1, 5))
        self.assertEqual(numbers[-1], (2, 0))
        self.assertEqual(numbers, sorted(numbers))
        self.assertEqual(labels[5], "1.10")

    def test_sequence_or_percentage_rule_refused(self):
        for field, value in (("two_digit_presentation_required", False), ("sequential_advancement_required", True),
                             ("not_completion_percentage", False)):
            registry = copy.deepcopy(REGISTRY)
            registry["numbering"][field] = value
            with self.assertRaises(ValueError):
                validate(registry)

    def test_official_number_is_not_remaster_number(self):
        for value in ("1.04", "1.00", "0.10"):
            with self.assertRaises(ValueError):
                remaster_number(value)

    def test_automatic_bump_refused(self):
        value = copy.deepcopy(REGISTRY)
        value["numbering"]["automatic_advancement"] = True
        with self.assertRaises(ValueError):
            validate(value)

    def test_retroactive_legacy_relabel_refused(self):
        value = copy.deepcopy(REGISTRY)
        value["current_baseline"]["in_game_label"] = "Remastered 1.10"
        with self.assertRaises(ValueError):
            validate(value)

    def versioned(self):
        value = copy.deepcopy(REGISTRY)
        value["current_baseline"].update(remaster_version="1.10", in_game_label="Remastered 1.10",
                                         version_identity_status="master-approved-remaster-version",
                                         display_binding="validated-shared-version-record")
        value["milestone_reviews"].append(dict(decision="advance", assigned_remaster_version="1.10",
                                                reviewed_build_id=value["current_baseline"]["build_id"],
                                                evidence=["synthetic test evidence, not real approval"],
                                                rationale="synthetic milestone", significance="validated-integrated-milestone",
                                                approved_by="Master Integration"))
        return value

    def test_evidence_backed_prospective_version(self):
        self.assertTrue(validate(self.versioned()))

    def test_proposed_number_without_decision_refused(self):
        value = self.versioned()
        value["milestone_reviews"].pop()
        with self.assertRaises(ValueError):
            validate(value)

    def test_routine_changes_not_milestones(self):
        value = self.versioned()
        value["milestone_reviews"][-1]["significance"] = "routine-fix"
        with self.assertRaises(ValueError):
            validate(value)

    def test_200_requires_goal_assessment(self):
        value = self.versioned()
        value["current_baseline"].update(remaster_version="2.00", in_game_label="Remastered 2.00")
        value["milestone_reviews"][-1]["assigned_remaster_version"] = "2.00"
        with self.assertRaises(ValueError):
            validate(value)
        value["standing_goal"]["status"] = "achieved"
        with self.assertRaises(ValueError):
            validate(value)
        value["milestone_reviews"][-1]["goal_assessment"] = {k: "synthetic assessment, not real approval" for k in value["standing_goal"]["evidence_categories"]}
        self.assertTrue(validate(value))

    def test_missing_goal_category_refused(self):
        value = self.versioned()
        value["current_baseline"].update(remaster_version="2.00", in_game_label="Remastered 2.00")
        value["standing_goal"]["status"] = "achieved"
        value["milestone_reviews"][-1].update(assigned_remaster_version="2.00", goal_assessment={"one category": "pass"})
        with self.assertRaises(ValueError):
            validate(value)

    def identity(self):
        c = REGISTRY["current_baseline"]
        return dict(schema=1, product="Dominion Wars Remastered", original_game_version="1.04",
                    remaster_version=None, release_stage=c["release_stage"], integration_build_id=c["build_id"],
                    source_revision=c["source_revision"], flavor=c["flavor"],
                    version_decision_id="synthetic-legacy-example-not-shipped", in_game_label=c["in_game_label"])

    def test_prospective_identity_contract(self):
        identity = self.identity()
        self.assertTrue(validate(REGISTRY, identity=identity))
        identity.update(remaster_version="1.10", in_game_label="Remastered 1.10")
        self.assertTrue(validate_identity(identity))
        with self.assertRaises(ValueError):
            validate(REGISTRY, identity=identity)

    def test_identity_conflicting_label(self):
        identity = self.identity()
        identity.update(remaster_version="1.10", in_game_label="Remastered 2.00")
        with self.assertRaises(ValueError):
            validate_identity(identity)

    def test_unknown_identity_data_or_stage_refused(self):
        identity = self.identity()
        identity["wrapper_version"] = "2.00"
        with self.assertRaises(ValueError):
            validate_identity(identity)
        identity = self.identity()
        identity["release_stage"] = "invented"
        with self.assertRaises(ValueError):
            validate_identity(identity)

    def test_boolean_schema_refused(self):
        identity = self.identity()
        identity["schema"] = True
        with self.assertRaises(ValueError):
            validate_identity(identity)

    def test_identity_binding_to_manifest(self):
        identity = self.identity()
        manifest = dict(build_id=identity["integration_build_id"], source_revision=identity["source_revision"], flavor=identity["flavor"])
        manifest.update({k: identity[k] for k in ("remaster_version", "release_stage", "in_game_label", "version_decision_id")})
        self.assertTrue(validate_identity(identity, manifest))
        manifest["source_revision"] = "0" * 40
        with self.assertRaises(ValueError):
            validate_identity(identity, manifest)

    def test_current_hash_drift_refused(self):
        c = REGISTRY["current_baseline"]
        record = {k: c[k] for k in ("build_id", "source_revision", "package_sha256", "manifest_sha256")}
        record["status"] = "approved-internal-baseline"
        self.assertTrue(validate(REGISTRY, current_record=record))
        record["package_sha256"] = "0" * 64
        with self.assertRaises(ValueError):
            validate(REGISTRY, current_record=record)

    def test_legacy_manifest_does_not_invent_new_version(self):
        c = REGISTRY["current_baseline"]
        manifest = dict(build_id=c["build_id"], source_revision=c["source_revision"], flavor=c["flavor"],
                        engine=dict(input_sha256=REGISTRY["official_original"]["unmodified_executable_sha256"],
                                    output_sha256=c["generated_executable_sha256"]))
        self.assertTrue(validate(REGISTRY, manifest=manifest))
        manifest["remaster_version"] = "2.00"
        with self.assertRaises(ValueError):
            validate(REGISTRY, manifest=manifest)


if __name__ == "__main__":
    unittest.main()
