#pragma once
#include <string>
#include "eWeaponType.h"

enum eLockOnTargetType {
    TARGET_DEFAULT,
    TARGET_SA,
    TARGET_LCS
};

enum eCrosshairSprite {
    CROSSHAIR_ASSAULT,
    CROSSHAIR_CROSS,
    CROSSHAIR_DOT,
    CROSSHAIR_LASERDOT,
    CROSSHAIR_SHOTGUN,
    CROSSHAIR_SITELASER,
    CROSSHAIR_SITEM16,
    CROSSHAIR_SITEROCKET,
    CROSSHAIR_SITESNIPER,
    CROSSHAIR_SMG,
    TOTAL_CROSSHAIRS
};

struct CrosshairConfig {
    float scale;
    float recoilKick;
    int sprite;
};

struct CrosshairMapping {
    int weaponType;
    const char* weaponName;
    float defaultScale;
    float defaultRecoilKick;
    int defaultSprite;
};

class Configs {
public:
    bool forceAutoAim, showTriangleForMouseRecruit, storiesAimingCoords;
    bool storiesPointingArm, zoomForAssaultRifles, modernCamera;
    bool persistentCrosshairDot, animatedCrosshairs, killEffect, thirdPersonRocketLauncher;
    int lockOnTargetType, reloadSettings, crosshairType, hitIndicator, killEffectDuration;
    float cameraCrosshairMultX, cameraCrosshairMultY;
    float rightAnalogStickSensitivityX, rightAnalogStickSensitivityY;
    std::string walkKey, crosshairFolderPath;

#ifdef GTA3
    std::string crouchKey;
    float rocketAimPitch;
    float rocketAimRotPitch, rocketAimRotYaw, rocketAimRotRoll;
    float rocketAimOffsetX, rocketAimOffsetY, rocketAimOffsetZ;
#endif

    float dotScale, hitIndicatorScale, maxRecoilKick, recoilRecoveryRate;
    CrosshairConfig weaponCrosshairs[64];
    CrosshairConfig defaultCrosshair;

    void ReadSettings();
    void WriteSettings();
    void ReadCrosshairInfo(const std::string& folderPath);
    void WriteCrosshairInfo(const std::string& folderPath);
    const CrosshairConfig& GetCrosshair(eWeaponType weaponType);
};

extern Configs configs;
