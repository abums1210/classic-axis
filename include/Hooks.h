#pragma once

class Hooks {
public:
    static void Init();

private:
    static void RegisterCoreEvents();
    static void ApplyCameraMovementAndStancePatches();
    static void ApplyCameraModeAndControlPatches();
    static void ApplyHudAndMenuPatches();
    static void ApplyWeaponCycleAndDuckingPatches();
    static void ApplyAimingArmAndTargetVectorPatches();
};
