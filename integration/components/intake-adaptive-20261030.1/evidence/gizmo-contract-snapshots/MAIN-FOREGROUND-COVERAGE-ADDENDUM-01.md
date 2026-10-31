# Main foreground coverage: existing solid and glyph semantics

2026-10-27. Bounded addendum requested by Rendering for the shared MainBackdrop design. Existing4A1710/49F050 and font contracts only; no implementation, new export, codec/game/build or unrelated reconstruction. Main/Gizmo contract remains GIZMO-AND-MAIN-NATIVE-CONTRACT-01.

| Admitted raw8 source | Actual write / coverage rule |
|---|---|
|AND=FF,XOR=0,flags1|Opaque indexed image: every actual clipped write, including index0, owns coverage. Convert through the then-live RGB565 table.|
|AND=FF,XOR=0,flags3|Usual button/glyph: original index0 does not write. Every nonzero source index owns coverage, even if its RGB565 color is black.|
|AND=0,transparency bit2 clear (flags0/1)|Constant-index fill: every effective pixel gets table_u16[XOR]. **No source-pixel read is needed.** Include black/index0 as opaque coverage.|
|AND=0,transparency bit2 set (flags2/3)|Constant-colored glyph/mask: read original source indices; nonzero indices write table_u16[XOR],zero leaves the destination. Coverage is the original glyph shape.|

These are distinct branches in49F050. Do not require a valid pixel array for a nontransparent AND0 solid, or turn its XOR0 into a transparent color key. Conversely, AND0 with transparency enabled does require valid raw source bytes; the constant output color does not make its whole rectangle opaque.

4A1710 constructs a24-byte stack descriptor with flags0,tagFF,AND0,XOR=its color argument,bits8,origin/dimensions from the destination,then calls49F050. Pixel pointer/surface/pitch fields are inactive/uninitialized. Read active fields only and do not dereference imaginary source storage. The known Main native clear is this family but is explicitly excluded from foreground by its owner/caller/full-native geometry witness. A validated local/modal fill is actual foreground and must not be excluded merely because it shares flags0/AND0.

**Flags0 does not activate source-side clipping.** GlobalA39B78 only takes precedence when source flags&1 is set; then it replaces the outer wrapper clip rather than being implicitly intersected with it. Origin arithmetic and pixel spans still need bounded validation. For a private800x600 shadow, require a contained safe fill rectangle or an explicitly reviewed private clipping descriptor. Never send a native-sized unclipped fill into logical storage. Unknown native-sized modal/background operations need a distinct supported role or safe fallback, not a guessed logical rectangle.

Use the original trampoline immediately to generate colors, preserving scratch/global clip/source pitch. In RGB565, the explicit AND0 branch uses live4EC390[XOR],not a replicated raw color byte. Glyph zero transparency precedes any color transform/lookup. A font shadow can therefore be both AND0 and genuinely shape-masked; its opaque black pixels must survive native-background composition.

General indexed mask/XOR paths express `(original_index & AND) ^ XOR` after the original zero-key decision, but **do not generalize that formula into universal16-bit support**. The existing helper includes byte-oriented general-mask branches and alternative codecs. The parent deliberately limits its ordinary Main shadow to rawP8 flags1/3 ANDFF/XOR0. The bounded extension can add the explicit typed AND0 branches above; other masks/effects/codecs/tagged/GPU sources remain unsupported until separately witnessed. No new decoding campaign is necessary for this candidate.

Coverage remains visual only; it does not change original rectangular button admission. Capture it across the existing pure Main/outer-modal ownership handoff and retain original operation order/generations. Pixel differences or black-key inference remain invalid substitutes for these write rules.

Source pins/byte verification: `evidence/main-foreground-coverage-01/audit.json`; script `scripts/audit_main_foreground_coverage.py`; manifest `validation/main-foreground-coverage-01-manifest.json`. Related existing details: FONT-BLIT-CONTRACT-02 and MAIN-MENU-CANVAS-CONTRACT-01.
