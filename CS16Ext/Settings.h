#pragma once
#include <Windows.h>
#include <string>

static int yoffset = 0;

struct ConfigWeapon
{
        bool    Enabled;
        float   FOV;
        float   Smootch = 1.f;
        float   RCS;

};

// Default weapon configs from user config
static ConfigWeapon Weapons[40] = {
        // 0: Glock - disabled
        { false, 3.f, 8.f, 3.f },
        // 1: USP - enabled
        { true, 2.5f, 8.f, 1.f },
        // 2: P250 - disabled
        { false, 3.f, 8.f, 3.f },
        // 3: Desert Eagle - enabled
        { true, 2.f, 5.f, 0.5f },
        // 4: Dual berettas - disabled
        { false, 3.f, 8.f, 3.f },
        // 5: Five Seven - enabled
        { true, 3.f, 10.f, 2.f },
        // 6: unused - disabled
        { false, 3.f, 8.f, 3.f },
        // 7: MAC-10 - enabled
        { true, 3.f, 9.f, 3.f },
        // 8: TMP - enabled
        { true, 2.5f, 7.f, 4.f },
        // 9: unused - disabled
        { false, 3.f, 8.f, 3.f },
        // 10: MP-5 - enabled
        { true, 2.5f, 8.f, 1.f },
        // 11: UMP - enabled
        { true, 2.5f, 9.f, 1.f },
        // 12: P-90 - enabled
        { true, 3.f, 8.f, 3.f },
        // 13: Galil - enabled
        { true, 1.5f, 4.f, 0.5f },
        // 14: Famas - enabled
        { true, 2.5f, 7.f, 4.f },
        // 15: AK-47 - enabled
        { true, 2.5f, 7.f, 3.5f },
        // 16: M4A1 - enabled
        { true, 2.5f, 8.f, 1.f },
        // 17: Scout - enabled
        { true, 2.5f, 8.f, 1.f },
        // 18: SG - enabled
        { true, 1.5f, 4.f, 0.5f },
        // 19: AUG - enabled
        { true, 3.f, 9.f, 3.f },
        // 20: AWP - enabled
        { true, 3.f, 9.f, 4.f },
        // 21: AUTO T - enabled
        { true, 3.f, 10.f, 2.f },
        // 22: M4A1 (CT) - enabled
        { true, 2.5f, 6.5f, 4.5f },
        // 23: TMP alt - enabled
        { true, 3.f, 9.f, 3.f },
        // 24: AUTO CT - enabled
        { true, 1.5f, 4.f, 0.5f },
        // 25: unused - disabled
        { false, 3.f, 8.f, 3.f },
        // 26: Desert Eagle alt - enabled
        { true, 2.f, 6.f, 1.5f },
        // 27: SG alt - enabled
        { true, 2.5f, 7.f, 4.f },
        // 28: AK-47 alt - enabled
        { true, 2.5f, 6.5f, 5.f },
        // 29: unused - disabled
        { false, 3.f, 8.f, 3.f },
        // 30: P-90 alt - enabled
        { true, 3.f, 8.f, 3.f },
        // 31-39: unused - disabled
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f },
        { false, 3.f, 8.f, 3.f }
};

namespace Aimbot
{
        static bool Enabled = true;
        static bool Deathmatch = false;
        static bool SquareFov = true;
        static float FOV = 15.f;
        static float Smooth = 2.f;
        static float RCS = 4;
        static float AimOffset = 10.f;  // Aim height offset: lower = aim lower (18=neck, 10=chest, 0=feet)
        static bool HumanizeAim = true;  // Human-like mouse movement (anti-detect)
        static float HumanizeJitter = 0.3f;  // Random jitter amount (0=none, 1=high)
        static float HumanizeDelay = 1.f;  // Delay between micro-moves (ms)

        static bool DrawFov = true;
        static float DrawFovColor[4] = { 1.f, 1.f, 1.f, 0.784314f };
        static float DrawFovWidth = 0.5f;
}
namespace ESP
{
        static bool Box = true;
        static int BoxType = 1;  // Corner box
        static float BoxColor[4] = { 0.0213523f, 0.947776f, 1.f, 0.7f };  // Sky blue
        static float BoxRounding = 0.f;
        static float BoxWidth = 3.037f;
        static float BoxSize = 1.f;  // Box scale multiplier (0.5=small, 1.0=normal, 1.5=big)
        static bool ShowTeam = false;

        static bool Dist = false;
        static float DistColor[4] = { 0.871886f, 0.0992895f, 0.0992895f, 0.6f };
        static bool Names = true;
        static float NamesColor[4] = { 0.8f, 0.8f, 0.2f, 0.7f };  // Yellow-ish

        static bool Crosshair = true;
        static float CrosshairColor[4] = { 0.f, 0.7f, 0.f, 0.8f };  // Green
        static float CrosshairSize = 5.f;
        static float CrosshairWidth = 1.f;
}
namespace MISC
{
        bool Bhop = true;
        bool DDrun = false;
        bool AutoPistol = true;
        bool FpsUnlock = true;
}
namespace TRIGGERBOT
{
        static bool Enabled = false;
        static int Delay = 120;
        static int ShotDelay = 150;
        static bool Deathmatch = false;
}
namespace KEYS
{
        int AimbotKey1 = 1;  // MOUSE 1
        int AimbotKey2 = 86;  // V key
        int BhopKey = 32;  // SPACE
        int DDrunKey = 18;  // ALT
        int MenuKey = 45;  // INSERT
}
