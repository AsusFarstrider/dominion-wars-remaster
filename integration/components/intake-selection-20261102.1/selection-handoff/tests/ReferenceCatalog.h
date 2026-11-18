#pragma once
// Offline comparison authority only. Not included by any production source or
// packaged runtime asset. The owner's private old catalog stays external.
#define SelectionRestoration LegacySelectionRestoration
#include "E:/Codex/research/dominion-rendering/briefing-escape-source-20261101-02/DDrawCompat/Remaster/SelectionCatalog.h"
#undef SelectionRestoration
inline std::shared_ptr<LegacySelectionRestoration::Catalog> referenceCatalog() {
    static auto authority=[] {
        std::ifstream f("E:/Codex/research/dominion-rendering/mission-select-assets-20261101-04/selection.dwselect",std::ios::binary);
        auto c=LegacySelectionRestoration::Catalog::parse({std::istreambuf_iterator<char>(f),{}});
        if(!c)throw std::runtime_error("external private test reference missing");return c;
    }();return authority;
}
inline const std::vector<uint8_t>& reference(const SelectionRestoration::Entry& e) {
    const auto* old=referenceCatalog()->get(e.kind,e.key);
    if(!old)throw std::runtime_error("reference entry missing");return old->original;
}
