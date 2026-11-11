// Copyright 2026 MM3D presentation contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>

namespace MM3D::OriginalMenuCadence {
// The Items and Masks pages are the 3DS bottom-screen layouts, which the game
// animates at 60 Hz even while gameplay runs at 30 Hz. In Original (30 FPS)
// mode, hold each presented menu frame for one 30 Hz interval so these pages
// match the rest of the game. Timing uses emulated swap time, so 30 Hz pages
// (Gear) are never halved and guest animation speed is unchanged.
constexpr std::int64_t MinimumIntervalUs = 25000; // between 1/60 s and 1/30 s

class Gate {
public:
    void Reset() {last_frame=0;last_time_us=-1;}
    // Returns false when this host present should be skipped, keeping the
    // previous menu image on screen.
    bool Present(bool active,std::uint64_t frame,std::int64_t time_us) {
        if(!active||time_us<0) {Accept(frame,time_us);return true;}
        if(frame==last_frame)return true; // repeated vblank of the shown frame
        if(last_time_us>=0&&time_us>=last_time_us&&
           time_us-last_time_us<MinimumIntervalUs)return false;
        Accept(frame,time_us);
        return true;
    }
private:
    void Accept(std::uint64_t frame,std::int64_t time_us) {last_frame=frame;last_time_us=time_us;}
    std::uint64_t last_frame{};
    std::int64_t last_time_us{-1};
};
} // namespace MM3D::OriginalMenuCadence
