#pragma once
#include <Windows.h>

namespace Offsets
{
    // ===== Updated for Exe build: 8684 (Aug 3 2020) =====
    // ViewMatrix method: Mahmoudalabsi/Evelion (4x4 matrix from hw.dll)
    // Other offsets: KleskBY/CS-1.6-External-cheat (verified working)

    // Engine (hw.dll) offsets
    const DWORD MoveType      = 0x100A0B8;  // 520=ground, 8=air, 24=water, 536=water+ground
    const DWORD OnGround      = 0x122DF54;  // 1=On Ground, 0=in air
    const DWORD IsChatting    = 0x64429C;   // 1 when chat is enabled
    const DWORD InMenu        = 0x135484;   // 1 when in menu
    const DWORD ViewMatrix    = 0xEC9780;   // 4x4 float matrix (16 floats = 64 bytes) - hw.dll
    const DWORD fovscale1     = 0xEC9AA0;   // FOV scale X (~1.0) - hw.dll (backup)
    const DWORD fovscale2     = 0xEC9AB4;   // FOV scale Y (~1.333) - hw.dll (backup)
    const DWORD Recoil        = 0x122DFA4;  // recoil value - hw.dll
    const DWORD WeaponID      = 0x108DA10;  // current weapon ID - hw.dll
    const DWORD AnimState     = 0x01204418; // entity 1 anim_frame - hw.dll
    const DWORD Posithion     = 0x01204420; // entity 1 position - hw.dll
    const DWORD Model         = 0x012043C8; // entity 1 model - hw.dll
    const DWORD Name          = 0x0120439C; // entity 1 name - hw.dll
    const DWORD dwMaxFps      = 0x149B14;   // max FPS value - hw.dll

    // Client (client.dll) offsets
    const DWORD PlayerTeam    = 0x100DE4;   // 1=CT, 2=T
    const DWORD InCross       = 0x125314;   // entity index under crosshair - client.dll
    const DWORD dwForceJump   = 0x131424;   // force jump command - client.dll
    const DWORD dwForceDuck   = 0x1313A0;   // force duck command - client.dll
    const DWORD dwForceAttack = 0x131360;   // force attack command - client.dll
}
