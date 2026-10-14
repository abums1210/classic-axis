#pragma once
#include <string>

enum eLockOnTargetType {
    TARGET_DEFAULT,
    TARGET_SA,
    TARGET_LCS
};

class Configs {
public:
    bool forceAutoAim, showTriangleForMouseRecruit, storiesAimingCoords;
    bool storiesPointingArm, zoomForAssaultRifles, modernCamera;
    int lockOnTargetType, reloadSettings;
    float cameraCrosshairMultX, cameraCrosshairMultY;
    float rightAnalogStickSensitivityX, rightAnalogStickSensitivityY;
    std::string walkKey;

#ifdef GTA3
    std::string crouchKey;
#endif

    void Read();
    void Save();
};

extern Configs configs;
