#pragma once
#include "FleetArtwork.h"

namespace FleetCaptains {

using MainBackdrop::read;
using MainBackdrop::value;
using MainBackdrop::write;
using Widget = std::array<uint8_t, 0x90>;
constexpr uint32_t root = 0x4E3378;
constexpr uint32_t templateCell = 0x4E3C78;
constexpr uint32_t leftArrow = 0x4E4068;
constexpr uint32_t rightArrow = 0x4E40F8;
constexpr uint32_t hidden = 8, selected = 0x100000;
constexpr uint32_t firstExtra = FleetArtwork::Layout::seventhCaptain;
constexpr uint32_t originalArrow = 0x46CC40;
constexpr uint32_t originalRefresh = 0x46DE50;

inline bool enabled = false;
inline unsigned inputCells = 6;
alignas(4) inline std::array<Widget, 2> widgets{};
inline thread_local uint32_t discardDestination = 0;
inline thread_local bool observedExtra = false;

inline uint32_t address(const Widget& widget) noexcept {
    return uint32_t(reinterpret_cast<uintptr_t>(widget.data()));
}

inline bool owns(uint32_t node) noexcept {
    return node == address(widgets[0]) || node == address(widgets[1]);
}

template<class T> void set(Widget& widget, size_t offset, T v) noexcept {
    std::memcpy(widget.data() + offset, &v, sizeof(v));
}

template<class T> T get(const Widget& widget, size_t offset) noexcept {
    T v{};
    std::memcpy(&v, widget.data() + offset, sizeof(v));
    return v;
}

struct Roster {
    int32_t count = 0;
    int32_t start = 0;

    int last(unsigned cells) const noexcept { return std::max(0, count - int(cells)); }
    int scroll(unsigned cells, bool forward) const noexcept {
        constexpr int step = 5;
        return std::clamp(start + (forward ? step : -step), 0, last(cells));
    }
    int entry(unsigned cell) const noexcept { return start + int(cell); }
    bool contains(int entryIndex) const noexcept { return entryIndex >= 0 && entryIndex < count; }
};

inline bool roster(Roster& list) noexcept {
    return read(0x75F0C0, &list.count, sizeof(list.count)) &&
        read(0x760708, &list.start, sizeof(list.start)) &&
        list.count >= 0 && list.count <= 96 && list.start >= 0 && list.start <= list.last(6);
}

struct Captain { uint32_t id, detail, unavailable, level, assignment; };
inline uint32_t status(const Captain& captain) noexcept {
    if (captain.unavailable) return 4;
    if (!captain.assignment) return 2;
    return captain.level ? 0 : 1;
}

inline void hide() noexcept {
    for (auto& widget : widgets) set(widget, 4, get<uint32_t>(widget, 4) | hidden);
}

inline bool catalogMode() noexcept {
    return enabled && FleetArtwork::admitted() && value<uint8_t>(0x7606F8) == 1;
}

inline unsigned update(unsigned capacity, bool mayAttach);

inline void __cdecl arrow(Widget* widget, uint32_t event) noexcept {
    const auto node = widget ? address(*widget) : 0;
    using Callback = void(__cdecl*)(Widget*, uint32_t);
    if (!catalogMode() || (node != leftArrow && node != rightArrow) || event != 0 ||
        value<uint32_t>(0x4D0A24) != 11 || value<uint32_t>(0x4D0A28) != 11) {
        reinterpret_cast<Callback>(originalArrow)(widget, event);
        return;
    }
    Roster list;
    if (!roster(list)) {
        reinterpret_cast<Callback>(originalArrow)(widget, event);
        return;
    }
    if (get<uint32_t>(*widget, 4) & hidden) return;
    const auto next = list.scroll(inputCells, node == rightArrow);
    if (next == list.start) return;
    try {
        write(0x760708, next);
        using Refresh = void(__cdecl*)();
        reinterpret_cast<Refresh>(originalRefresh)();
        update(inputCells, false);
        FleetArtwork::note("fleet_captain_scroll",
            std::to_string(list.start)+" to "+std::to_string(next));
    } catch (...) {
        hide();
        FleetArtwork::note("fleet_captain_scroll_failed", "state update failed");
    }
}

inline void __cdecl input(Widget* widget, uint32_t event) noexcept {
    if (!widget || !owns(address(*widget)) || !catalogMode() ||
        value<uint32_t>(0x4D0A24) != 11 || value<uint32_t>(0x4D0A28) != 11 ||
        (get<uint32_t>(*widget, 4) & hidden) || (event != 0 && event != 10)) return;
    const uint32_t offset = get<uint32_t>(*widget, 0) - firstExtra;
    Roster list;
    if (offset >= widgets.size() || !roster(list) || !list.contains(list.entry(6 + offset))) return;
    Widget native = *widget;
    // The original callback uses this ID only to compute the roster index.
    // The temporary record is never attached to the tree.
    set(native, 0, uint32_t(0x416 + offset));
    using Callback = void(__cdecl*)(Widget*, uint32_t);
    reinterpret_cast<Callback>(0x46D9E0)(&native, event);
    if (event == 0) FleetArtwork::note("fleet_captain_extra_select", std::to_string(7 + offset));
}

inline void __cdecl paint(Widget* widget, int x, int y) noexcept {
    if (!widget || !owns(address(*widget)) || !catalogMode() ||
        (get<uint32_t>(*widget, 4) & hidden) || discardDestination) return;
    const uint32_t offset = get<uint32_t>(*widget, 0) - firstExtra;
    Roster list;
    if (offset >= widgets.size() || !roster(list) || !list.contains(list.entry(6 + offset))) return;
    const auto outer = value<uint32_t>(value<uint32_t>(0xA39B64));
    if (!outer) return;
    Widget native = *widget;
    set(native, 0, uint32_t(0x416 + offset));
    struct Restore { ~Restore() { discardDestination = 0; } } restore;
    discardDestination = outer;
    observedExtra = false;
    using Painter = void(__cdecl*)(Widget*, int, int);
    reinterpret_cast<Painter>(0x46E340)(&native, x, y);
    if (observedExtra) FleetArtwork::note("fleet_captain_extra_paint", std::to_string(7 + offset));
}

inline bool install() noexcept {
    const uint32_t sites[] = {0x41D590, 0x46D9E0, 0x46E340, 0x46E230, 0x46E2B6, 0x46D430,
        originalArrow, originalRefresh};
    const std::array<uint8_t, 8> expected[] = {
        {0x8B,0x44,0x24,0x08,0x85,0xC0,0x75,0x05}, {0x51,0x8B,0x54,0x24,0x08,0x8B,0x02,0x8B},
        {0x83,0xEC,0x18,0x8B,0x54,0x24,0x24,0x53}, {0x8B,0x15,0x08,0x07,0x76,0x00,0x57,0x8D},
        {0x81,0xFF,0x15,0x04,0x00,0x00,0x0F,0x8E}, {0x83,0xC0,0xFA,0x3B,0xC8,0x0F,0x8D,0xBC},
        {0x53,0x8B,0x5C,0x24,0x08,0x8B,0x03,0x55}, {0x51,0x53,0x55,0x56,0x57,0xE8,0x66,0xDE},
    };
    for (size_t i = 0; i < std::size(sites); ++i) {
        std::array<uint8_t, 8> actual{};
        if (!read(sites[i], actual.data(), actual.size()) || actual != expected[i]) return false;
    }
    enabled = true;
    return true;
}

inline bool attachmentValid(const MenuProbe::Node& original) noexcept {
    auto node = value<uint32_t>(root + 0x73);
    for (unsigned n = 0; node && n < 64; ++n) {
        if (node == original.address) return true;
        node = value<uint32_t>(node + 0x6B);
    }
    return false;
}

inline void attach() {
    MenuProbe::Node original{};
    if (!MenuProbe::node(read, templateCell, original) || original.id != 0x415 ||
        original.parent != root || original.input != 0x46D9E0 || original.draw != 0x46E340 ||
        original.x != 718 || original.y != 69 || original.w != 76 || original.h != 79 ||
        !attachmentValid(original)) return;
    using Attach = void(__cdecl*)(Widget*, uint32_t);
    for (unsigned i = 0; i < widgets.size(); ++i) {
        auto& widget = widgets[i];
        if (get<uint32_t>(widget, 0x77)) continue;
        widget = {};
        set(widget, 0, firstExtra + i);
        set(widget, 4, 1u | hidden);
        set(widget, 8, original.x);
        set(widget, 10, original.y);
        set(widget, 12, original.w);
        set(widget, 14, original.h);
        set(widget, 0x46, value<uint16_t>(templateCell + 0x46));
        set(widget, 0x48, uint32_t(reinterpret_cast<uintptr_t>(&input)));
        set(widget, 0x4C, uint32_t(reinterpret_cast<uintptr_t>(&paint)));
        reinterpret_cast<Attach>(0x41D590)(&widget, root);
    }
}

inline void arrows(const Roster& list, unsigned cells) {
    const auto update = [](uint32_t addr, uint32_t id, bool show) {
        if (value<uint32_t>(addr) != id || value<uint32_t>(addr + 0x77) != root) return;
        if (value<uint32_t>(addr + 0x48) == originalArrow)
            write(addr + 0x48, uint32_t(reinterpret_cast<uintptr_t>(&arrow)));
        const auto old = value<uint32_t>(addr + 4);
        const auto flags = show ? old & ~hidden : old | hidden;
        if (flags != old) write(addr + 4, flags);
    };
    update(leftArrow, 0x40A, list.start > 0);
    update(rightArrow, 0x40B, list.start < list.last(cells));
}

inline unsigned update(unsigned capacity, bool mayAttach) {
    Roster list;
    if (!catalogMode() || !roster(list)) { hide(); inputCells=6; return 6; }
    if (capacity <= 6 || list.count <= 6 || list.start > list.last(capacity)) {
        hide();
        arrows(list, 6);
        inputCells=6;
        return 6;
    }
    if (mayAttach) attach();
    if (get<uint32_t>(widgets[0], 0x77) != root || get<uint32_t>(widgets[1], 0x77) != root) {
        hide();
        arrows(list, 6);
        inputCells=6;
        return 6;
    }
    const auto slot = value<uint32_t>(0x7606FC);
    const auto fleet = value<uint8_t>(0x5FF7F8);
    uint32_t active = 0xFFFFFFFF;
    if (slot < 6) read(0x5F1630 + (unsigned(fleet) * 6 + slot) * 0x220, &active, sizeof(active));
    for (unsigned i = 0; i < widgets.size(); ++i) {
        auto& widget = widgets[i];
        const auto index = list.entry(6 + i);
        auto flags = get<uint32_t>(widget, 4) & ~(hidden | selected);
        Captain row{};
        if (6 + i >= capacity || !list.contains(index) ||
            !read(0x75FF00 + uint32_t(index) * sizeof(row), &row, sizeof(row))) flags |= hidden;
        else {
            set(widget, 0x50, status(row));
            if (row.id == active) flags |= selected;
        }
        set(widget, 4, flags);
    }
    arrows(list, capacity);
    inputCells=capacity;
    return capacity;
}

}
