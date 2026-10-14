#include "ConfigHelper.h"
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

void ConfigHelper::Format(const std::string& headerComments) {
    FILE* f;
    char lineBuffer[1024];
    std::string outContent, currentSection, line, trimmed, key, compKey;
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
                outContent += "; " + m_comments[compKey] + "\n";
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
