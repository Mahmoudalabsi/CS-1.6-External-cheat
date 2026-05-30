#include "main.h"
#include <string>
#include <thread>
#include <chrono>
#include "MemoryManager.h"
#include "Settings.h"
#include "Config.h"
#include "Renderer.h"
#include "Structs.h"
#include "Offsets.h"
MemoryManager* m;
int Page = 0;
int SelectedGroup = 0;
int SelectedWeapon = 0;
int SelectedWeaponID = 0;
bool lastAutoPistolState = 0;
int WeaponID = 0;
int PlayerTeam;
float gWorldToScreen[16];
int InMenu;
float recoil;
// Triggerbot state
DWORD triggerLastShotTime = 0;
bool triggerIsShooting = false;
std::vector<int> Anims;
std::vector<int> OldAnims;
std::vector<int> OldOldAnims;
std::vector<Vector3> Targets;
std::vector<Vector3> TargetsWS;
std::vector<std::string> TargetModels;
std::vector<std::string> TargetNames;
bool SetMenuKey = false;
std::string MenuKeyLabel = "INSERT";
bool SetAimbotKey = false;
std::string AimbotKeyLabel = "MOUSE 1";
bool SetAimbot2Key = false;
std::string Aimbot2KeyLabel = "MOUSE 2";
bool SetBhopKey = false;
std::string BhopKeyLabel = "SPACE";
bool SetDDrunKey = false;
std::string DDrunKeyLabel = "ALT";
// Helper: determine if entity is CT based on model name
bool IsEntityCT(int entityIndex)
{
	if (entityIndex < 0 || entityIndex >= (int)TargetModels.size()) return false;
	std::string mdl = TargetModels[entityIndex];
	return (mdl.find("urban") != std::string::npos ||
		mdl.find("gsg9") != std::string::npos ||
		mdl.find("sas") != std::string::npos ||
		mdl.find("gign") != std::string::npos);
}
// Helper: determine if entity is T based on model name
bool IsEntityT(int entityIndex)
{
	if (entityIndex < 0 || entityIndex >= (int)TargetModels.size()) return false;
	std::string mdl = TargetModels[entityIndex];
	return (mdl.find("terror") != std::string::npos ||
		mdl.find("arctic") != std::string::npos ||
		mdl.find("guerrilla") != std::string::npos ||
		mdl.find("leet") != std::string::npos);
}
// Helper: check if entity is a teammate of the local player
bool IsTeammate(int entityIndex)
{
	if (PlayerTeam == 1 && IsEntityCT(entityIndex)) return true;  // We are CT, entity is CT
	if (PlayerTeam == 2 && IsEntityT(entityIndex)) return true;   // We are T, entity is T
	return false;
}
void InitCheat()
{
	std::string configDir = std::string(getenv("appdata")) + std::string("\\INTERIUM");
	CreateDirectory(configDir.c_str(), 0);
	configDir = configDir + std::string("\\CS16Ext");
	CreateDirectory(configDir.c_str(), 0);
	Sleep(100);
	m = new MemoryManager;
	DWORD PID = 0;
	DWORD client = getModuleAddress(PID, "client.dll");
}
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
bool ScreenTransform(Vector3 vPoint, float* vScreen)
{
	vScreen[0] = gWorldToScreen[0]*vPoint.x + gWorldToScreen[4]*vPoint.y + gWorldToScreen[8]*vPoint.z  + gWorldToScreen[12];
	vScreen[1] = gWorldToScreen[1]*vPoint.x + gWorldToScreen[5]*vPoint.y + gWorldToScreen[9]*vPoint.z  + gWorldToScreen[13];
	float w     = gWorldToScreen[3]*vPoint.x + gWorldToScreen[7]*vPoint.y + gWorldToScreen[11]*vPoint.z + gWorldToScreen[15];
	if (w == 0.0f) return (0.0f >= w);
	float invW = 1.0f / w;
	vScreen[0] *= invW;
	vScreen[1] *= invW;
	return (0.0f >= invW);
}
Vector3 W2S(Vector3 WorldPos)
{
	float vScreen[2] = { 0, 0 };
	bool behind = ScreenTransform(WorldPos, vScreen);
	if (!behind && vScreen[0] > -1.0f && vScreen[1] > -1.0f)
	{
		float screenX = vScreen[0] * (Width  / 2.0f) + (Width  / 2.0f);
		float screenY = -vScreen[1] * (Height / 2.0f) + (Height / 2.0f);
		return Vector3(screenX, screenY, 0);
	}
	return Vector3(0, 0, 0);
}
void Hack()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		PlayerTeam = m->ReadMem<int>(m->cDll.base + Offsets::PlayerTeam);
		InMenu = m->ReadMem<int>(m->eDll.base + Offsets::InMenu);
		m->Read(m->eDll.base + Offsets::ViewMatrix, gWorldToScreen, sizeof(gWorldToScreen));
		recoil = m->ReadMem<float>(m->eDll.base + Offsets::Recoil);
		WeaponID = m->ReadMem<int>(m->eDll.base + Offsets::WeaponID);
		Aimbot::FOV = Weapons[WeaponID].FOV;
		Aimbot::Smooth = Weapons[WeaponID].Smootch;
		Aimbot::RCS = Weapons[WeaponID].RCS;
		Targets.clear();
		Anims.clear();
		for (int i = 0; i < 64; i++)
		{
			int AnimState = m->ReadMem<int>(m->eDll.base + Offsets::AnimState + i * 592);
			Anims.push_back(AnimState);
			Vector3 Posithion = m->ReadMem<Vector3>(m->eDll.base + Offsets::Posithion + i * 592);
			Targets.push_back(Posithion);
		}
		if (GetForegroundWindow() == FindWindow("SDL_app", NULL) && !InMenu && !ShowMenu)
		{
			if (MISC::Bhop && GetAsyncKeyState(KEYS::BhopKey))
			{
				int OnGround = m->ReadMem<int>(m->eDll.base + Offsets::OnGround);
				if (OnGround == 1)
				{
					m->WriteMem<int>(m->cDll.base + Offsets::dwForceJump, 5);
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
					m->WriteMem<int>(m->cDll.base + Offsets::dwForceJump, 4);
				}
			}
			if (MISC::AutoPistol)
			{
				if (GetAsyncKeyState(VK_LBUTTON))
				{
					if (WeaponID == 17 || WeaponID == 16 || WeaponID == 1 || WeaponID == 26 || WeaponID == 10 || WeaponID == 11)
					{
						m->WriteMem<int>(m->cDll.base + Offsets::dwForceAttack, 5);
						std::this_thread::sleep_for(std::chrono::milliseconds(15));
						m->WriteMem<int>(m->cDll.base + Offsets::dwForceAttack, 4);
					}
				}
				else m->WriteMem<int>(m->cDll.base + Offsets::dwForceAttack, 4);
			}
			// ===== TRIGGERBOT (InCross Method) =====
			// Activated via menu toggle only, no keyboard key needed
			if (TRIGGERBOT::Enabled)
			{
				// Read InCross: entity index under crosshair (0 = nobody)
				int InCrossID = m->ReadMem<int>(m->cDll.base + Offsets::InCross);

				bool shouldShoot = false;

				if (InCrossID > 0 && InCrossID < 33)
				{
					// There is a player under the crosshair
					if (TRIGGERBOT::Deathmatch)
					{
						// Deathmatch mode: shoot everyone
						shouldShoot = true;
					}
					else
					{
						// Team mode: only shoot enemies
						// Check if the entity under crosshair is a teammate
						bool teammate = IsTeammate(InCrossID - 1); // InCrossID is 1-based, our arrays are 0-based
						if (!teammate) shouldShoot = true;
					}
				}

				// Handle shooting with delays
				DWORD currentTime = GetTickCount();
				if (shouldShoot)
				{
					if (!triggerIsShooting)
					{
						if (currentTime - triggerLastShotTime >= (DWORD)TRIGGERBOT::Delay)
						{
							m->WriteMem<int>(m->cDll.base + Offsets::dwForceAttack, 5);
							triggerIsShooting = true;
							triggerLastShotTime = currentTime;
						}
					}
					else
					{
						if (currentTime - triggerLastShotTime >= (DWORD)TRIGGERBOT::ShotDelay)
						{
							m->WriteMem<int>(m->cDll.base + Offsets::dwForceAttack, 5);
							triggerLastShotTime = currentTime;
						}
					}
				}
				else
				{
					if (triggerIsShooting)
					{
						m->WriteMem<int>(m->cDll.base + Offsets::dwForceAttack, 4);
						triggerIsShooting = false;
						triggerLastShotTime = currentTime;
					}
				}
			}
			else
			{
				if (triggerIsShooting)
				{
					m->WriteMem<int>(m->cDll.base + Offsets::dwForceAttack, 4);
					triggerIsShooting = false;
				}
			}
			// ===== END TRIGGERBOT =====
			if (MISC::DDrun && GetAsyncKeyState(KEYS::DDrunKey))
			{
				int OnGround = m->ReadMem<int>(m->eDll.base + Offsets::OnGround);
				if (OnGround == 1)
				{
					m->WriteMem<int>(m->cDll.base + Offsets::dwForceDuck, 5);
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
					m->WriteMem<int>(m->cDll.base + Offsets::dwForceDuck, 4);
				}
			}
		}
		if (GetAsyncKeyState(KEYS::MenuKey))
		{
			if (!ShowMenu)
			{
				if (WeaponID == 17) { SelectedGroup = 0; SelectedWeapon = 0; }
				if (WeaponID == 16) { SelectedGroup = 0; SelectedWeapon = 1; }
				if (WeaponID == 1 ) { SelectedGroup = 0; SelectedWeapon = 2; }
				if (WeaponID == 26) { SelectedGroup = 0; SelectedWeapon = 3; }
				if (WeaponID == 10) { SelectedGroup = 0; SelectedWeapon = 4; }
				if (WeaponID == 11) { SelectedGroup = 0; SelectedWeapon = 5; }
				if (WeaponID == 7) { SelectedGroup = 1; SelectedWeapon = 0; }
				if (WeaponID == 23) { SelectedGroup = 1; SelectedWeapon = 1; }
				if (WeaponID == 19) { SelectedGroup = 1; SelectedWeapon = 2; }
				if (WeaponID == 12) { SelectedGroup = 1; SelectedWeapon = 3; }
				if (WeaponID == 30) { SelectedGroup = 1; SelectedWeapon = 4; }
				if (WeaponID == 14) { SelectedGroup = 2; SelectedWeapon = 0; }
				if (WeaponID == 15) { SelectedGroup = 2; SelectedWeapon = 1; }
				if (WeaponID == 28) { SelectedGroup = 2; SelectedWeapon = 2; }
				if (WeaponID == 22) { SelectedGroup = 2; SelectedWeapon = 3; }
				if (WeaponID == 3) { SelectedGroup = 2; SelectedWeapon = 4; }
				if (WeaponID == 27) { SelectedGroup = 2; SelectedWeapon = 5; }
				if (WeaponID == 8) { SelectedGroup = 2; SelectedWeapon = 6; }
				if (WeaponID == 18) { SelectedGroup = 2; SelectedWeapon = 7; }
				if (WeaponID == 24) { SelectedGroup = 2; SelectedWeapon = 8; }
				if (WeaponID == 13) { SelectedGroup = 2; SelectedWeapon = 9; }
				if (WeaponID == 21) { SelectedGroup = 3; SelectedWeapon = 0; }
				if (WeaponID == 5) { SelectedGroup = 3; SelectedWeapon = 1; }
				if (WeaponID == 20) { SelectedGroup = 3; SelectedWeapon = 2; }
				SetForegroundWindow(g_hwnd);
				SetWindowLong(g_hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST);
				SetForegroundWindow(g_hwnd);
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				SetForegroundWindow(g_hwnd);
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				SetForegroundWindow(g_hwnd);
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			}
			else
			{
				SetForegroundWindow(GameHWND);
				SetWindowLong(g_hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT);
				SetForegroundWindow(GameHWND);
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				SetForegroundWindow(GameHWND);
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				SetForegroundWindow(GameHWND);
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			}
			ShowMenu = !ShowMenu;
			while (GetAsyncKeyState(KEYS::MenuKey))
			{
				if (!ShowMenu) SetForegroundWindow(g_hwnd);
				else SetForegroundWindow(GameHWND);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
		else if (GetAsyncKeyState(VK_END)) exit(1);
	}
}
void Updater()
{
	while (true)
	{
		GameHWND = FindWindow("SDL_app", NULL);
		if (GameHWND)
		{
			GetWindowRect(GameHWND, &tSize);
			Width = tSize.right - tSize.left;
			Height = tSize.bottom - tSize.top;
			DWORD dwStyle = GetWindowLong(GameHWND, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				tSize.top += 23;
				Height -= 23;
			}
			MoveWindow(g_hwnd, tSize.left, tSize.top + yoffset, Width, Height, true);
		}
		else
		{
			m->~MemoryManager();
			delete m;
			Sleep(3000);
			GameHWND = FindWindow("SDL_app", NULL);
			if (GameHWND)
			{
				InitCheat();
				DWORD PID = 0;
				GetWindowThreadProcessId(GameHWND, &PID);
				DWORD client = getModuleAddress(PID, "client.dll");
			}
			else exit(1);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		OldOldAnims.clear();
		for (int i = 0; i < 64; i++)
		{
			int AnimState = m->ReadMem<int>(m->eDll.base + Offsets::AnimState + i * 592);
			OldOldAnims.push_back(AnimState);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		TargetModels.clear();
		for (int i = 0; i < 64; i++)
		{
			char PlayerModel[128];
			m->Read(m->eDll.base + Offsets::Model + i * 592, PlayerModel, 128);
			std::string Model = PlayerModel;
			if (strlen(Model.c_str()) > 3) Model.substr(0, Model.size() - 2);
			TargetModels.push_back(Model);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		TargetNames.clear();
		for (int i = 0; i < 64; i++)
		{
			char PlayerName[32];
			m->Read(m->eDll.base + Offsets::Name + i * 592, PlayerName, 31);
			std::string Name = PlayerName;
			TargetNames.push_back(Name);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		OldAnims.clear();
		for (int i = 0; i < 64; i++)
		{
			int AnimState = m->ReadMem<int>(m->eDll.base + Offsets::AnimState + i * 592);
			OldAnims.push_back(AnimState);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (MISC::FpsUnlock)
		{
			m->WriteMem<float>(m->eDll.base + Offsets::dwMaxFps, 999.f);
			m->WriteMem<float>(m->eDll.base + Offsets::dwMaxFps + 104, 999.f);
		}
	}
}
int main(int, char**)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WINNAME;
	wc.lpszMenuName = WINNAME;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&wc);
	g_hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED , WINNAME, WINNAME, WS_POPUP, 0, 0, Width, Height, 0, 0, 0, 0);
	SetLayeredWindowAttributes(g_hwnd, 0, 255, LWA_ALPHA);
	SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
	DwmExtendFrameIntoClientArea(g_hwnd, &MARGIN);
	ShowWindow(g_hwnd, SW_SHOW);
	if (CreateDeviceD3D(g_hwnd) < 0) ExitProcess(1);
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	ImFont* fontEsp = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	ImGui_ImplWin32_Init(g_hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	if (!FindWindow("SDL_app", NULL)) {MessageBox(NULL, "START CS 1.6 BEFORE STARTING THE CHEAT", NULL, NULL); ExitProcess(EXIT_SUCCESS);}
	InitCheat();
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Updater, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Hack, 0, 0, 0);
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		if (!open) ExitProcess(0);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::SetNextWindowPosCenter();
			if (ShowMenu)
			{
				ImGui::Begin("INTERIUM", &open, ImVec2(600, 480), 1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
				ImGui::Columns(2, NULL, false);
				ImGui::SetColumnWidth(0, 110.f);
				if (ImGui::Button(("AIMBOT"), ImVec2(100, 25))) Page = 0;
				if (ImGui::Button(("ESP"), ImVec2(100, 25))) Page = 1;
				if (ImGui::Button(("MISC"), ImVec2(100, 25))) Page = 2;
				if (ImGui::Button(("TRIGGER"), ImVec2(100, 25))) Page = 5;
				if (ImGui::Button(("KEYS"), ImVec2(100, 25))) Page = 4;
				if (ImGui::Button(("CFG"), ImVec2(100, 25))) Page = 3;
				ImGui::TextWrapped("This cheat was made by KleskBY in 2019");
				ImGui::PopStyleVar(3);
				ImGui::NextColumn();
				if (Page == 0)
				{
					ImGui::Spacing(0, 5);
					ImGui::Checkbox(("Aimbot"), &Aimbot::Enabled); ImGui::SameLine(); ImGui::Checkbox(("Deathmatch"), &Aimbot::Deathmatch);
					ImGui::Combo(("Group"), &SelectedGroup, ("Pistols\0SMG\0Rifles\0Heavy\0"));
					if (SelectedGroup == 0) ImGui::Combo(("Weapon"), &SelectedWeapon, ("Glock\0USP\0P250\0Desert Eagle\0Dual berettas\0Five Seven\0"));
					else  if (SelectedGroup == 1) ImGui::Combo(("Weapon"), &SelectedWeapon, ("MAC-10\0TMP\0MP-5\0UMP\0P-90\0"));
					else if (SelectedGroup == 2) ImGui::Combo(("Weapon"), &SelectedWeapon, ("Galil\0Famas\0AK-47\0M4A1\0Scout\0SG\0AUG\0AWP\0AUTO T\0AUTO CT\0"));
					else if (SelectedGroup == 3) ImGui::Combo(("Weapon"), &SelectedWeapon, ("NOVA\0XM-ATUO\0M249\0"));
					if (SelectedGroup == 0 && SelectedWeapon == 0) SelectedWeaponID = 17;
					else if (SelectedGroup == 0 && SelectedWeapon == 1) SelectedWeaponID = 16;
					else if (SelectedGroup == 0 && SelectedWeapon == 2) SelectedWeaponID = 1;
					else if (SelectedGroup == 0 && SelectedWeapon == 3) SelectedWeaponID = 26;
					else if (SelectedGroup == 0 && SelectedWeapon == 4) SelectedWeaponID = 10;
					else if (SelectedGroup == 0 && SelectedWeapon == 5) SelectedWeaponID = 11;
					else if (SelectedGroup == 1 && SelectedWeapon == 0) SelectedWeaponID = 7;
					else if (SelectedGroup == 1 && SelectedWeapon == 1) SelectedWeaponID = 23;
					else if (SelectedGroup == 1 && SelectedWeapon == 2) SelectedWeaponID = 19;
					else if (SelectedGroup == 1 && SelectedWeapon == 3) SelectedWeaponID = 12;
					else if (SelectedGroup == 1 && SelectedWeapon == 4) SelectedWeaponID = 30;
					else if (SelectedGroup == 2 && SelectedWeapon == 0) SelectedWeaponID = 14;
					else if (SelectedGroup == 2 && SelectedWeapon == 1) SelectedWeaponID = 15;
					else if (SelectedGroup == 2 && SelectedWeapon == 2) SelectedWeaponID = 28;
					else if (SelectedGroup == 2 && SelectedWeapon == 3) SelectedWeaponID = 22;
					else if (SelectedGroup == 2 && SelectedWeapon == 4) SelectedWeaponID = 3;
					else if (SelectedGroup == 2 && SelectedWeapon == 5) SelectedWeaponID = 27;
					else if (SelectedGroup == 2 && SelectedWeapon == 6) SelectedWeaponID = 8;
					else if (SelectedGroup == 2 && SelectedWeapon == 7) SelectedWeaponID = 18;
					else if (SelectedGroup == 2 && SelectedWeapon == 8) SelectedWeaponID = 24;
					else if (SelectedGroup == 2 && SelectedWeapon == 9) SelectedWeaponID = 13;
					else if (SelectedGroup == 3 && SelectedWeapon == 0) SelectedWeaponID = 21;
					else if (SelectedGroup == 3 && SelectedWeapon == 1) SelectedWeaponID = 5;
					else if (SelectedGroup == 3 && SelectedWeapon == 2) SelectedWeaponID = 20;
					ImGui::SliderFloat(("FOV"), &Weapons[SelectedWeaponID].FOV, 0.f, 12.f);
					ImGui::SliderFloat(("Smooth"), &Weapons[SelectedWeaponID].Smootch, 1.f, 12.f);
					ImGui::SliderFloat(("RCS"), &Weapons[SelectedWeaponID].RCS, 0.f, 12.f);
					ImGui::Checkbox(("Square FOV"), &Aimbot::SquareFov);
					ImGui::Checkbox(("Draw FOV"), &Aimbot::DrawFov);
					ImGui::ColorEdit4(("FOV color"), Aimbot::DrawFovColor, ImGuiColorEditFlags_NoInputs);
					ImGui::SliderFloat(("FOV thickness"), &Aimbot::DrawFovWidth, 0.f, 10.f);
				}
				else if (Page == 1)
				{
					ImGui::Spacing(0, 5);
					ImGui::Checkbox(("Box"), &ESP::Box);
					ImGui::SameLine(); ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
					ImGui::PushItemWidth(120.f);
					ImGui::Combo(("Type"), &ESP::BoxType, ("Classic\0Corner\0"));
					ImGui::PopItemWidth();
					ImGui::ColorEdit4(("Enemy Box color"), ESP::BoxColor, ImGuiColorEditFlags_NoInputs);
					ImGui::SliderFloat(("Box thickness"), &ESP::BoxWidth, 1.f, 10.f);
					if(ESP::BoxType == 0) ImGui::SliderFloat(("Box rounding"), &ESP::BoxRounding, 0.f, 10.f);
					ImGui::Separator();
					ImGui::Checkbox(("Show Teammates"), &ESP::ShowTeam);
					if (ESP::ShowTeam)
					{
						ImGui::ColorEdit4(("Team Box color"), ESP::TeamBoxColor, ImGuiColorEditFlags_NoInputs);
					}
					ImGui::Separator();
					ImGui::Checkbox(("Draw distance"), &ESP::Dist);
					ImGui::ColorEdit4(("Dist color"), ESP::DistColor, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox(("Draw names"), &ESP::Names);
					ImGui::ColorEdit4(("Names color"), ESP::NamesColor, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox(("Crosshair"), &ESP::Crosshair);
					ImGui::ColorEdit4(("Crosshair color"), ESP::CrosshairColor, ImGuiColorEditFlags_NoInputs);
					ImGui::SliderFloat(("Crosshair thickness"), &ESP::CrosshairWidth, 0.f, 10.f);
					ImGui::SliderFloat(("Crosshair size"), &ESP::CrosshairSize, 0.f, 10.f);
				}
				else if (Page == 2)
				{
					ImGui::Spacing(0, 5);
					ImGui::Checkbox(("BunnyHop"), &MISC::Bhop);
					ImGui::Checkbox(("DDRun"), &MISC::DDrun);
					ImGui::Checkbox(("AutoPistol"), &MISC::AutoPistol);
					ImGui::Checkbox(("FPS Unlock"), &MISC::FpsUnlock);
				}
				else if (Page == 5)
				{
					ImGui::Spacing(0, 5);
					ImGui::Checkbox(("Triggerbot"), &TRIGGERBOT::Enabled); ImGui::SameLine(); ImGui::Checkbox(("Deathmatch"), &TRIGGERBOT::Deathmatch);
					ImGui::SliderInt(("Delay (ms)"), &TRIGGERBOT::Delay, 0, 500);
					ImGui::SliderInt(("Shot Delay (ms)"), &TRIGGERBOT::ShotDelay, 10, 500);
					ImGui::Separator();
					ImGui::TextWrapped("Toggle ON/OFF from menu. No key needed.");
					ImGui::TextWrapped("Uses InCross detection: auto-shoots");
					ImGui::TextWrapped("when crosshair is over an enemy.");
				}
				else if (Page == 3)
				{
					ImGui::Spacing(0, 5);
					ImGui::BeginChild(("##config"), ImVec2(457,300));
					RefreshSettings();
					for (int i = 0; i < SettingsList.size(); i++)
					{
						char cblabel[128];
						sprintf(cblabel, "%s", SettingsList[i].c_str());
						IsConfigSelected[i] = false;
						ImGui::Checkbox(cblabel, &IsConfigSelected[i]);
						if (IsConfigSelected[i]) SelectedConfig = i;
					}
					ImGui::EndChild();
					if (ImGui::Button(("SAVE"), ImVec2(225, 25)))
					{
						if (strlen(cfgname) > 0)
						{
							std::string configDir = std::string(getenv("appdata")) + std::string("\\INTERIUM\\CS16Ext\\");
							SaveConfig(configDir + cfgname + ".ini");
						}
					}
					if (ImGui::Button(("DELETE"), ImVec2(225, 25)))
					{
						for (int j = 0; j < 50; j++)
						{
							if (IsConfigSelected[j] == true)
							{
								remove(SettingsList[j].c_str());
								IsConfigSelected[j] = false;
								break;
							}
						}
					}
					ImGui::InputText(("##configname"), cfgname, 25);
					if (ImGui::Button(("LOAD"), ImVec2(225, 25)))
					{
						for (int j = 0; j < 50; j++)
						{
							if (IsConfigSelected[j] == true)
							{
								LoadConfig(SettingsList[j]);
								break;
							}
						}
					}
				}
				else if (Page == 4)
				{
					ImGui::Spacing(0, 5);
					ImGui::Text(("Aimbot Key 1")); ImGui::SameLine();
					if (ImGui::Button(AimbotKeyLabel.c_str(), ImVec2(100, 20)))
						SetAimbotKey = true;
					ImGui::Text(("Aimbot Key 2")); ImGui::SameLine();
					if (ImGui::Button(Aimbot2KeyLabel.c_str(), ImVec2(100, 20)))
						SetAimbot2Key = true;
					ImGui::Text(("Bhop Key")); ImGui::SameLine();
					if (ImGui::Button(BhopKeyLabel.c_str(), ImVec2(100, 20)))
						SetBhopKey = true;
					ImGui::Text(("DDRun Key")); ImGui::SameLine();
					if (ImGui::Button(DDrunKeyLabel.c_str(), ImVec2(100, 20)))
						SetDDrunKey = true;
					ImGui::Text(("Menu Key")); ImGui::SameLine();
					if (ImGui::Button(MenuKeyLabel.c_str(), ImVec2(100, 20)))
						SetMenuKey = true;
					if (SetAimbotKey)
					{
						for (int i = 1; i < 256; i++)
						{
							if (GetAsyncKeyState(i))
							{
								KEYS::AimbotKey1 = i;
								SetAimbotKey = false;
								AimbotKeyLabel = std::to_string(i);
							}
						}
					}
					if (SetAimbot2Key)
					{
						for (int i = 1; i < 256; i++)
						{
							if (GetAsyncKeyState(i))
							{
								KEYS::AimbotKey2 = i;
								SetAimbot2Key = false;
								Aimbot2KeyLabel = std::to_string(i);
							}
						}
					}
					if (SetBhopKey)
					{
						for (int i = 1; i < 256; i++)
						{
							if (GetAsyncKeyState(i))
							{
								KEYS::BhopKey = i;
								SetBhopKey = false;
								BhopKeyLabel = std::to_string(i);
							}
						}
					}
					if (SetDDrunKey)
					{
						for (int i = 1; i < 256; i++)
						{
							if (GetAsyncKeyState(i))
							{
								KEYS::DDrunKey = i;
								SetDDrunKey = false;
								DDrunKeyLabel = std::to_string(i);
							}
						}
					}
					if (SetMenuKey)
					{
						for (int i = 1; i < 256; i++)
						{
							if (GetAsyncKeyState(i))
							{
								KEYS::MenuKey = i;
								SetMenuKey = false;
								MenuKeyLabel = std::to_string(i);
							}
						}
					}
				}
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.f));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::End();
				ImGui::PopStyleColor();
				ImGui::PopStyleVar(2);
			}
			// ESP Rendering (always, even when menu is hidden)
			{
				int BestTarget = -1;
				double ClosestPos = 9999999;
				float ScreenCenterX = Width / 2.0f;
				float ScreenCenterY = Height / 2.0f;
				float radiusx = Aimbot::FOV * (ScreenCenterX / 100);
				float radiusy = Aimbot::FOV * (ScreenCenterY / 100);
				if (Aimbot::SquareFov) radiusy = radiusx;
				if (ESP::Crosshair)
				{
					RenderLine(ImVec2(Width / 2 - ESP::CrosshairSize, Height / 2), ImVec2(Width / 2 + ESP::CrosshairSize, Height / 2), ImVec4(ESP::CrosshairColor[0], ESP::CrosshairColor[1], ESP::CrosshairColor[2], ESP::CrosshairColor[3]), ESP::CrosshairWidth);
					RenderLine(ImVec2(Width / 2, Height / 2 - ESP::CrosshairSize), ImVec2(Width / 2, Height / 2 + ESP::CrosshairSize), ImVec4(ESP::CrosshairColor[0], ESP::CrosshairColor[1], ESP::CrosshairColor[2], ESP::CrosshairColor[3]), ESP::CrosshairWidth);
				}
				if (Aimbot::DrawFov) RenderRect(ImVec2(ScreenCenterX - radiusx, ScreenCenterY - radiusy), ImVec2(ScreenCenterX + radiusx, ScreenCenterY + radiusy), ImVec4(Aimbot::DrawFovColor[0], Aimbot::DrawFovColor[1], Aimbot::DrawFovColor[2], Aimbot::DrawFovColor[3]), 2.f, ImDrawCornerFlags_All, Aimbot::DrawFovWidth);
				for (int i = 0; i < 64; i++)
				{
					if (Anims[i] > 200) continue;
					Vector3 Draw = W2S(Targets[i]);
					Vector3 headPos = Targets[i];
					headPos.z += 72.f;
					Vector3 DrawHead = W2S(headPos);
					if (Draw.x == 0 && Draw.y == 0) continue;
					float BoxHeight = fabs(Draw.y - DrawHead.y);
					float BoxWidth = BoxHeight / 2.f;
					// Team check - determine if this entity is a teammate
					bool isTeammate = IsTeammate(i);
					// Skip drawing ESP for teammates if ShowTeam is disabled
					if (isTeammate && !ESP::ShowTeam) continue;
					if (Draw.x >= ScreenCenterX - radiusx && Draw.x <= ScreenCenterX + radiusx &&
						Draw.y >= ScreenCenterY - radiusy && Draw.y <= ScreenCenterY + radiusy)
					{
						float PosX = Draw.x - (BoxHeight / 4.f);
						float PosY = DrawHead.y;
						float dx = Draw.x - ScreenCenterX;
						float dy = (Draw.y + DrawHead.y) / 2.f - ScreenCenterY;
						if (sqrt(dx * dx + dy * dy) < ClosestPos)
						{
							ClosestPos = sqrt(dx * dx + dy * dy);
							BestTarget = i;
						}
					}
					if (ESP::Box)
					{
						ImVec4 BoxCol;
						if (isTeammate)
						{
							// Teammate color (blue-ish, semi-transparent)
							BoxCol = ImVec4(ESP::TeamBoxColor[0], ESP::TeamBoxColor[1], ESP::TeamBoxColor[2], ESP::TeamBoxColor[3]);
						}
						else
						{
							// Enemy color: CT = blue, T = orange (original behavior)
							bool isCT = IsEntityCT(i);
							if (isCT)
								BoxCol = ImVec4(0.3f, 0.5f, 1.f, ESP::BoxColor[3]);
							else
								BoxCol = ImVec4(1.f, 0.5f, 0.3f, ESP::BoxColor[3]);
						}
						float bx = Draw.x - BoxWidth / 2.f;
						float by = DrawHead.y;
						if (ESP::BoxType == 0)
							RenderRect(ImVec2(bx, by), ImVec2(bx + BoxWidth, by + BoxHeight), BoxCol, ESP::BoxRounding, ImDrawCornerFlags_All, ESP::BoxWidth);
						else if (ESP::BoxType == 1)
						{
							float cornerLen = BoxHeight / 4.f;
							RenderLine(ImVec2(bx, by), ImVec2(bx + cornerLen, by), BoxCol, ESP::BoxWidth);
							RenderLine(ImVec2(bx, by), ImVec2(bx, by + cornerLen), BoxCol, ESP::BoxWidth);
							RenderLine(ImVec2(bx + BoxWidth, by), ImVec2(bx + BoxWidth - cornerLen, by), BoxCol, ESP::BoxWidth);
							RenderLine(ImVec2(bx + BoxWidth, by), ImVec2(bx + BoxWidth, by + cornerLen), BoxCol, ESP::BoxWidth);
							RenderLine(ImVec2(bx, by + BoxHeight), ImVec2(bx + cornerLen, by + BoxHeight), BoxCol, ESP::BoxWidth);
							RenderLine(ImVec2(bx, by + BoxHeight), ImVec2(bx, by + BoxHeight - cornerLen), BoxCol, ESP::BoxWidth);
							RenderLine(ImVec2(bx + BoxWidth, by + BoxHeight), ImVec2(bx + BoxWidth - cornerLen, by + BoxHeight), BoxCol, ESP::BoxWidth);
							RenderLine(ImVec2(bx + BoxWidth, by + BoxHeight), ImVec2(bx + BoxWidth, by + BoxHeight - cornerLen), BoxCol, ESP::BoxWidth);
						}
					}
					if (ESP::Dist)
					{
						Vector3 LocalPos = Targets[0];
						float dist = sqrt(pow(Targets[i].x - LocalPos.x, 2) + pow(Targets[i].y - LocalPos.y, 2) + pow(Targets[i].z - LocalPos.z, 2));
						char distStr[32];
						sprintf(distStr, "[%.0f]", dist);
						RenderText(distStr, ImVec2(Draw.x, Draw.y + 5), 16.0f, ImVec4(ESP::DistColor[0], ESP::DistColor[1], ESP::DistColor[2], ESP::DistColor[3]), true, fontEsp);
					}
					if (ESP::Names && i < (int)TargetNames.size())
					{
						RenderText(TargetNames[i].c_str(), ImVec2(Draw.x, DrawHead.y - 15), 16.0f, ImVec4(ESP::NamesColor[0], ESP::NamesColor[1], ESP::NamesColor[2], ESP::NamesColor[3]), true, fontEsp);
					}
				}
				// Aimbot
				if (BestTarget != -1 && Aimbot::Enabled && (GetAsyncKeyState(KEYS::AimbotKey1) || GetAsyncKeyState(KEYS::AimbotKey2)))
				{
					Vector3 aimTarget = Targets[BestTarget];
					aimTarget.z += 72.f;
					Vector3 aimScreen = W2S(aimTarget);
					float dx = aimScreen.x - ScreenCenterX;
					float dy = aimScreen.y - ScreenCenterY;
					if (Aimbot::Smooth > 1.f)
					{
						dx /= Aimbot::Smooth;
						dy /= Aimbot::Smooth;
					}
					mouse_event(MOUSEEVENTF_MOVE, (DWORD)dx, (DWORD)dy, 0, 0);
					if (Aimbot::RCS > 0.f)
					{
						float rcsX = recoil * Aimbot::RCS / 10.f;
						mouse_event(MOUSEEVENTF_MOVE, 0, -(DWORD)rcsX, 0, 0);
					}
				}
			}
			ImGui::EndFrame();
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ResetDevice();
			InitCheat();
		}
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CleanupDeviceD3D();
	DestroyWindow(g_hwnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
}

