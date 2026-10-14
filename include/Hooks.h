#pragma once

class Hooks {
public:
    static void Init();

private:
    static void ApplyCameraPatches();
    static void ApplyMovementPatches();
    static void ApplyWeaponPatches();
    static void ApplyMenuPatches();
    static void ApplyHudPatches();
    static void RegisterEvents();
};
