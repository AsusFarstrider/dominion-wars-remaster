# Paramount1440 history: independent offline confirmation

2026-10-26. Addendum to PARAMOUNT-1440-NATIVE-COPY-DESIGN-01; Movies/Textures supplied a stronger, separate receipt. The earlier same-handle/two-copy test and frozen package remain preserved. This changes the offline evidence status, not the proposed runtime design or live acceptance.

The new test opens **two independent decoder handles, with one Copy per decoded frame per handle**. One path initializes with8000000A then uses selective0A; the other uses full8000000A throughout. Both use the exact original shipped DLL,2560x1440 public/internal dimensions, privateXY0, pitch5120 and7,372,800-byte guarded RGB565 buffers. All225 frames compared equal, skipped0, guards intact, exit0. Engine read the test source and verified receipt/source/runner/asset/DLL identities; it did not execute the codec.

This removes the earlier comparator's shared-handle/two-copy confound for the tested video/conversion path. It supports persistent source history with full initialization followed by selective updates. It does not test audio output, BinkWait pacing, late-audio skipped conversion, QUANTUM Lock/Unlock/presentation, input, reset or lifecycle behavior. The first-copy result1 handling and safe HD refusal contract remain required implementation checks. Live1440 playback is still pending Rendering's safe candidate.

Asset SHA256 remains `5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3`; shipped DLL `65358b3cde7281df10cb68ac8e1c9f8705893a47df33f87bb882073fd0955c90`. Independent source SHA256 `ed25fd1b363f4d752bde004b0991b50752f01f5dc32b1559d6ff23e4675ecdd6`; runner `78d18bd281fe73b8cdfb9bb582f7359217c807bac52f2bb610fdffe4967270fc`.

Attribution/source: `paramount-restoration/private-1440-v1/history-independent-receipt.json` and `paramount-restoration/history_check_independent.cpp`. Separate KB receipt: `evidence/paramount-1440-history-result-02`; validation: `validation/paramount-1440-history-result-02-manifest.json`. Rendering remains the sole implementation/build/install/launch owner. No game, build, Current or accepted-candidate changes by Engine.
