#pragma once
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "Settings.h"

static std::vector<std::string> SettingsList;
static bool IsConfigSelected[50];
static int SelectedConfig = 0;
static char cfgname[25] = "";

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
    void valBool(bool v) { comma(); os << (v ? "true" : "false"); }
    void valInt(int v) { comma(); os << v; }
    void valFloat(float v) { comma(); os << v; }
    void valDouble(double v) { comma(); os << v; }
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
        // Simple flat key-value parser for {"key":value,...} and nested {"section":{"key":value,...}}
        size_t i = 0;
        auto skipWS = [&]() { while (i < json.size() && (json[i]==' '||json[i]=='\t'||json[i]=='\r'||json[i]=='\n'||json[i]=='\n')) i++; };
        
        skipWS();
        if (i >= json.size() || json[i] != '{') return false;
        i++; // skip {
        
        while (i < json.size()) {
            skipWS();
            if (i >= json.size()) break;
            if (json[i] == '}') break;
            if (json[i] == ',') { i++; continue; }
            
            // Read key
            if (json[i] != '"') { i++; continue; }
            i++; // skip opening "
            std::string secKey;
            while (i < json.size() && json[i] != '"') { secKey += json[i]; i++; }
            if (i < json.size()) i++; // skip closing "
            
            skipWS();
            if (i >= json.size() || json[i] != ':') break;
            i++; // skip :
            skipWS();
            
            if (i < json.size() && json[i] == '{') {
                // Nested object: section
                i++; // skip {
                while (i < json.size()) {
                    skipWS();
                    if (i >= json.size()) break;
                    if (json[i] == '}') { i++; break; }
                    if (json[i] == ',') { i++; continue; }
                    
                    if (json[i] != '"') { i++; continue; }
                    i++; // skip opening "
                    std::string innerKey;
                    while (i < json.size() && json[i] != '"') { innerKey += json[i]; i++; }
                    if (i < json.size()) i++; // skip closing "
                    
                    skipWS();
                    if (i >= json.size() || json[i] != ':') break;
                    i++; // skip :
                    skipWS();
                    
                    // Read value
                    std::string val;
                    if (i < json.size() && json[i] == '"') {
                        i++; // skip opening "
                        while (i < json.size() && json[i] != '"') { val += json[i]; i++; }
                        if (i < json.size()) i++; // skip closing "
                    } else {
                        while (i < json.size() && json[i] != ',' && json[i] != '}' && json[i] != ' ' && json[i] != '\n' && json[i] != '\r') {
                            val += json[i]; i++;
                        }
                    }
                    data[secKey + "." + innerKey] = trim(val);
                }
            } else {
                // Flat value
                std::string val;
                if (i < json.size() && json[i] == '"') {
                    i++; // skip opening "
                    while (i < json.size() && json[i] != '"') { val += json[i]; i++; }
                    if (i < json.size()) i++; // skip closing "
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

void getFilesList(std::string filePath, std::string extension, std::vector<std::string>& returnFileName)
{
        WIN32_FIND_DATA fileInfo;
        HANDLE hFind;
        std::string  fullPath = filePath + extension;
        hFind = FindFirstFile(fullPath.c_str(), &fileInfo);
        if (hFind != INVALID_HANDLE_VALUE)
        {
                returnFileName.push_back(filePath + fileInfo.cFileName);
                while (FindNextFile(hFind, &fileInfo) != 0)
                {
                        returnFileName.push_back(filePath + fileInfo.cFileName);
                }
        }
}

void RefreshSettings()
{
        memset(&SettingsList, 0, sizeof(SettingsList));

        std::string configDir = std::string(getenv("appdata")) + std::string("\\INTERIUM\\CS16Ext\\");

        std::vector<std::string> filesPaths;
        getFilesList(configDir, "*.json*", filesPaths);
        std::vector<std::string>::const_iterator it = filesPaths.begin();
        while (it != filesPaths.end())
        {
                std::string outpath = it->c_str();
                SettingsList.push_back(outpath);
                it++;
        }
}

void SaveConfig(std::string ConfigName)
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

        std::ofstream ofs(ConfigName);
        // Write pretty JSON with indentation
        std::string raw = w.str();
        int indent = 0;
        for (size_t j = 0; j < raw.size(); j++) {
                char c = raw[j];
                if (c == '{' || c == '[') {
                        ofs << c << "\n";
                        indent += 2;
                        for (int k = 0; k < indent; k++) ofs << ' ';
                } else if (c == '}' || c == ']') {
                        ofs << "\n";
                        indent -= 2;
                        for (int k = 0; k < indent; k++) ofs << ' ';
                        ofs << c;
                } else if (c == ',') {
                        ofs << ",\n";
                        for (int k = 0; k < indent; k++) ofs << ' ';
                } else if (c == ':') {
                        ofs << ": ";
                } else {
                        ofs << c;
                }
        }
        ofs << "\n";
        ofs.close();
}

bool LoadConfig(std::string ConfigName)
{
        if (!IsFileExists(ConfigName)) return false;

        std::ifstream ifs(ConfigName);
        if (!ifs.is_open()) return false;

        std::stringstream ss;
        ss << ifs.rdbuf();
        ifs.close();

        JsonReader r;
        if (!r.parse(ss.str())) return false;

        // AIMBOT
        Aimbot::Enabled = r.getBool("aimbot.Enabled", true);
        Aimbot::Deathmatch = r.getBool("aimbot.Deathmatch", true);
        Aimbot::SquareFov = r.getBool("aimbot.SquareFov", true);
        Aimbot::DrawFov = r.getBool("aimbot.DrawFov", true);
        Aimbot::DrawFovColor[0] = r.getFloat("aimbot.DrawFovColorR", 1.f);
        Aimbot::DrawFovColor[1] = r.getFloat("aimbot.DrawFovColorG", 1.f);
        Aimbot::DrawFovColor[2] = r.getFloat("aimbot.DrawFovColorB", 1.f);
        Aimbot::DrawFovColor[3] = r.getFloat("aimbot.DrawFovColorA", 1.f);
        Aimbot::DrawFovWidth = r.getFloat("aimbot.DrawFovWidth", 1.f);

        for (int i = 0; i < 40; i++)
        {
                Weapons[i].Enabled = r.getBool("aimbot.Weapon" + std::to_string(i) + "Enable", false);
                Weapons[i].FOV = r.getFloat("aimbot.Weapon" + std::to_string(i) + "FOV", 12.5f);
                Weapons[i].Smootch = r.getFloat("aimbot.Weapon" + std::to_string(i) + "Smooth", 5.f);
                Weapons[i].RCS = r.getFloat("aimbot.Weapon" + std::to_string(i) + "RCS", 4.f);
        }

        // ESP
        ESP::Box = r.getBool("esp.Box", false);
        ESP::BoxType = r.getInt("esp.BoxType", 0);
        ESP::BoxColor[0] = r.getFloat("esp.BoxColorR", 1.f);
        ESP::BoxColor[1] = r.getFloat("esp.BoxColorG", 1.f);
        ESP::BoxColor[2] = r.getFloat("esp.BoxColorB", 1.f);
        ESP::BoxColor[3] = r.getFloat("esp.BoxColorA", 1.f);
        ESP::BoxRounding = r.getFloat("esp.BoxRounding", 1.f);
        ESP::BoxWidth = r.getFloat("esp.BoxWidth", 1.f);
        ESP::ShowTeam = r.getBool("esp.ShowTeam", true);
        ESP::Dist = r.getBool("esp.Dist", true);
        ESP::DistColor[0] = r.getFloat("esp.DistColorR", 1.f);
        ESP::DistColor[1] = r.getFloat("esp.DistColorG", 1.f);
        ESP::DistColor[2] = r.getFloat("esp.DistColorB", 1.f);
        ESP::DistColor[3] = r.getFloat("esp.DistColorA", 1.f);
        ESP::Names = r.getBool("esp.Names", true);
        ESP::NamesColor[0] = r.getFloat("esp.NamesColorR", 1.f);
        ESP::NamesColor[1] = r.getFloat("esp.NamesColorG", 1.f);
        ESP::NamesColor[2] = r.getFloat("esp.NamesColorB", 1.f);
        ESP::NamesColor[3] = r.getFloat("esp.NamesColorA", 1.f);
        ESP::Crosshair = r.getBool("esp.Crosshair", true);
        ESP::CrosshairColor[0] = r.getFloat("esp.CrosshairColorR", 1.f);
        ESP::CrosshairColor[1] = r.getFloat("esp.CrosshairColorG", 0.f);
        ESP::CrosshairColor[2] = r.getFloat("esp.CrosshairColorB", 0.f);
        ESP::CrosshairColor[3] = r.getFloat("esp.CrosshairColorA", 1.f);
        ESP::CrosshairSize = r.getFloat("esp.CrosshairSize", 5.f);
        ESP::CrosshairWidth = r.getFloat("esp.CrosshairWidth", 1.f);

        // MISC
        MISC::Bhop = r.getBool("misc.Bhop", false);
        MISC::DDrun = r.getBool("misc.DDrun", false);
        MISC::AutoPistol = r.getBool("misc.AutoPistol", false);
        MISC::FpsUnlock = r.getBool("misc.FpsUnlock", false);

        // TRIGGERBOT
        TRIGGERBOT::Enabled = r.getBool("triggerbot.Enabled", false);
        TRIGGERBOT::Delay = r.getInt("triggerbot.Delay", 50);
        TRIGGERBOT::ShotDelay = r.getInt("triggerbot.ShotDelay", 100);
        TRIGGERBOT::Deathmatch = r.getBool("triggerbot.Deathmatch", true);

        // KEYS
        KEYS::AimbotKey1 = r.getInt("keys.AimbotKey1", VK_LBUTTON);
        KEYS::AimbotKey2 = r.getInt("keys.AimbotKey2", 0x56);
        KEYS::BhopKey = r.getInt("keys.BhopKey", VK_SPACE);
        KEYS::DDrunKey = r.getInt("keys.DDrunKey", VK_MENU);
        KEYS::MenuKey = r.getInt("keys.MenuKey", VK_INSERT);

        return true;
}
