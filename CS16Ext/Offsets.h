#pragma once
#include <Windows.h>

namespace Offsets
{
    // ===== Updated for Exe build: 8684 (Aug 3 2020) =====

    // Engine (hw.dll) offsets — exact values from programmer
    const DWORD MoveType    = 0x100A0B8;  // 520=ground, 8=air, 24=water, 536=water+ground
    const DWORD OnGround    = 0x122E2D4;  // 1=On Ground, 0=in air
    const DWORD IsChatting  = 0x64429C;   // 1 when chat is enabled
    const DWORD InMenu      = 0x6C3AB0;   // 1 when in menu
    const DWORD Recoil      = 0x122E324;  // recoil value
    const DWORD EntityList  = 0x12043C8;  // entity list base (PlayerInfo struct base)
    const DWORD fovscale1   = 0xEC7AB0;  // updated for build 8684
    const DWORD fovscale2   = 0xEC7AB0 + 0x14;  // = 0xEC7AC4
    const DWORD WeaponID    = 0x108DA10;  // needs verification for build 8684
    const DWORD AnimState   = 0x1204548;  // EntityList + 0x180 (anim_frame)
    const DWORD Posithion   = 0x1204550;  // EntityList + 0x188 (position)
    const DWORD dwMaxFps    = 0x149B14;   // needs verification for build 8684

    // Client (client.dll) offsets — exact values from programmer
    const DWORD PlayerTeam  = 0x100DE4;   // needs verification for build 8684
    const DWORD OnLadder    = 0x136D8C;   // 1 on ladder, 0 on ground
    const DWORD InCross     = 0x125314;   // entity index under crosshair
    const DWORD ViewMatrix  = 0x12EB30;  // 0x12EAF0 + 0x40 (pattern offset)
    const DWORD Model       = 0x12044F8;  // EntityList + 0x130 (model)
    const DWORD Name        = 0x12044CC;  // EntityList + 0x104 (name)
    const DWORD dwForceJump  = 0x131424;  // needs verification for build 8684
    const DWORD dwForceDuck  = 0x1313A0;  // needs verification for build 8684
    const DWORD dwForceAttack = 0x131360; // needs verification for build 8684
}
