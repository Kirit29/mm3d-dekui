// SPDX-License-Identifier: GPL-2.0-or-later
// Controller support for the F1 settings menu: Start+Select opens/closes it,
// and the game sees no buttons while it is open (or while buttons used for the
// menu are still held after it closes). Adapted from Majora ReMasked
// (OTPR26/DekUI-Fork), which forwards the same buttons to the menu.
#pragma once
#include <cstdint>

namespace MM3D::MenuController {
// Menu buttons, in the order the frontend polls them.
enum Button : int { Up, Down, Left, Right, A, B, ZL, ZR, Start, Select, Count };

// 3DS HID pad bits (see Service::HID::PadState).
inline constexpr std::uint32_t PadSelect = 1u << 2, PadStart = 1u << 3;
inline constexpr std::uint32_t OpenChord = PadStart | PadSelect;

// Keeps menu input away from the game. Buttons held while the menu is open
// stay hidden until released, so closing the menu with B or Start+Select
// doesn't also press them in the game. The Start+Select chord never reaches
// the game, so opening the menu doesn't open the game's pause screen.
class Gate {
public:
    std::uint32_t Filter(std::uint32_t pad, bool menu_open) {
        if (menu_open) {
            suppressed |= pad;
            return 0;
        }
        suppressed &= pad;
        if ((pad & OpenChord) == OpenChord)
            suppressed |= OpenChord;
        return pad & ~suppressed;
    }
    void Reset() { suppressed = 0; }
private:
    std::uint32_t suppressed{};
};
} // namespace MM3D::MenuController
