# Main foreground coverage-loss correction

2026-10-27. Follow-up to `MAIN-BACKGROUND-SOURCE-REVIEW-01.md`; the original finding and snapshots remain preserved. **The specific coverage-loss blocker is corrected in the reviewed source.** No additional blocker was identified in this narrow follow-up. This is not candidate-DLL or live-test acceptance.

Observed corrections: owner-thread coverage loss now invalidates the shared weak-current layer, releases `mainOwns`, and revokes OpeningFlow's epoch/pass/publication as well as Canvas publication. Retirement works when the inner active pass has already ended. Foreign callers set an atomic pending flag; owner-thread draw/input and post-Unlock publication paths drain it before committing. Ordinary causal Main-to-modal handoff still releases capture ownership without invalidating its valid foreground history.

The actual controller test source contains new cases for an active gap after background/glyph capture, an outer layer after inner retirement, and deferred foreign-thread invalidation. Engine inspected these cases but did not compile or execute them. Rendering reports compilation/testing underway and owns the subsequent build, freeze and live combined test. No further reconstruction is requested.

Four exact follow-up source snapshots and hashes: `evidence/main-background-source-review-02/manifest.json`. Engine changed no specialist code, executable or installation and launched no experiment.
