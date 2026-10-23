#include "ConfigHelper.h"
#include "Configs.h"
#include "Crosshairs.h"
#include <windows.h>
#include <stdio.h>
#include <fstream>
#include <sstream>

ConfigHelper::ConfigHelper(const std::string& iniPath) : m_path(iniPath) {}

bool ConfigHelper::Exists() {
    DWORD attr;
    attr = GetFileAttributesA(m_path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

void ConfigHelper::CreateParentFolder() {
    size_t lastSlash;
    std::string dir;

    lastSlash = m_path.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        dir = m_path.substr(0, lastSlash);
        CreateDirectoryA(dir.c_str(), NULL);
    }
}

void ConfigHelper::StoreComment(const char* sec, const char* key, const char* comment) {
    std::string compKey;

    if (!comment || !comment[0])
        return;

    compKey = std::string(sec) + "|" + std::string(key);
    m_comments[compKey] = comment;
}

int ConfigHelper::ReadInt(const char* sec, const char* key, int def) {
    return GetPrivateProfileIntA(sec, key, def, m_path.c_str());
}

float ConfigHelper::ReadFloat(const char* sec, const char* key, float def) {
    char buf[64];

    if (GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), m_path.c_str()) > 0)
        return static_cast<float>(atof(buf));

    return def;
}

bool ConfigHelper::ReadBool(const char* sec, const char* key, bool def) {
    char buf[32];
    int intVal;

    if (GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), m_path.c_str()) > 0) {
        if (_stricmp(buf, "true") == 0 || _stricmp(buf, "yes") == 0 || _stricmp(buf, "on") == 0)
            return true;
        if (_stricmp(buf, "false") == 0 || _stricmp(buf, "no") == 0 || _stricmp(buf, "off") == 0)
            return false;

        intVal = atoi(buf);
        return intVal != 0;
    }

    return def;
}

std::string ConfigHelper::ReadString(const char* sec, const char* key, const std::string& def) {
    char buf[512];

    if (GetPrivateProfileStringA(sec, key, def.c_str(), buf, sizeof(buf), m_path.c_str()) > 0)
        return std::string(buf);

    return def;
}

void ConfigHelper::WriteInt(const char* sec, const char* key, int val, const char* comment) {
    char buf[32];

    sprintf_s(buf, "%d", val);
    WritePrivateProfileStringA(sec, key, buf, m_path.c_str());
    StoreComment(sec, key, comment);
}

void ConfigHelper::WriteFloat(const char* sec, const char* key, float val, const char* comment) {
    char buf[32];

    sprintf_s(buf, "%.2f", val);
    WritePrivateProfileStringA(sec, key, buf, m_path.c_str());
    StoreComment(sec, key, comment);
}

void ConfigHelper::WriteBool(const char* sec, const char* key, bool val, const char* comment) {
    WritePrivateProfileStringA(sec, key, val ? "true" : "false", m_path.c_str());
    StoreComment(sec, key, comment);
}

void ConfigHelper::WriteString(const char* sec, const char* key, const std::string& val, const char* comment) {
    WritePrivateProfileStringA(sec, key, val.c_str(), m_path.c_str());
    StoreComment(sec, key, comment);
}

bool ConfigHelper::ReadVector3(const char* sec, const char* key, float& outX, float& outY, float& outZ) {
    std::string line, clean;
    size_t i;
    float x, y, z;
    int matches;

    line = ReadString(sec, key, "");
    if (line.empty())
        return false;

    clean = line;
    for (i = 0; i < clean.size(); i++) {
        if (clean[i] == ',')
            clean[i] = ' ';
    }

    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    matches = sscanf(clean.c_str(), " %f %f %f", &x, &y, &z);
    if (matches == 3) {
        outX = x;
        outY = y;
        outZ = z;
        return true;
    }
    return false;
}

void ConfigHelper::WriteVector3(const char* sec, const char* key, float x, float y, float z, const char* comment) {
    char buf[128];

    sprintf_s(buf, "%.8f, %.8f, %.8f", x, y, z);
    WriteString(sec, key, buf, comment);
}

bool ConfigHelper::ReadCrosshair(const char* sec, const char* key, CrosshairConfig& target) {
    std::string line, clean;
    size_t i;
    char sprBuf[64];
    float s, k;
    int matches;

    line = ReadString(sec, key, "");
    if (line.empty())
        return false;

    clean = line;
    for (i = 0; i < clean.size(); i++) {
        if (clean[i] == ',')
            clean[i] = ' ';
    }

    sprBuf[0] = '\0';
    s = target.scale;
    k = target.recoilKick;
    matches = sscanf(clean.c_str(), " %f %f %63s", &s, &k, sprBuf);
    if (matches >= 1) {
        target.scale = s;
        if (matches >= 2)
            target.recoilKick = k;
        if (matches >= 3)
            target.sprite = Crosshairs::StringToSprite(sprBuf);
        return true;
    }
    return false;
}

void ConfigHelper::WriteCrosshair(const char* sec, const char* key, const CrosshairConfig& target, const char* comment) {
    char buf[128];

    sprintf_s(buf, "%.2f,  %.2f,       %s", target.scale, target.recoilKick, Crosshairs::SpriteToString(target.sprite));
    WriteString(sec, key, buf, comment);
}

void ConfigHelper::Format(const std::string& headerComments) {
    FILE* f;
    char lineBuffer[1024];
    std::string outContent, currentSection, line, trimmed, key, compKey, cmt, cmtLine;
    std::stringstream ss;
    size_t eqPos, start, end;

    f = nullptr;
    fopen_s(&f, m_path.c_str(), "r");
    if (!f)
        return;

    if (!headerComments.empty()) {
        outContent += headerComments;
        if (headerComments.back() != '\n')
            outContent += "\n";
    }

    while (fgets(lineBuffer, sizeof(lineBuffer), f)) {
        line = lineBuffer;
        start = line.find_first_not_of(" \t\r\n");

        if (start == std::string::npos) {
            outContent += "\n";
            continue;
        }

        end = line.find_last_not_of(" \t\r\n");
        trimmed = line.substr(start, end - start + 1);

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = trimmed.substr(1, trimmed.length() - 2);
            outContent += "\n" + trimmed + "\n";
            continue;
        }

        if (trimmed.front() == ';' || trimmed.front() == '#') {
            outContent += trimmed + "\n";
            continue;
        }

        eqPos = trimmed.find('=');
        if (eqPos != std::string::npos) {
            key = trimmed.substr(0, eqPos);
            end = key.find_last_not_of(" \t");
            if (end != std::string::npos)
                key = key.substr(0, end + 1);

            compKey = currentSection + "|" + key;
            if (m_comments.find(compKey) != m_comments.end()) {
                cmt = m_comments[compKey];
                ss.clear();
                ss.str(cmt);
                while (std::getline(ss, cmtLine)) {
                    start = cmtLine.find_first_not_of(" \t");
                    if (start == std::string::npos) {
                        outContent += ";\n";
                    }
                    else if (cmtLine[start] == ';' || cmtLine[start] == '#') {
                        outContent += cmtLine + "\n";
                    }
                    else {
                        outContent += "; " + cmtLine + "\n";
                    }
                }
                m_comments.erase(compKey);
            }
        }

        outContent += trimmed + "\n";
    }
    fclose(f);

    fopen_s(&f, m_path.c_str(), "w");
    if (f) {
        fputs(outContent.c_str(), f);
        fclose(f);
    }
}
