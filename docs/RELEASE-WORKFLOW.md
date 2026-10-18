# Integration to experimental release

Master integrates; Release documents/audits/packages; the user authorizes release.
An internal baseline is not automatically a public download. No independent merge
of specialist artifacts, replacement build or runtime change is allowed here.

For every release, record exact integration ID/source commit, selected original
inputs, package/schema1 manifest hashes, component/license inventory, reproduction
recipe, tests/evidence, hardware coverage, known limitations and explicit approval.
The private handoff manifest is only an index, not a competing package schema.

1. Verify the selected approved package and committed source.
2. Audit selected source AND all Git history, plus every package member. Exclude
   commercial assets/media/updaters, private artwork, disposable installs, secrets,
   personal evidence and inappropriate artifacts. Do not delete research resources.
3. Resolve license/distribution issues and preserve required notices/source.
4. Check build reproduction and package-bound install/launch/save-load/rollback
   evidence. Request any functional/package change from Master; changed outputs
   need a new identified candidate and appropriate validation/approval.
5. Prepare truthful player docs, release notes, checksums and a publication record
   mapping the exact release/tag to the integration revision and approved ZIP.
6. Publish only with user authorization; mark the first build experimental and
   prerelease. Never silently replace a release asset with different bytes.

Use the existing shared history and an agreed remaster remote, not another source
repository assembled from experiments. Keep versioning simple. The first prerelease
tag is the integration ID int-20261018.1, not an unrelated alpha version. Later
public version mappings must remain explicit.

Publication preparation for the first release targets int-20261018.1 original-assets,
source 921826392c5b6f5acdc523f4db2429cbad234782, ZIP SHA256
7633f657e8c5845d083c2d6cf6a871b8360155b7f8d7bc1733e839134392c864.
Its frozen manifest's historical approval fields are supplemented by later records;
do not edit that manifest and thereby change the approved ZIP.
