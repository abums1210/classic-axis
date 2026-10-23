#include "Configs.h"
#include "ConfigHelper.h"
#include "Aiming.h"
#include "plugin.h"

Configs configs;

static const char* SECTION_NAME = "ClassicAxis";

static std::string GetConfigPath() {
#ifdef GTA3
    return PLUGIN_PATH("ClassicAxisIII.ini");
#else
    return PLUGIN_PATH("ClassicAxisVC.ini");
#endif
}


#ifdef GTAVC
static const CrosshairMapping defaultCrosshairMappings[] = {
    { WEAPONTYPE_PISTOL, "Colt45", 0.38f, 0.07f, CROSSHAIR_SITEM16 },
    { WEAPONTYPE_PYTHON, "Python", 0.38f, 0.14f, CROSSHAIR_SITEM16 },
    { WEAPONTYPE_SHOTGUN, "Shotgun", 0.65f, 0.16f, CROSSHAIR_SHOTGUN },
    { WEAPONTYPE_SPAS12_SHOTGUN, "Spas12Shotgun", 0.60f, 0.16f, CROSSHAIR_SHOTGUN },
    { WEAPONTYPE_STUBBY_SHOTGUN, "StubbyShotgun", 0.70f, 0.16f, CROSSHAIR_SHOTGUN },
    { WEAPONTYPE_TEC9, "Tec9", 0.40f, 0.06f, CROSSHAIR_SMG },
    { WEAPONTYPE_UZI, "Uzi", 0.40f, 0.06f, CROSSHAIR_SMG },
    { WEAPONTYPE_UZI_DRIVEBY, "UziDriveby", 0.40f, 0.06f, CROSSHAIR_SMG },
    { WEAPONTYPE_SILENCED_INGRAM, "SilencedIngram", 0.40f, 0.06f, CROSSHAIR_SMG },
    { WEAPONTYPE_MP5, "Mp5", 0.40f, 0.06f, CROSSHAIR_SMG },
    { WEAPONTYPE_M4, "M4", 0.38f, 0.08f, CROSSHAIR_ASSAULT },
    { WEAPONTYPE_RUGER, "Ruger", 0.38f, 0.09f, CROSSHAIR_ASSAULT },
    { WEAPONTYPE_M60, "M60", 0.38f, 0.10f, CROSSHAIR_ASSAULT },
    { WEAPONTYPE_MINIGUN, "Minigun", 0.38f, 0.06f, CROSSHAIR_SITEM16 },
    { WEAPONTYPE_ROCKETLAUNCHER, "RocketLauncher", 0.38f, 0.08f, CROSSHAIR_SITEROCKET },
};
#else
static const CrosshairMapping defaultCrosshairMappings[] = {
    { WEAPONTYPE_COLT45, "Colt45", 0.38f, 0.07f, CROSSHAIR_SITEM16 },
    { WEAPONTYPE_UZI, "Uzi", 0.40f, 0.06f, CROSSHAIR_SMG },
    { WEAPONTYPE_UZI_DRIVEBY, "UziDriveby", 0.40f, 0.06f, CROSSHAIR_SMG },
    { WEAPONTYPE_SHOTGUN, "Shotgun", 0.65f, 0.16f, CROSSHAIR_SHOTGUN },
    { WEAPONTYPE_AK47, "AK47", 0.38f, 0.08f, CROSSHAIR_ASSAULT },
    { WEAPONTYPE_M16, "M16", 0.38f, 0.08f, CROSSHAIR_ASSAULT },
    { WEAPONTYPE_ROCKETLAUNCHER, "RocketLauncher", 0.38f, 0.08f, CROSSHAIR_SITEROCKET },
};
#endif




void Configs::ReadSettings() {
    std::string path;
    path = GetConfigPath();
    ConfigHelper cfg(path);

    if (!cfg.Exists()) {
        forceAutoAim = false;
        lockOnTargetType = TARGET_SA;
        showTriangleForMouseRecruit = true;
        walkKey = "LALT";
        storiesAimingCoords = false;
        cameraCrosshairMultX = 0.53f;
        cameraCrosshairMultY = 0.4f;
        storiesPointingArm = false;
        rightAnalogStickSensitivityX = 1.0f;
        rightAnalogStickSensitivityY = 1.0f;
#ifdef GTA3
        zoomForAssaultRifles = true;
        crouchKey = "C";
        rocketAimPitch = -45.0f;
#else
        zoomForAssaultRifles = false;
#endif
        modernCamera = true;
        reloadSettings = 0;
        crosshairType = 2;
        persistentCrosshairDot = false;
        hitIndicator = 2;
        animatedCrosshairs = true;
        killEffect = true;
        killEffectDuration = 400;
        thirdPersonRocketLauncher = true;

        WriteSettings();
        return;
    }

    forceAutoAim = cfg.ReadBool(SECTION_NAME, "ForceAutoAim", false);
    lockOnTargetType = cfg.ReadInt(SECTION_NAME, "LockOnTargetType", TARGET_SA);
    showTriangleForMouseRecruit = cfg.ReadBool(SECTION_NAME, "ShowTriangleForMouseRecruit", true);
    walkKey = cfg.ReadString(SECTION_NAME, "WalkKey", "LALT");
    storiesAimingCoords = cfg.ReadBool(SECTION_NAME, "StoriesAimingCoords", false);
    cameraCrosshairMultX = cfg.ReadFloat(SECTION_NAME, "CameraCrosshairMultX", 0.53f);
    cameraCrosshairMultY = cfg.ReadFloat(SECTION_NAME, "CameraCrosshairMultY", 0.4f);
    storiesPointingArm = cfg.ReadBool(SECTION_NAME, "StoriesPointingArm", false);
    rightAnalogStickSensitivityX = cfg.ReadFloat(SECTION_NAME, "RightAnalogStickSensitivityX", 1.0f);
    rightAnalogStickSensitivityY = cfg.ReadFloat(SECTION_NAME, "RightAnalogStickSensitivityY", 1.0f);
#ifdef GTA3
    zoomForAssaultRifles = cfg.ReadBool(SECTION_NAME, "ZoomForAssaultRifles", true);
    crouchKey = cfg.ReadString(SECTION_NAME, "CrouchKey", "C");
    rocketAimPitch = cfg.ReadFloat(SECTION_NAME, "RocketAimPitch", -45.0f);
#else
    zoomForAssaultRifles = cfg.ReadBool(SECTION_NAME, "ZoomForAssaultRifles", false);
#endif
    modernCamera = cfg.ReadBool(SECTION_NAME, "ModernCamera", true);
    reloadSettings = cfg.ReadInt(SECTION_NAME, "ReloadSettings", 0);
    crosshairType = cfg.ReadInt(SECTION_NAME, "CrosshairType", 2);
    persistentCrosshairDot = cfg.ReadBool(SECTION_NAME, "PersistentCrosshairDot", false);
    hitIndicator = cfg.ReadInt(SECTION_NAME, "HitIndicator", 2);
    animatedCrosshairs = cfg.ReadBool(SECTION_NAME, "AnimatedCrosshairs", true);
    killEffect = cfg.ReadBool(SECTION_NAME, "KillEffect", true);
    killEffectDuration = cfg.ReadInt(SECTION_NAME, "KillEffectDuration", 400);
    thirdPersonRocketLauncher = cfg.ReadBool(SECTION_NAME, "ThirdPersonRocketLauncher", true);
}

void Configs::WriteSettings() {
    std::string path;
    path = GetConfigPath();
    ConfigHelper cfg(path);

    if (cfg.Exists())
        return;

    cfg.CreateParentFolder();
    cfg.WriteBool(SECTION_NAME, "ForceAutoAim", forceAutoAim, "Force auto aim while playing with keyboard and mouse.");
    cfg.WriteInt(SECTION_NAME, "LockOnTargetType", lockOnTargetType, "Choose the lock on target to display while auto aiming (0 = default, 1 = SA, 2 = LCS/VCS).");
    cfg.WriteBool(SECTION_NAME, "ShowTriangleForMouseRecruit", showTriangleForMouseRecruit, "While aiming with mouse, displays a triangle with the color of the health over the head of the pedestrian.");
    cfg.WriteString(SECTION_NAME, "WalkKey", walkKey, "Makes the player walk using the defined keyboard key (NULL to disable).");
    cfg.WriteBool(SECTION_NAME, "StoriesAimingCoords", storiesAimingCoords, "Makes camera while aiming similar to LCS/VCS.");
    cfg.WriteFloat(SECTION_NAME, "CameraCrosshairMultX", cameraCrosshairMultX, "Crosshair position from (0.0f - 1.0f) while aiming in third person (default 0.53f, 0.4f).");
    cfg.WriteFloat(SECTION_NAME, "CameraCrosshairMultY", cameraCrosshairMultY);
    cfg.WriteBool(SECTION_NAME, "StoriesPointingArm", storiesPointingArm, "Makes player arm angle (while pointing with one hand) similar to LCS/VCS.");
    cfg.WriteFloat(SECTION_NAME, "RightAnalogStickSensitivityX", rightAnalogStickSensitivityX, "Adjust right analog stick sensitivity.");
    cfg.WriteFloat(SECTION_NAME, "RightAnalogStickSensitivityY", rightAnalogStickSensitivityY);
    cfg.WriteBool(SECTION_NAME, "ZoomForAssaultRifles", zoomForAssaultRifles, "Decrease FOV automatically while aiming with an assault rifle.");
    cfg.WriteBool(SECTION_NAME, "ModernCamera", modernCamera);
    cfg.WriteInt(SECTION_NAME, "ReloadSettings", reloadSettings, "Reload Settings WINDOWS KEYCODE (162 - LCtrl)");
    cfg.WriteInt(SECTION_NAME, "CrosshairType", crosshairType, "Choose crosshair type (0 = default, 1 = dot only, 2 = dot with complex).");
    cfg.WriteBool(SECTION_NAME, "PersistentCrosshairDot", persistentCrosshairDot, "Renders persistent crosshair dot on screen in 128 alpha.");
    cfg.WriteInt(SECTION_NAME, "HitIndicator", hitIndicator, "Hit indicator on ped hit (0 = off, 1 = on while ped dead, 2 = always on).");
    cfg.WriteBool(SECTION_NAME, "AnimatedCrosshairs", animatedCrosshairs, "Dynamically animate complex crosshair radius based on weapon recoil and accuracy.");
    cfg.WriteBool(SECTION_NAME, "KillEffect", killEffect, "Screen desaturation and contrast flash effect on ped kill (GTA V / RDR2 style).");
    cfg.WriteInt(SECTION_NAME, "KillEffectDuration", killEffectDuration, "Duration of the kill effect in milliseconds (default 400).");
    cfg.WriteBool(SECTION_NAME, "ThirdPersonRocketLauncher", thirdPersonRocketLauncher, "Enable third person over-the-shoulder aiming and firing for Rocket Launcher.");

#ifdef GTA3
    cfg.WriteString(SECTION_NAME, "CrouchKey", crouchKey, "Makes the player crouch using the defined keyboard key (NULL to disable).");
    cfg.WriteFloat(SECTION_NAME, "RocketAimPitch", rocketAimPitch, "Pitch angle (in degrees) for rocket launcher alignment in 3rd-person aim (default -45.0f).");
#endif

    cfg.Format();
}

void Configs::ReadCrosshairInfo(const std::string& folderPath) {
    std::string iniPath;
    size_t count, i;
    int wType;

    crosshairFolderPath = folderPath;
    iniPath = folderPath + "\\crosshair_info.ini";
    ConfigHelper cfg(iniPath);

    dotScale = 0.15f;
    hitIndicatorScale = 0.15f;
    maxRecoilKick = 0.40f;
    recoilRecoveryRate = 0.15f;

    defaultCrosshair = { 0.38f, 0.06f, CROSSHAIR_SITEM16 };
    for (i = 0; i < 64; i++) {
        weaponCrosshairs[i] = defaultCrosshair;
    }

    count = sizeof(defaultCrosshairMappings) / sizeof(defaultCrosshairMappings[0]);
    for (i = 0; i < count; i++) {
        wType = defaultCrosshairMappings[i].weaponType;
        if (wType >= 0 && wType < 64) {
            weaponCrosshairs[wType].scale = defaultCrosshairMappings[i].defaultScale;
            weaponCrosshairs[wType].recoilKick = defaultCrosshairMappings[i].defaultRecoilKick;
            weaponCrosshairs[wType].sprite = defaultCrosshairMappings[i].defaultSprite;
        }
    }

    if (!cfg.Exists()) {
        WriteCrosshairInfo(folderPath);
        return;
    }

    dotScale = cfg.ReadFloat("General", "DotScale", 0.15f);
    hitIndicatorScale = cfg.ReadFloat("General", "HitIndicatorScale", 0.15f);
    maxRecoilKick = cfg.ReadFloat("General", "MaxRecoilKick", 0.40f);
    recoilRecoveryRate = cfg.ReadFloat("General", "RecoilRecoveryRate", 0.15f);

    cfg.ReadCrosshair("Weapons", "Default", defaultCrosshair);

    for (i = 0; i < count; i++) {
        wType = defaultCrosshairMappings[i].weaponType;
        if (wType >= 0 && wType < 64) {
            cfg.ReadCrosshair("Weapons", defaultCrosshairMappings[i].weaponName, weaponCrosshairs[wType]);
        }
    }

#ifdef GTAVC
    cfg.ReadCrosshair("Weapons", "Pistol", weaponCrosshairs[WEAPONTYPE_PISTOL]);
    cfg.ReadCrosshair("Weapons", "Spas12", weaponCrosshairs[WEAPONTYPE_SPAS12_SHOTGUN]);
    cfg.ReadCrosshair("Weapons", "Stubby", weaponCrosshairs[WEAPONTYPE_STUBBY_SHOTGUN]);
    cfg.ReadCrosshair("Weapons", "m4", weaponCrosshairs[WEAPONTYPE_M4]);
#endif

#ifdef GTA3
    cfg.ReadCrosshair("Weapons", "Pistol", weaponCrosshairs[WEAPONTYPE_COLT45]);

    rocketAimRotPitch = 0.0f;
    rocketAimRotYaw = 0.0f;
    rocketAimRotRoll = 0.0f;
    rocketAimOffsetX = 0.0f;
    rocketAimOffsetY = 0.0f;
    rocketAimOffsetZ = 0.0f;

    cfg.ReadVector3("RPGAimAdjustment", "Rotation", rocketAimRotPitch, rocketAimRotYaw, rocketAimRotRoll);
    cfg.ReadVector3("RPGAimAdjustment", "Offset", rocketAimOffsetX, rocketAimOffsetY, rocketAimOffsetZ);

    Aiming::UpdateRocketAimAdjustment(rocketAimRotPitch, rocketAimRotYaw, rocketAimRotRoll,
        rocketAimOffsetX, rocketAimOffsetY, rocketAimOffsetZ);
#endif
}

void Configs::WriteCrosshairInfo(const std::string& folderPath) {
    std::string iniPath;
    size_t count, i;
    int wType;

    iniPath = folderPath + "\\crosshair_info.ini";
    ConfigHelper cfg(iniPath);

    if (cfg.Exists())
        return;

    cfg.CreateParentFolder();
    cfg.WriteFloat("General", "DotScale", dotScale, "Size multiplier for the center dot (default: 0.15)");
    cfg.WriteFloat("General", "HitIndicatorScale", hitIndicatorScale, "Size multiplier for the hit indicator cross (default: 0.15)");
    cfg.WriteFloat("General", "MaxRecoilKick", maxRecoilKick, "Maximum spread/recoil expansion offset (default: 0.40)");
    cfg.WriteFloat("General", "RecoilRecoveryRate", recoilRecoveryRate, "Speed at which recoil contracts back to standby size (default: 0.15)");

#ifdef GTA3
    cfg.WriteVector3("RPGAimAdjustment", "Rotation",
        rocketAimRotPitch, rocketAimRotYaw, rocketAimRotRoll,
        "Rocket launcher 3rd-person aim visual adjustment on Claude's shoulder (GTA III)\nRotation: Pitch, Yaw, Roll (degrees added to default shoulder pose)");
    cfg.WriteVector3("RPGAimAdjustment", "Offset",
        rocketAimOffsetX, rocketAimOffsetY, rocketAimOffsetZ,
        "Offset: X, Y, Z (relative position offset in units/meters)");
#endif

    cfg.WriteCrosshair("Weapons", "Default", defaultCrosshair, "Columns: Scale, RecoilKick, Sprite\nAvailable sprites: assault, smg, shotgun, siteM16, sitelaser, siterocket, sitesniper, laserdot\n;\nWeapon (weapons.dat) = Scale, RecoilKick, Sprite");

    count = sizeof(defaultCrosshairMappings) / sizeof(defaultCrosshairMappings[0]);
    for (i = 0; i < count; i++) {
        wType = defaultCrosshairMappings[i].weaponType;
        if (wType >= 0 && wType < 64) {
            cfg.WriteCrosshair("Weapons", defaultCrosshairMappings[i].weaponName, weaponCrosshairs[wType]);
        }
    }

    cfg.Format("; ClassicAxis - Crosshair Configuration\n; Customize crosshair general settings and per-weapon properties.");
}

const CrosshairConfig& Configs::GetCrosshair(eWeaponType weaponType) {
    if (weaponType >= 0 && weaponType < 64 && weaponCrosshairs[weaponType].scale > 0.0f)
        return weaponCrosshairs[weaponType];
    return defaultCrosshair;
}

