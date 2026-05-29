#pragma once
#include <Windows.h>

namespace Offsets
{
    // ===== Updated for Exe build: 8684 (Aug 3 2020) =====
    // Source: KleskBY patterns + CokpaT verified offsets (UnknownCheats)

    // Engine (hw.dll) offsets
    const DWORD MoveType      = 0x100A0B8;  // 520=ground, 8=air, 24=water, 536=water+ground
    const DWORD OnGround      = 0x122E2D4;  // 1=On Ground, 0=in air
    const DWORD IsChatting    = 0x64429C;   // 1 when chat is enabled
    const DWORD InMenu        = 0x6C3AB0;   // 1 when in menu
    const DWORD Recoil        = 0x122E324;  // recoil value
    const DWORD EntityList    = 0x12043C8;  // entity list base
    const DWORD fovscale1     = 0xEC9AA0;   // backup, not used by Evelion W2S
    const DWORD fovscale2     = 0xEC9AB4;   // backup, not used by Evelion W2S
    const DWORD WeaponID      = 0x108DD90;  // current weapon ID
    const DWORD AnimState     = 0x1204798;  // entity 1 anim_frame
    const DWORD Posithion     = 0x12047A0;  // entity 1 position
    const DWORD dwMaxFps      = 0x149CA8;   // max FPS value

    // Client (client.dll) offsets
    const DWORD PlayerTeam    = 0x100DF4;   // 1=CT, 2=T
    const DWORD OnLadder      = 0x136D8C;   // 1 on ladder, 0 on ground
    const DWORD InCross       = 0x125314;   // entity index under crosshair
    const DWORD ViewMatrix    = 0xEC9780;   // hw.dll (NOT client.dll!) - build 8684
    const DWORD Model         = 0x1204748;  // entity 1 model
    const DWORD Name          = 0x120471C;  // entity 1 name
    const DWORD dwForceJump   = 0x131434;   // force jump command
    const DWORD dwForceDuck   = 0x1313B0;   // force duck command
    const DWORD dwForceAttack = 0x131370;   // force attack command
}
