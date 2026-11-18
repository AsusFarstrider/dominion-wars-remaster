# Selection catalog without original identity rasters

2026-11-02. New disposable child of accepted selection03 plus the paused-briefing/confirmation repair. No game launch, UI automation, Current adoption, version advancement or publication was performed.

`DWSSEL2` replaces each original P8 identity raster with a SHA-256 fingerprint and a 256-bit nonzero glyph-class mask. Live owner bytes are still read and content-checked; dimensions or resource IDs alone do not admit replacements. Original logical metrics, caller/owner checks, state selection, font style/header/full-face checks, palette ramps, ordered paint, resource lifetime and original fallback remain. No cache of unverifiable pointer identities is introduced. Existing Windows BCrypt performs hashing; the wrapper already linked bcrypt.lib. No image decoder or new license dependency is added.

The converted catalog contains114 descriptors, fingerprints/class masks and unchanged replacement payloads. It removes593,038 identity pixels, adds7,296 bytes of digest/mask metadata and shrinks7548682→6962940 bytes. This resolves the **original reference-pixel storage** gate structurally. It does not independently establish permission to redistribute the supplied replacement artwork or other release content.

## Integration contract

`runtime-inventory.json` supplies every affected target path, file size and SHA256, flags/defaults, base/dependencies and configuration requirements. `format.json` describes the binary fields and their no-reference-raster contract. Master should use these through the ordinary locked intake/package path, rather than adopting a manually rebound test install. No new test installation was created.

The six runtime files are ddraw.dll and selection.dwselect, selection.dwcard, cursor.dwcur, selection.flags and cursor.flags under rs/Remaster/Selection. Catalog magic is now DWSSEL2; old DWSSEL1 is deliberately unsupported by this child and fails to original artwork. Catalog/compiled hash pin must be adopted together. Default selection.flags=15 retains map/body/font/faction features; any bit combination0..15 is supported,0 or absence keeps original selection art at process startup. Independent cursor.flags=1 is unchanged. Removal restores parent wrapper/catalog and their ordinary manifest binding, or disables optional flags before starting. Hot reload is not a supported interface.

Three production headers change: SelectionCatalog.h, SelectionRuntime.h and SelectionAssetPins.h. All472 source pins are in source.json; the full tree is E:/Codex/research/dominion-rendering/selection-reference-free-source-20261102-01. Parent receipt is E:/Codex/research/dominion-rendering/briefing-escape-candidate-20261101-02/source.json, hash aba815d5642ae852f825c770c17af2f14d6ec1b0bfc464dd6f26c0134a2a11b2. The accepted OpeningFlow.cpp remains exactly2ce80819adf65a86ea6d1a6b483824a7766583de355995b10f2a6140d078d8ff. Apply the included three files/diff to that parent; build DDrawCompat.sln Release/x86 with Visual Studio18/v145, SDK10.0.26100.0 and inherited WDK headers. Final wrapper41653c0f7c3961cc0b5fd8eb6b67374fbe23ad807c5f17c370fb9ab15aacaba4; catalog7e1b54d2fb728ff2eb76414493407886a04374d89d045b9b4aa050ad98d0ebe1.

## Conversion and validation

Standard-library tools require no decoder:

```powershell
python tools/convert_catalog.py --legacy-catalog OWNER_PRIVATE/selection.dwselect --output NEW_OUTPUT
python tools/validate_catalog.py NEW_OUTPUT/selection.dwselect --receipt NEW_RECEIPT.json
```

This deliberately reuses the existing pinned private authoring output. The legacy catalog remains an owner-local conversion input; never put it in the runtime/public asset inventory. The conversion copies all replacement pixels byte-for-byte and emits descriptors, SHA256 and class masks, with a receipt. Independent validator rejects unsupported magic, geometry, count, duplicate/trailing/truncated records, invalid classes and payloads. C++ performs the same bounded admission plus compiled whole-file identity before publishing optional assets. Missing/malformed/unsupported files or mismatched live owner content retain the original path.

For tests, build tests/SelectionTests.vcxproj with WrapperSourceInclude pointing to this child's DDrawCompat; build CanvasRuntimeTests.vcxproj with that plus CanvasContractInclude pointing to DDrawCompat/Research/MenuCanvas, Release/Win32, separate OutDir/IntDir. bcrypt.lib is linked. Run SelectionTests with the new asset folder argument. Controller install tests require a fresh output folder because they create synthetic adjacent rs/Remaster/Selection files. The external owner-private V1 catalog/source header is test authority in ReferenceCatalog.h only; it is neither a production dependency nor copied into this handoff. Adapt its paths to a verified local parent if moving the QA environment. `tools/check_conversion.py` also uses the recorded private authority paths. None of these tools launches a game.

## Results and limits

-5532 catalog/color/composition/adaptive/cursor assertions,628 actual-controller memory/COM/blit/install assertions and369 independent conversion/format assertions passed. Full wrapper:0 warnings,0 errors.
-All114 logical/replacement descriptors and every replacement payload match the parent. All96 glyphs were checked under four synthetic RGB565 ramps with fractional alpha. An ordered scene containing all113 replacement entries was rendered through old and new catalogs; **every final RGB565 byte and padded row byte matched** at1024×768,1280×800,1366×768,1920×1080,1920×1200,2560×1440,3840×2160 and3440×1440. Per-output hashes are in validation.json/evidence. This is exact offscreen equivalence, not live-screen/hardware qualification.
-Actual controller tests retain caller/style/metric/content refusal, font joins, body/faction state selection, original raw fallback and existing cursor lifetime behavior. Startup tests cover absent/disabled/invalid flags, absent/old/malformed/tampered catalog and missing map before valid publication.
-Measured hashing on this workstation:480000-byte map≈261us; one hash of every114 identities/593038 bytes≈356us. No identity copy is retained in the catalog. Each call is bounded by800×600 bytes; font validation retains its existing once-per-layer behavior, and there is no new pointer cache. These timings exclude owner memory reads, allocations/composition and do not claim game FPS. Body state searches can hash the same source up to three times; the previous bounded search structure is preserved.
-No live test or new hardware/reset qualification was performed. Existing approved1.15 user-reported debrief/objective/hover-label issues are separate new work and are not claimed fixed by this catalog change.

Master owns acceptance and release decisions. Texture authoring can keep the exact existing artwork/metrics and add this final conversion/validation step. No original BOLT loader, font layout or artwork redraw is required.
