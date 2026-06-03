#pragma once
#include <Windows.h>
#include <string>

static int yoffset = 0;

struct ConfigWeapon
{
        bool    Enabled = false;
        float   FOV = 3.f;
        float   Smootch = 8.f;
        float   RCS = 3.f;
};
static ConfigWeapon Weapons[40];

// ===== Initialize weapon defaults from user config =====
inline void InitWeaponDefaults()
{
        // Pistols
        Weapons[1].Enabled = false; Weapons[1].FOV = 3.f;   Weapons[1].Smootch = 8.f;  Weapons[1].RCS = 3.f;    // P250
        Weapons[16].Enabled = true; Weapons[16].FOV = 2.5f; Weapons[16].Smootch = 8.f; Weapons[16].RCS = 1.f;   // USP
        Weapons[17].Enabled = true; Weapons[17].FOV = 2.5f; Weapons[17].Smootch = 8.f; Weapons[17].RCS = 1.f;   // Glock
        Weapons[26].Enabled = true; Weapons[26].FOV = 2.f;  Weapons[26].Smootch = 6.f; Weapons[26].RCS = 1.5f;  // Desert Eagle
        Weapons[10].Enabled = true; Weapons[10].FOV = 2.5f; Weapons[10].Smootch = 8.f; Weapons[10].RCS = 1.f;   // Dual Berettas
        Weapons[11].Enabled = true; Weapons[11].FOV = 2.5f; Weapons[11].Smootch = 9.f; Weapons[11].RCS = 1.f;   // Five Seven

        // SMGs
        Weapons[7].Enabled = true;  Weapons[7].FOV = 3.f;   Weapons[7].Smootch = 9.f;  Weapons[7].RCS = 3.f;    // MAC-10
        Weapons[23].Enabled = true; Weapons[23].FOV = 3.f;  Weapons[23].Smootch = 9.f; Weapons[23].RCS = 3.f;   // TMP
        Weapons[19].Enabled = true; Weapons[19].FOV = 3.f;  Weapons[19].Smootch = 9.f; Weapons[19].RCS = 3.f;   // MP-5
        Weapons[12].Enabled = true; Weapons[12].FOV = 3.f;  Weapons[12].Smootch = 8.f; Weapons[12].RCS = 3.f;   // UMP
        Weapons[30].Enabled = true; Weapons[30].FOV = 3.f;  Weapons[30].Smootch = 8.f; Weapons[30].RCS = 3.f;   // P-90

        // Rifles
        Weapons[14].Enabled = true; Weapons[14].FOV = 2.5f; Weapons[14].Smootch = 7.f; Weapons[14].RCS = 4.f;   // Galil
        Weapons[15].Enabled = true; Weapons[15].FOV = 2.5f; Weapons[15].Smootch = 7.f; Weapons[15].RCS = 3.5f;  // Famas
        Weapons[28].Enabled = true; Weapons[28].FOV = 2.5f; Weapons[28].Smootch = 6.5f; Weapons[28].RCS = 5.f;  // AK-47
        Weapons[22].Enabled = true; Weapons[22].FOV = 2.5f; Weapons[22].Smootch = 6.5f; Weapons[22].RCS = 4.5f; // M4A1
        Weapons[3].Enabled = true;  Weapons[3].FOV = 2.f;   Weapons[3].Smootch = 5.f;  Weapons[3].RCS = 0.5f;   // Scout
        Weapons[27].Enabled = true; Weapons[27].FOV = 2.5f; Weapons[27].Smootch = 7.f; Weapons[27].RCS = 4.f;   // SG
        Weapons[8].Enabled = true;  Weapons[8].FOV = 2.5f;  Weapons[8].Smootch = 7.f;  Weapons[8].RCS = 4.f;    // AUG
        Weapons[18].Enabled = true; Weapons[18].FOV = 1.5f; Weapons[18].Smootch = 4.f; Weapons[18].RCS = 0.5f;  // AWP
        Weapons[24].Enabled = true; Weapons[24].FOV = 1.5f; Weapons[24].Smootch = 4.f; Weapons[24].RCS = 0.5f;  // AUTO T
        Weapons[13].Enabled = true; Weapons[13].FOV = 1.5f; Weapons[13].Smootch = 4.f; Weapons[13].RCS = 0.5f;  // AUTO CT

        // Heavy
        Weapons[21].Enabled = true; Weapons[21].FOV = 3.f;  Weapons[21].Smootch = 10.f; Weapons[21].RCS = 2.f;  // NOVA
        Weapons[5].Enabled = true;  Weapons[5].FOV = 3.f;   Weapons[5].Smootch = 10.f; Weapons[5].RCS = 2.f;    // XM-Auto
        Weapons[20].Enabled = true; Weapons[20].FOV = 3.f;  Weapons[20].Smootch = 9.f;  Weapons[20].RCS = 4.f;  // M249
}

namespace Aimbot
{
        static bool Enabled = true;
        static bool Deathmatch = false;
        static bool SquareFov = true;
        static float FOV = 15.f;
        static float Smooth = 2.f;
        static float RCS = 4;
        static float AimOffset = 10.f;  // Aim height offset: lower = aim lower (18=neck, 10=chest, 0=feet)

        static bool DrawFov = true;
        static float DrawFovColor[4] = { 1.f, 1.f, 1.f, 0.784314f };
        static float DrawFovWidth = 0.5f;
}
namespace ESP
{
        static bool Box = true;
        static int BoxType = 1;  // 0=Classic, 1=Corner
        static float BoxColor[4] = { 0.0213523f, 0.947776f, 1.f, 0.7f };  // Sky blue
        static float BoxRounding = 0.f;
        static float BoxWidth = 3.037f;
        static float BoxSize = 1.f;  // Box scale multiplier (0.5=small, 1.0=normal, 1.5=big)
        static bool ShowTeam = false;

        static bool Dist = false;
        static float DistColor[4] = { 0.871886f, 0.0992895f, 0.0992895f, 0.6f };
        static bool Names = true;
        static float NamesColor[4] = { 0.8f, 0.8f, 0.2f, 0.7f };

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
        int AimbotKey1 = VK_LBUTTON;
        int AimbotKey2 = 0x56;
        int BhopKey = VK_SPACE;
        int DDrunKey = VK_MENU;
        int MenuKey = VK_INSERT;
}
