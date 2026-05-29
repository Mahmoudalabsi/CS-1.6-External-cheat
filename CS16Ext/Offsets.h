#pragma once
#include <Windows.h>

namespace Offsets
{
    // ===== Updated for Exe build: 8684 (Aug 3 2020) =====

    // Engine (hw.dll) offsets
    const DWORD MoveType    = 0x100A0B8;  // NEW: 520=ground, 8=air, 24=water, 536=water+ground
    const DWORD OnGround    = 0x122E2D4;  // UPDATED: was 0x122DF54
    const DWORD IsChatting  = 0x64429C;   // NEW: 1 when chat is open
    const DWORD InMenu      = 0x6C3AB0;   // UPDATED: was 0x135484
    const DWORD Recoil      = 0x122E324;  // UPDATED: was 0x122DFA4
    const DWORD EntityList  = 0x12043C8;  // NEW: entity list base
    const DWORD fovscale1   = 0xEC9AA0;   // NOT UPDATED: may need verification
    const DWORD fovscale2   = 0xEC9AB4;   // NOT UPDATED: may need verification
    const DWORD WeaponID    = 0x108DA10;  // NOT UPDATED: may need verification
    const DWORD AnimState   = 0x01204418; // NOT UPDATED: may need verification
    const DWORD Posithion   = 0x01204420; // NOT UPDATED: may need verification
    const DWORD dwMaxFps    = 0x149B14;   // NOT UPDATED: may need verification

    // Client (client.dll) offsets
    const DWORD PlayerTeam  = 0x100DE4;   // NOT UPDATED: may need verification
    const DWORD OnLadder    = 0x136D8C;   // NEW: 1 on ladder, 0 on ground
    const DWORD InCross     = 0x125314;   // NEW: entity index under crosshair
    const DWORD ViewMatrix  = 0x12EAF0;   // UPDATED: was 0x12EAE0 (named "view")
    const DWORD Model       = 0x012043C8; // NOT UPDATED: may need verification
    const DWORD Name        = 0x0120439C; // NOT UPDATED: may need verification
    const DWORD dwForceJump  = 0x131424;  // NOT UPDATED: may need verification
    const DWORD dwForceDuck  = 0x1313A0;  // NOT UPDATED: may need verification
    const DWORD dwForceAttack = 0x131360; // NOT UPDATED: may need verification
}
