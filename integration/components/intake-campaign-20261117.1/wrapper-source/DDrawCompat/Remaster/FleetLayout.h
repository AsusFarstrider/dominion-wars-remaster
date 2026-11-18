#pragma once
#include <cstdint>

namespace FleetArtwork {

struct Layout {
    static constexpr int catalogLeft = 338;
    static constexpr int catalogCell = 76;
    static constexpr int captainPitch = catalogCell + 10;
    static constexpr uint32_t seventhCaptain = 0x20000416;
    unsigned width = 800;
    unsigned captainCells = 6;

    unsigned captainCapacity() const noexcept {
        const int available = int(width) - catalogLeft - 22;
        const int cells = (available + captainPitch - catalogCell) / captainPitch;
        return cells >= 8 ? 8 : (cells >= 7 ? 7 : 6);
    }

    static Layout forOutput(unsigned width, unsigned height) noexcept {
        if (width < 800 || height < 600) return {};
        const auto logicalWidth = (uint64_t(width) * 600 + height / 2) / height;
        if (logicalWidth <= 800 || logicalWidth > 1600 || logicalWidth > width) return {};
        return {unsigned(logicalWidth)};
    }

    int extra() const noexcept { return int(width) - 800; }
    int center() const noexcept { return (extra() + 1) / 2; }

    int distribute(unsigned index, unsigned intervals) const noexcept {
        return (extra() * int(index) + int(intervals / 2)) / int(intervals);
    }

    int shift(int x, int y, uint32_t id = 0, uint8_t catalogMode = 0) const noexcept {
        if (catalogMode == 1 && captainCells > 6) {
            if (id >= 0x410 && id <= 0x415) return catalogLeft + captainPitch * int(id - 0x410) - x;
            if (id == seventhCaptain || id == seventhCaptain + 1)
                return catalogLeft + captainPitch * (6 + int(id - seventhCaptain)) - x;
        }
        // The engine owns each slot's picture, selection border and labels.
        // In crew mode the first three catalog cells own one +/− pair each.
        if (id >= 0x410 && id <= 0x415) {
            if (catalogMode == 2 && id <= 0x412) return distribute(id - 0x410, 2);
            if (catalogMode == 1 || catalogMode == 4) {
                // Keep the browse row near its preview and cap the added
                // space between neighboring controls at 18 logical pixels.
                const int addedWidth = extra() < 270 ? extra() : 270;
                return (addedWidth * int(id - 0x410) + 7) / 15;
            }
            return distribute(id - 0x410, 5);
        }
        if (id >= 0x418 && id <= 0x41D) return distribute((id - 0x418) / 2, 2);
        if (id >= 0x41F && id <= 0x424) return distribute(id - 0x41F, 5);
        switch (id) {
        case 0x40F: return 0;
        case 0x40C: return distribute(1, 3);
        case 0x40E: return distribute(2, 3);
        case 0x40D: return extra();
        case 0x40A: case 0x40B: case 0x417: case 0x426: return extra();
        }
        if (y < 400) return x >= 615 ? extra() : (x >= 235 ? center() : 0);
        if (y < 540) {
            const unsigned slot = unsigned(x > 21 ? x - 21 : 0) / 126;
            return distribute(slot < 6 ? slot : 5, 5);
        }
        if (x >= 615) return extra();
        return x >= 200 ? center() : 0;
    }

    static bool ownsPreview(uint32_t caller) noexcept {
        // Fleet's root draws the title, the left preview, then the lower panels.
        // All direct preview sprites and text in this block stay with its frame.
        return caller > 0x46F13A && caller < 0x47070D;
    }

    double backgroundX(double x) const noexcept {
        // Extend the rail at two vertical strips between the LCARS panels.
        constexpr int leftSeam = 235, rightSeam = 615;
        if (x < leftSeam) return x;
        if (x < leftSeam + center()) return leftSeam - 0.125;
        if (x < rightSeam + center()) return x - center();
        if (x < rightSeam + extra()) return rightSeam - 0.125;
        return x - extra();
    }
};

}
