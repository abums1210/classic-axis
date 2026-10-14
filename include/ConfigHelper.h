#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>

class ConfigHelper {
private:
    std::string m_path;
    std::map<std::string, std::string> m_comments;

    void StoreComment(const char* sec, const char* key, const char* comment);

public:
    ConfigHelper(const std::string& iniPath);

    int ReadInt(const char* sec, const char* key, int def);
    float ReadFloat(const char* sec, const char* key, float def);
    bool ReadBool(const char* sec, const char* key, bool def);
    std::string ReadString(const char* sec, const char* key, const std::string& def);

    void WriteInt(const char* sec, const char* key, int val, const char* comment = nullptr);
    void WriteFloat(const char* sec, const char* key, float val, const char* comment = nullptr);
    void WriteBool(const char* sec, const char* key, bool val, const char* comment = nullptr);
    void WriteString(const char* sec, const char* key, const std::string& val, const char* comment = nullptr);

    bool Exists();
    void CreateParentFolder();
    void Format(const std::string& headerComments = "");
};
