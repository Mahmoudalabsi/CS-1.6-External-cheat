#pragma once
#include <Windows.h>
#include <string>

static int yoffset = 0;

struct ConfigWeapon
{
	bool 	Enabled;
	float 	FOV;
	float	Smootch = 1.f;
	float	RCS;

};
static ConfigWeapon Weapons[40];

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
	static float DrawFovColor[4] = { 1.f,1.f,1.f,1.f };
	static float DrawFovWidth = 1.f;
}
namespace ESP
{
	static bool Box = true;
	static int BoxType = 0;
	static float BoxColor[4] = { 1.f,1.f,1.f,1.f };
	static float BoxRounding = 0.f;
	static float BoxWidth = 1.f;
	static float BoxSize = 1.f;  // Box scale multiplier (0.5=small, 1.0=normal, 1.5=big)
	static bool ShowTeam = true;

	static bool Dist = true;
	static float DistColor[4] = { 1.f,1.f,1.f,1.f };
	static bool Names = true;
	static float NamesColor[4] = { 1.f,1.f,1.f,1.f };

	static bool Crosshair = true;
	static float CrosshairColor[4] = { 1.f,0.f,0.f,1.f };
	static float CrosshairSize = 5.f;
	static float CrosshairWidth = 1.f;
}
namespace MISC
{
	bool Bhop = false;
	bool DDrun = false;
	bool AutoPistol = false;
	bool FpsUnlock = false;
}
namespace TRIGGERBOT
{
	static bool Enabled = false;
	static int Delay = 50;
	static int ShotDelay = 100;
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
