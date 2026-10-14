#include "Configs.h"
#include "Hooks.h"

#pragma optimize("", off)

class ClassicAxis {
public:
    ClassicAxis() {
        configs.Read();
        Hooks::Init();
    }
} classicAxis;
