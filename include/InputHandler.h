#pragma once
#include <string>
#include <cstdint>

class InputHandler {
public:
    static int32_t StringToKey(const std::string& str);
    static bool GetKeyDown(int32_t key, bool old = false);
    static bool WalkKeyDown();
#ifdef GTA3
    static bool DuckKeyDown();
#endif
    static bool HasPadInHands();
    static void InitGInput();
};
