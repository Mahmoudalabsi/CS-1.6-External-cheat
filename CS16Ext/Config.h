#pragma once
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <shlobj.h>
#include "Settings.h"

static std::vector<std::string> SettingsList;
static bool IsConfigSelected[50];
static int SelectedConfig = 0;
static char cfgname[25] = "";

// ===== Memory-only config storage (NO disk writes from the app) =====
static std::map<std::string, std::string> g_MemoryConfigs;

// ===== Full System Cleanup (Anti-Detect) =====
// Removes ALL traces the app could leave on the system:
// - Windows Prefetch (.pf files)
// - Recent files list
// - Temp files
// - Registry run/history entries
// - Thumbnail cache
// - Old INTERIUM config directory (from previous versions)
// Called on startup (clean old traces) and on exit (clean current traces)
inline void FullSystemCleanup()
{
        char sysDir[MAX_PATH];
        char userProfile[MAX_PATH];
        char windowsDir[MAX_PATH];

        // 1. CLEAN PREFETCH - This is the main thing WarGods checks
        GetWindowsDirectory(windowsDir, MAX_PATH);
        std::string prefetchDir = std::string(windowsDir) + "\\Prefetch\\";
        {
                WIN32_FIND_DATA findData;
                std::string searchPath = prefetchDir + "*.pf";
                HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                        do {
                                std::string filePath = prefetchDir + findData.cFileName;
                                DeleteFile(filePath.c_str());
                        } while (FindNextFile(hFind, &findData));
                        FindClose(hFind);
                }
        }

        // 2. CLEAN RECENT FILES
        GetEnvironmentVariable("USERPROFILE", userProfile, MAX_PATH);
        {
                std::string recentDir = std::string(userProfile) + "\\Recent\\";
                WIN32_FIND_DATA findData;
                std::string searchPath = recentDir + "*.*";
                HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                        do {
                                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
                                {
                                        std::string filePath = recentDir + findData.cFileName;
                                        DeleteFile(filePath.c_str());
                                }
                        } while (FindNextFile(hFind, &findData));
                        FindClose(hFind);
                }
        }

        // 3. CLEAN TEMP FILES
        {
                char tempDir[MAX_PATH];
                GetTempPath(MAX_PATH, tempDir);
                std::string tempPath = std::string(tempDir) + "*.*";
                WIN32_FIND_DATA findData;
                HANDLE hFind = FindFirstFile(tempPath.c_str(), &findData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                        do {
                                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
                                {
                                        std::string filePath = std::string(tempDir) + findData.cFileName;
                                        DeleteFile(filePath.c_str());
                                }
                        } while (FindNextFile(hFind, &findData));
                        FindClose(hFind);
                }

                // Also clean Windows\Temp
                GetWindowsDirectory(windowsDir, MAX_PATH);
                std::string winTemp = std::string(windowsDir) + "\\Temp\\*.*";
                hFind = FindFirstFile(winTemp.c_str(), &findData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                        do {
                                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
                                {
                                        std::string filePath = std::string(windowsDir) + std::string("\\Temp\\") + findData.cFileName;
                                        DeleteFile(filePath.c_str());
                                }
                        } while (FindNextFile(hFind, &findData));
                        FindClose(hFind);
                }
        }

        // 4. CLEAN THUMBNAIL CACHE
        {
                char localAppData[MAX_PATH];
                GetEnvironmentVariable("LOCALAPPDATA", localAppData, MAX_PATH);
                std::string thumbDir = std::string(localAppData) + "\\Microsoft\\Windows\\Explorer\\";
                WIN32_FIND_DATA findData;
                std::string searchPath = thumbDir + "thumbcache_*";
                HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                        do {
                                std::string filePath = thumbDir + findData.cFileName;
                                DeleteFile(filePath.c_str());
                        } while (FindNextFile(hFind, &findData));
                        FindClose(hFind);
                }
        }

        // 5. CLEAN OLD CONFIG TRACES (from previous versions that saved to disk)
        {
                char appData[MAX_PATH];
                GetEnvironmentVariable("APPDATA", appData, MAX_PATH);
                std::string oldConfigDir = std::string(appData) + "\\INTERIUM\\CS16Ext\\";
                WIN32_FIND_DATA findData;
                std::string searchPath = oldConfigDir + "*.json";
                HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                        do {
                                std::string filePath = oldConfigDir + findData.cFileName;
                                DeleteFile(filePath.c_str());
                        } while (FindNextFile(hFind, &findData));
                        FindClose(hFind);
                }
                // Remove the directories themselves if empty
                RemoveDirectory(oldConfigDir.c_str());
                std::string interiumDir = std::string(appData) + "\\INTERIUM\\";
                RemoveDirectory(interiumDir.c_str());
        }

        // 6. CLEAR REGISTRY TRACES
        {
                RegDeleteTreeA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU");
                RegDeleteTreeA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TypedPaths");
        }

        // 7. FLUSH SYSTEM CACHES - give Windows time to complete deletions
        Sleep(300);
}

// ===== Simple JSON Writer =====
class JsonWriter {
    std::ostringstream os;
    bool first = true;
    void comma() { if (!first) os << ","; first = false; }
public:
    void beginObject() { os << "{"; first = true; }
    void endObject() { os << "}"; }
    void beginArray() { os << "["; first = true; }
    void endArray() { os << "]"; }
    void key(const std::string& k) { comma(); os << "\"" << k << "\":"; }
    void valBool(bool v) { os << (v ? "true" : "false"); }
    void valInt(int v) { os << v; }
    void valFloat(float v) { os << v; }
    void valDouble(double v) { os << v; }
    std::string str() const { return os.str(); }
};

// ===== Simple JSON Reader =====
class JsonReader {
    std::map<std::string, std::string> data;
    static std::string trim(const std::string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        return s.substr(a, b - a + 1);
    }
    static std::string unquote(const std::string& s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
            return s.substr(1, s.size() - 2);
        return s;
    }
public:
    bool parse(const std::string& json) {
        data.clear();
        size_t i = 0;
        auto skipWS = [&]() { while (i < json.size() && (json[i]==' '||json[i]=='\t'||json[i]=='\r'||json[i]=='\n')) i++; };

        skipWS();
        if (i >= json.size() || json[i] != '{') return false;
        i++;

        while (i < json.size()) {
            skipWS();
            if (i >= json.size()) break;
            if (json[i] == '}') break;
            if (json[i] == ',') { i++; continue; }

            if (json[i] != '"') { i++; continue; }
            i++;
            std::string secKey;
            while (i < json.size() && json[i] != '"') { secKey += json[i]; i++; }
            if (i < json.size()) i++;

            skipWS();
            if (i >= json.size() || json[i] != ':') break;
            i++;
            skipWS();

            if (i < json.size() && json[i] == '{') {
                i++;
                while (i < json.size()) {
                    skipWS();
                    if (i >= json.size()) break;
                    if (json[i] == '}') { i++; break; }
                    if (json[i] == ',') { i++; continue; }

                    if (json[i] != '"') { i++; continue; }
                    i++;
                    std::string innerKey;
                    while (i < json.size() && json[i] != '"') { innerKey += json[i]; i++; }
                    if (i < json.size()) i++;

                    skipWS();
                    if (i >= json.size() || json[i] != ':') break;
                    i++;
                    skipWS();

                    std::string val;
                    if (i < json.size() && json[i] == '"') {
                        i++;
                        while (i < json.size() && json[i] != '"') { val += json[i]; i++; }
                        if (i < json.size()) i++;
                    } else {
                        while (i < json.size() && json[i] != ',' && json[i] != '}' && json[i] != ' ' && json[i] != '\n' && json[i] != '\r') {
                            val += json[i]; i++;
                        }
                    }
                    data[secKey + "." + innerKey] = trim(val);
                }
            } else {
                std::string val;
                if (i < json.size() && json[i] == '"') {
                    i++;
                    while (i < json.size() && json[i] != '"') { val += json[i]; i++; }
                    if (i < json.size()) i++;
                } else {
                    while (i < json.size() && json[i] != ',' && json[i] != '}' && json[i] != ' ' && json[i] != '\n' && json[i] != '\r') {
                        val += json[i]; i++;
                    }
                }
                data[secKey] = trim(val);
            }
        }
        return !data.empty();
    }

    bool getBool(const std::string& key, bool def = false) const {
        auto it = data.find(key);
        if (it == data.end()) return def;
        std::string v = trim(it->second);
        return (v == "true" || v == "1");
    }
    int getInt(const std::string& key, int def = 0) const {
        auto it = data.find(key);
        if (it == data.end()) return def;
        return atoi(trim(it->second).c_str());
    }
    float getFloat(const std::string& key, float def = 0.f) const {
        auto it = data.find(key);
        if (it == data.end()) return def;
        return (float)atof(trim(it->second).c_str());
    }
};

inline bool IsFileExists(const std::string& name)
{
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
}

// ===== Build JSON string from current settings =====
static std::string BuildConfigJson()
{
        JsonWriter w;
        w.beginObject();

        // AIMBOT
        w.key("aimbot"); w.beginObject();
        w.key("Enabled"); w.valBool(Aimbot::Enabled);
        w.key("Deathmatch"); w.valBool(Aimbot::Deathmatch);
        w.key("SquareFov"); w.valBool(Aimbot::SquareFov);
        w.key("DrawFov"); w.valBool(Aimbot::DrawFov);
        w.key("DrawFovColorR"); w.valFloat(Aimbot::DrawFovColor[0]);
        w.key("DrawFovColorG"); w.valFloat(Aimbot::DrawFovColor[1]);
        w.key("DrawFovColorB"); w.valFloat(Aimbot::DrawFovColor[2]);
        w.key("DrawFovColorA"); w.valFloat(Aimbot::DrawFovColor[3]);
        w.key("DrawFovWidth"); w.valFloat(Aimbot::DrawFovWidth);
        w.key("AimOffset"); w.valFloat(Aimbot::AimOffset);
        for (int i = 0; i < 40; i++)
        {
                w.key("Weapon" + std::to_string(i) + "Enable"); w.valBool(Weapons[i].Enabled);
                w.key("Weapon" + std::to_string(i) + "FOV"); w.valFloat(Weapons[i].FOV);
                w.key("Weapon" + std::to_string(i) + "Smooth"); w.valFloat(Weapons[i].Smootch);
                w.key("Weapon" + std::to_string(i) + "RCS"); w.valFloat(Weapons[i].RCS);
        }
        w.endObject();

        // ESP
        w.key("esp"); w.beginObject();
        w.key("Box"); w.valBool(ESP::Box);
        w.key("BoxType"); w.valInt(ESP::BoxType);
        w.key("BoxColorR"); w.valFloat(ESP::BoxColor[0]);
        w.key("BoxColorG"); w.valFloat(ESP::BoxColor[1]);
        w.key("BoxColorB"); w.valFloat(ESP::BoxColor[2]);
        w.key("BoxColorA"); w.valFloat(ESP::BoxColor[3]);
        w.key("BoxRounding"); w.valFloat(ESP::BoxRounding);
        w.key("BoxWidth"); w.valFloat(ESP::BoxWidth);
        w.key("BoxSize"); w.valFloat(ESP::BoxSize);
        w.key("ShowTeam"); w.valBool(ESP::ShowTeam);
        w.key("Dist"); w.valBool(ESP::Dist);
        w.key("DistColorR"); w.valFloat(ESP::DistColor[0]);
        w.key("DistColorG"); w.valFloat(ESP::DistColor[1]);
        w.key("DistColorB"); w.valFloat(ESP::DistColor[2]);
        w.key("DistColorA"); w.valFloat(ESP::DistColor[3]);
        w.key("Names"); w.valBool(ESP::Names);
        w.key("NamesColorR"); w.valFloat(ESP::NamesColor[0]);
        w.key("NamesColorG"); w.valFloat(ESP::NamesColor[1]);
        w.key("NamesColorB"); w.valFloat(ESP::NamesColor[2]);
        w.key("NamesColorA"); w.valFloat(ESP::NamesColor[3]);
        w.key("Crosshair"); w.valBool(ESP::Crosshair);
        w.key("CrosshairColorR"); w.valFloat(ESP::CrosshairColor[0]);
        w.key("CrosshairColorG"); w.valFloat(ESP::CrosshairColor[1]);
        w.key("CrosshairColorB"); w.valFloat(ESP::CrosshairColor[2]);
        w.key("CrosshairColorA"); w.valFloat(ESP::CrosshairColor[3]);
        w.key("CrosshairSize"); w.valFloat(ESP::CrosshairSize);
        w.key("CrosshairWidth"); w.valFloat(ESP::CrosshairWidth);
        w.endObject();

        // MISC
        w.key("misc"); w.beginObject();
        w.key("Bhop"); w.valBool(MISC::Bhop);
        w.key("DDrun"); w.valBool(MISC::DDrun);
        w.key("AutoPistol"); w.valBool(MISC::AutoPistol);
        w.key("FpsUnlock"); w.valBool(MISC::FpsUnlock);
        w.endObject();

        // TRIGGERBOT
        w.key("triggerbot"); w.beginObject();
        w.key("Enabled"); w.valBool(TRIGGERBOT::Enabled);
        w.key("Delay"); w.valInt(TRIGGERBOT::Delay);
        w.key("ShotDelay"); w.valInt(TRIGGERBOT::ShotDelay);
        w.key("Deathmatch"); w.valBool(TRIGGERBOT::Deathmatch);
        w.endObject();

        // KEYS
        w.key("keys"); w.beginObject();
        w.key("AimbotKey1"); w.valInt(KEYS::AimbotKey1);
        w.key("AimbotKey2"); w.valInt(KEYS::AimbotKey2);
        w.key("BhopKey"); w.valInt(KEYS::BhopKey);
        w.key("DDrunKey"); w.valInt(KEYS::DDrunKey);
        w.key("MenuKey"); w.valInt(KEYS::MenuKey);
        w.endObject();

        w.endObject();
        return w.str();
}

// ===== Apply JSON config to current settings =====
static bool ApplyConfigJson(const std::string& json)
{
        JsonReader r;
        if (!r.parse(json)) return false;

        // AIMBOT - defaults from current values (preserves Settings.h defaults)
        Aimbot::Enabled = r.getBool("aimbot.Enabled", Aimbot::Enabled);
        Aimbot::Deathmatch = r.getBool("aimbot.Deathmatch", Aimbot::Deathmatch);
        Aimbot::SquareFov = r.getBool("aimbot.SquareFov", Aimbot::SquareFov);
        Aimbot::DrawFov = r.getBool("aimbot.DrawFov", Aimbot::DrawFov);
        Aimbot::DrawFovColor[0] = r.getFloat("aimbot.DrawFovColorR", Aimbot::DrawFovColor[0]);
        Aimbot::DrawFovColor[1] = r.getFloat("aimbot.DrawFovColorG", Aimbot::DrawFovColor[1]);
        Aimbot::DrawFovColor[2] = r.getFloat("aimbot.DrawFovColorB", Aimbot::DrawFovColor[2]);
        Aimbot::DrawFovColor[3] = r.getFloat("aimbot.DrawFovColorA", Aimbot::DrawFovColor[3]);
        Aimbot::DrawFovWidth = r.getFloat("aimbot.DrawFovWidth", Aimbot::DrawFovWidth);
        Aimbot::AimOffset = r.getFloat("aimbot.AimOffset", Aimbot::AimOffset);

        for (int i = 0; i < 40; i++)
        {
                Weapons[i].Enabled = r.getBool("aimbot.Weapon" + std::to_string(i) + "Enable", Weapons[i].Enabled);
                Weapons[i].FOV = r.getFloat("aimbot.Weapon" + std::to_string(i) + "FOV", Weapons[i].FOV);
                Weapons[i].Smootch = r.getFloat("aimbot.Weapon" + std::to_string(i) + "Smooth", Weapons[i].Smootch);
                Weapons[i].RCS = r.getFloat("aimbot.Weapon" + std::to_string(i) + "RCS", Weapons[i].RCS);
        }

        // ESP - defaults from current values (Box=true, BoxType=0, BoxSize=1.0 from Settings.h)
        ESP::Box = r.getBool("esp.Box", ESP::Box);
        ESP::BoxType = r.getInt("esp.BoxType", ESP::BoxType);
        ESP::BoxColor[0] = r.getFloat("esp.BoxColorR", ESP::BoxColor[0]);
        ESP::BoxColor[1] = r.getFloat("esp.BoxColorG", ESP::BoxColor[1]);
        ESP::BoxColor[2] = r.getFloat("esp.BoxColorB", ESP::BoxColor[2]);
        ESP::BoxColor[3] = r.getFloat("esp.BoxColorA", ESP::BoxColor[3]);
        ESP::BoxRounding = r.getFloat("esp.BoxRounding", ESP::BoxRounding);
        ESP::BoxWidth = r.getFloat("esp.BoxWidth", ESP::BoxWidth);
        ESP::BoxSize = r.getFloat("esp.BoxSize", ESP::BoxSize);
        ESP::ShowTeam = r.getBool("esp.ShowTeam", ESP::ShowTeam);
        ESP::Dist = r.getBool("esp.Dist", ESP::Dist);
        ESP::DistColor[0] = r.getFloat("esp.DistColorR", ESP::DistColor[0]);
        ESP::DistColor[1] = r.getFloat("esp.DistColorG", ESP::DistColor[1]);
        ESP::DistColor[2] = r.getFloat("esp.DistColorB", ESP::DistColor[2]);
        ESP::DistColor[3] = r.getFloat("esp.DistColorA", ESP::DistColor[3]);
        ESP::Names = r.getBool("esp.Names", ESP::Names);
        ESP::NamesColor[0] = r.getFloat("esp.NamesColorR", ESP::NamesColor[0]);
        ESP::NamesColor[1] = r.getFloat("esp.NamesColorG", ESP::NamesColor[1]);
        ESP::NamesColor[2] = r.getFloat("esp.NamesColorB", ESP::NamesColor[2]);
        ESP::NamesColor[3] = r.getFloat("esp.NamesColorA", ESP::NamesColor[3]);
        ESP::Crosshair = r.getBool("esp.Crosshair", ESP::Crosshair);
        ESP::CrosshairColor[0] = r.getFloat("esp.CrosshairColorR", ESP::CrosshairColor[0]);
        ESP::CrosshairColor[1] = r.getFloat("esp.CrosshairColorG", ESP::CrosshairColor[1]);
        ESP::CrosshairColor[2] = r.getFloat("esp.CrosshairColorB", ESP::CrosshairColor[2]);
        ESP::CrosshairColor[3] = r.getFloat("esp.CrosshairColorA", ESP::CrosshairColor[3]);
        ESP::CrosshairSize = r.getFloat("esp.CrosshairSize", ESP::CrosshairSize);
        ESP::CrosshairWidth = r.getFloat("esp.CrosshairWidth", ESP::CrosshairWidth);

        // MISC
        MISC::Bhop = r.getBool("misc.Bhop", MISC::Bhop);
        MISC::DDrun = r.getBool("misc.DDrun", MISC::DDrun);
        MISC::AutoPistol = r.getBool("misc.AutoPistol", MISC::AutoPistol);
        MISC::FpsUnlock = r.getBool("misc.FpsUnlock", MISC::FpsUnlock);

        // TRIGGERBOT
        TRIGGERBOT::Enabled = r.getBool("triggerbot.Enabled", TRIGGERBOT::Enabled);
        TRIGGERBOT::Delay = r.getInt("triggerbot.Delay", TRIGGERBOT::Delay);
        TRIGGERBOT::ShotDelay = r.getInt("triggerbot.ShotDelay", TRIGGERBOT::ShotDelay);
        TRIGGERBOT::Deathmatch = r.getBool("triggerbot.Deathmatch", TRIGGERBOT::Deathmatch);

        // KEYS
        KEYS::AimbotKey1 = r.getInt("keys.AimbotKey1", KEYS::AimbotKey1);
        KEYS::AimbotKey2 = r.getInt("keys.AimbotKey2", KEYS::AimbotKey2);
        KEYS::BhopKey = r.getInt("keys.BhopKey", KEYS::BhopKey);
        KEYS::DDrunKey = r.getInt("keys.DDrunKey", KEYS::DDrunKey);
        KEYS::MenuKey = r.getInt("keys.MenuKey", KEYS::MenuKey);

        return true;
}

// ===== Extract clean config name (remove path and extension) =====
static std::string CleanConfigName(const std::string& rawName)
{
        std::string name = rawName;
        size_t lastSlash = name.find_last_of("\\/");
        if (lastSlash != std::string::npos) name = name.substr(lastSlash + 1);
        size_t lastDot = name.find_last_of(".");
        if (lastDot != std::string::npos) name = name.substr(0, lastDot);
        return name;
}

// ===== Refresh config list (from memory only) =====
void RefreshSettings()
{
        SettingsList.clear();
        for (auto& pair : g_MemoryConfigs)
        {
                SettingsList.push_back(pair.first);
        }
}

// ===== SaveConfig: saves to RAM only (NO DISK WRITE) =====
void SaveConfig(std::string ConfigName)
{
        std::string name = CleanConfigName(ConfigName);
        if (name.empty()) name = "default";
        g_MemoryConfigs[name] = BuildConfigJson();
}

// ===== LoadConfig: loads from RAM only (NO DISK READ) =====
bool LoadConfig(std::string ConfigName)
{
        std::string name = CleanConfigName(ConfigName);
        auto it = g_MemoryConfigs.find(name);
        if (it == g_MemoryConfigs.end()) return false;
        return ApplyConfigJson(it->second);
}

// ===== DeleteConfig: removes from RAM only =====
void DeleteConfig(std::string ConfigName)
{
        std::string name = CleanConfigName(ConfigName);
        g_MemoryConfigs.erase(name);
}

// ===== ExportConfigToFile: USER-INITIATED, writes to disk =====
void ExportConfigToFile(std::string FilePath)
{
        std::ofstream ofs(FilePath);
        if (!ofs.is_open()) return;
        std::string raw = BuildConfigJson();
        int indent = 0;
        for (size_t j = 0; j < raw.size(); j++) {
                char c = raw[j];
                if (c == '{' || c == '[') { ofs << c << "\n"; indent += 2; for (int k = 0; k < indent; k++) ofs << ' '; }
                else if (c == '}' || c == ']') { ofs << "\n"; indent -= 2; for (int k = 0; k < indent; k++) ofs << ' '; ofs << c; }
                else if (c == ',') { ofs << ",\n"; for (int k = 0; k < indent; k++) ofs << ' '; }
                else if (c == ':') { ofs << ": "; }
                else { ofs << c; }
        }
        ofs << "\n";
        ofs.close();
}

// ===== ImportConfigFromFile: USER-INITIATED, reads from disk into RAM =====
bool ImportConfigFromFile(std::string FilePath, std::string ConfigName)
{
        std::ifstream ifs(FilePath);
        if (!ifs.is_open()) return false;
        std::stringstream ss;
        ss << ifs.rdbuf();
        ifs.close();
        std::string name = CleanConfigName(ConfigName);
        if (name.empty()) name = "imported";
        g_MemoryConfigs[name] = ss.str();
        return true;
}
