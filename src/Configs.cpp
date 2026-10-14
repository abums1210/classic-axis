#include "Configs.h"
#include "ConfigHelper.h"
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

void Configs::Read() {
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
#else
        zoomForAssaultRifles = false;
#endif
        modernCamera = true;
        reloadSettings = 0;

        Save();
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
#else
    zoomForAssaultRifles = cfg.ReadBool(SECTION_NAME, "ZoomForAssaultRifles", false);
#endif
    modernCamera = cfg.ReadBool(SECTION_NAME, "ModernCamera", true);
    reloadSettings = cfg.ReadInt(SECTION_NAME, "ReloadSettings", 0);
}

void Configs::Save() {
    std::string path;
    path = GetConfigPath();
    ConfigHelper cfg(path);

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

#ifdef GTA3
    cfg.WriteString(SECTION_NAME, "CrouchKey", crouchKey, "Makes the player crouch using the defined keyboard key (NULL to disable).");
#endif

    cfg.Format();
}
