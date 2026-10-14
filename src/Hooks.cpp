#include "Hooks.h"
#include "Camera.h"
#include "Aiming.h"
#include "Crosshairs.h"
#include "Configs.h"
#include "InputHandler.h"
#include "CamNew.h"

#include "plugin.h"
#include "CMenuManager.h"

#ifdef GTA3
static void __fastcall ClearWeaponTargetCallback(CPlayerPed* player, int) {
    if (!Aiming::isAiming)
        player->ClearWeaponTarget();
}
#else
static void __fastcall DummySpriteDraw(int, int, int, int, int, int, int, int, int, int, int, int) {}
#endif

static void OnInitRw() {
    InputHandler::InitGInput();
    Camera::Clear();
    Aiming::Clear();
    Crosshairs::Clear();

#ifdef GTA3
    Aiming::weaponInfoSet = false;
#endif
}

static void OnReinitGame() {
    Camera::Clear();
    Aiming::Clear();
    Crosshairs::Clear();
}

static void OnGameProcessReload() {
    if (configs.reloadSettings != 0) {
        if (configs.reloadSettings == -1 || plugin::KeyPressed(configs.reloadSettings))
            configs.Read();
    }
}

#ifdef GTA3
static void OnGameProcessWeaponInfo() {
    if (!Aiming::weaponInfoSet) {
        memcpy(&Aiming::backupWeaponInfo, aWeaponInfo, sizeof(CWeaponInfo) * WEAPONTYPE_HELICANNON + 1);
        Aiming::weaponInfoSet = true;
    }
}
#endif

static void OnDrawHud() {
    bool disableAutoAim;

    Crosshairs::DrawCrosshair();

    disableAutoAim = !InputHandler::HasPadInHands() && !configs.forceAutoAim;
    if (!disableAutoAim)
        Crosshairs::DrawAutoAimTarget();

    if (configs.showTriangleForMouseRecruit)
        Crosshairs::DrawTriangleForMouseRecruitPed();
}

void Hooks::RegisterCoreEvents() {
#ifdef GTA3
    plugin::CdeclEvent<plugin::AddressList<0x48C7BE, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPickNone, void()> onReinitGame;
#else
    plugin::CdeclEvent<plugin::AddressList<0x4A46F1, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPickNone, void()> onReinitGame;
#endif

    if (configs.reloadSettings) {
        plugin::Events::gameProcessEvent += OnGameProcessReload;
    }

    plugin::Events::initRwEvent += OnInitRw;

    onReinitGame.after += OnReinitGame;

#ifdef GTA3
    plugin::Events::gameProcessEvent += OnGameProcessWeaponInfo;
#endif
}

void Hooks::ApplyCameraMovementAndStancePatches() {
    int i;
#ifdef GTA3
    int playerMovementTypeAddr[] = { 0x4F2685, 0x469292, 0x4C7673, 0x4F03D5, 0x4D76CE, 0x45DB98 };
    int playerShootingDirectionAddr[] = { 0x4E6562, 0x55D88B, 0x560C25, 0x561E63, 0x4EDAD1 };
    plugin::ThiscallEvent<plugin::AddressList<0x4E6EBF, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPick3N<CWeapon*, 0, CEntity*, 1, CVector*, 2>, char(CWeapon*, CEntity*, CVector*)> onFiring;
#else
    int playerMovementTypeAddr[] = { 0x4720B1, 0x533BCE, 0x4711DB, 0x50A8E3, 0x537D1C, 0x4F002A };
    int playerShootingDirectionAddr[] = { 0x52D73E, 0x5D18E7, 0x5CDA44, 0x5CCC14 };
    plugin::ThiscallEvent<plugin::AddressList<0x52B749, plugin::H_CALL, 0x52B9B4, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPick3N<CWeapon*, 0, CEntity*, 1, CVector*, 2>, char(CWeapon*, CEntity*, CVector*)> onFiring;
#endif

    // 1. FollowPedWithMouse hook
#ifdef GTA3
    plugin::patch::RedirectCall(0x459A7C, (void(__fastcall*)(CCam*, int, CVector const&, float, float, float))Camera::ProcessFollowPedWithMouse);
    plugin::patch::Nop(0x459A65, 2);
#else
    plugin::patch::RedirectCall(0x483BA1, (void(__fastcall*)(CCam*, int, CVector const&, float, float, float))Camera::ProcessFollowPedWithMouse);
    plugin::patch::Nop(0x483B8A, 2);
#endif

    // 2. Fps to 3rd - m16 / rifles
#ifdef GTA3
    plugin::patch::SetChar(0x4F1F72 + 2, -1);
#else
    plugin::patch::Set<BYTE>(0x5349DB, 0x90);
    plugin::patch::Set<BYTE>(0x5349DB + 1, 0x90);
    plugin::patch::Set<BYTE>(0x5349E0, 0x90);
    plugin::patch::Set<BYTE>(0x5349E0 + 1, 0x90);
    plugin::patch::Set<BYTE>(0x5349E5, 0x90);
    plugin::patch::Set<BYTE>(0x5349E5 + 1, 0x90);
#endif

    // 3. No fight cam
#ifdef GTA3
    plugin::patch::Nop(0x469226, 9);
    plugin::patch::Nop(0x469260, 9);
#else
    plugin::patch::Nop(0x4715D9, 9);
    plugin::patch::Nop(0x471613, 9);
    plugin::patch::Set<BYTE>(0x4F0031, 0xEB);
#endif

    // 4. PlayerMovementType
    for (i = 0; i < ARRAYSIZE(playerMovementTypeAddr); i++)
        plugin::patch::RedirectCall(playerMovementTypeAddr[i], (int(__fastcall*)(int, int))Camera::GetPlayerMovementType);

    // 5. onFiring
    onFiring.before += Camera::OnFiringBefore;
    onFiring.after += Camera::OnFiringAfter;

    // 6. PlayerShootingDirection
    for (i = 0; i < ARRAYSIZE(playerShootingDirectionAddr); i++)
        plugin::patch::RedirectCall(playerShootingDirectionAddr[i], (int(__fastcall*)(int, int))Camera::GetPlayerShootingDirection);

    // 7. Using3rd
#ifdef GTA3
    plugin::patch::SetChar(0x468E83 + 3, 0);
    plugin::patch::RedirectCall(0x468E46, (bool(__fastcall*)(CCam*, int))Camera::DisableNative3rdPersonCam);
#else
    plugin::patch::SetChar(0x471218 + 3, 0);
    plugin::patch::RedirectCall(0x4711DB, (bool(__fastcall*)(CCam*, int))Camera::DisableNative3rdPersonCam);
#endif

    // 8. Clear weapon target / Clear aim flag
#ifdef GTA3
    plugin::patch::RedirectCall(0x4F22A0, (void(__fastcall*)(CPlayerPed*, int))ClearWeaponTargetCallback);
    plugin::patch::Nop(0x4F21A0, 6);
    plugin::patch::Nop(0x4F21CA, 7);
    plugin::patch::Set<BYTE>(0x468D09, 0x74);
    plugin::patch::Set<BYTE>(0x468D92, 0x74);
#else
    plugin::patch::Nop(0x534D42, 6);
    plugin::patch::Set<BYTE>(0x534D7B, 0x74);
    plugin::patch::Set<BYTE>(0x471093, 0x74);
    plugin::patch::Set<BYTE>(0x47111B, 0x74);
    plugin::patch::RedirectCall(0x52C274, (void(__fastcall*)(CPed*, int))Aiming::ClearAimFlag);
#endif

    // 9. No point gun cam
#ifdef GTA3
    plugin::patch::Nop(0x469C14, 6);
    plugin::patch::Nop(0x469CDD, 9);
    plugin::patch::Nop(0x469D5A, 9);
#else
    plugin::patch::Nop(0x472422, 6);
    plugin::patch::Nop(0x4724D5, 9);
    plugin::patch::Nop(0x47254F, 9);
#endif
}

void Hooks::ApplyCameraModeAndControlPatches() {
#ifdef GTA3
    plugin::ThiscallEvent<plugin::AddressList<0x46A9C3, plugin::H_CALL, 0x46AB94, plugin::H_CALL, 0x46AC11, plugin::H_CALL, 0x470D8C, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPick2N<CCam*, 0, short, 1>, void(CCam*, short)> onStartingTransition;
    plugin::ThiscallEvent<plugin::AddressList<0x46D5FF, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPickN<CCam*, 0>, void(CCam*)> onProcessingCam;
    plugin::ThiscallEvent<plugin::AddressList<0x4EFE50, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPickN<CPed*, 0>, void(CPed*)> onProcessingPlayerControl;
#else
    plugin::ThiscallEvent<plugin::AddressList<0x47360B, plugin::H_CALL, 0x4734E5, plugin::H_CALL, 0x473305, plugin::H_CALL, 0x46AB00, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPick2N<CCam*, 0, short, 1>, void(CCam*, short)> onStartingTransition;
    plugin::ThiscallEvent<plugin::AddressList<0x46C7A3, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPickN<CCam*, 0>, void(CCam*)> onProcessingCam;
    plugin::ThiscallEvent<plugin::AddressList<0x53739F, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPickN<CPed*, 0>, void(CPed*)> onProcessingPlayerControl;
#endif

    // 10. onStartingTransition
    onStartingTransition.after += Camera::OnStartingTransition;

    // 11. onProcessingCam
#ifdef GTA3
    plugin::patch::Nop(0x459CD0, 90);
#else
    plugin::patch::Nop(0x483DB6, 90);
#endif
    onProcessingCam.before += Camera::OnProcessingCam;

    // 12. CamControl
#ifdef GTA3
    plugin::patch::RedirectCall(0x46D500, (void(__fastcall*)(int, int))Camera::CamControl);
#else
    plugin::patch::RedirectCall(0x46C694, (void(__fastcall*)(int, int))Camera::CamControl);
#endif

    // 13. onProcessingPlayerControl
    onProcessingPlayerControl.before += Aiming::OnProcessingPlayerControlBefore;
    onProcessingPlayerControl.after += Aiming::OnProcessingPlayerControlAfter;
}

void Hooks::ApplyHudAndMenuPatches() {
#ifdef GTA3
    const CMenuScreen controllerSetup = {
        "FET_CTL", 41, 41, 41, 0, 0,
        MENUACTION_REDEFCTRL, "FET_RDK",  0, MENUPAGE_KEYBOARD_CONTROLS,
        MENUACTION_CHANGEMENU, "FET_AMS", 0, MENUPAGE_MOUSE_SETTINGS,
        MENUACTION_RESTOREDEF, "FET_DEF", 0, MENUPAGE_CONTROLLER_PC,
        MENUACTION_CHANGEMENU, "FEDS_TB", 0, MENUPAGE_NONE,
    };
#else
    const CMenuScreen controllerSetup = {
        "FET_CTL", MENUPAGE_OPTIONS, 0,
        {
            { MENUACTION_REDEFCTRL, "FEC_RED", 0, MENUPAGE_KEYBOARD_CONTROLS, 320, 150, 3 },
            { MENUACTION_CHANGEMENU,"FEC_MOU", 0, MENUPAGE_MOUSE_SETTINGS, 0, 0, 3 },
            { MENUACTION_RESTOREDEF, "FET_DEF", 0, MENUPAGE_CONTROLLER_PC, 0, 0, 3 },
            { MENUACTION_GOBACK, "FEDS_TB", 0, MENUPAGE_NONE, 0, 0, 3 },
        }
    };
#endif

    // 14. Crosshair suppression
#ifdef GTA3
    plugin::patch::SetChar(0x50554C + 1, 0);
    plugin::patch::SetChar(0x505627 + 1, 0);
#else
    plugin::patch::RedirectCall(0x557713, (void(__fastcall*)(int, int, int, int, int, int, int, int, int, int, int, int))DummySpriteDraw);
    plugin::patch::RedirectCall(0x5577F7, (void(__fastcall*)(int, int, int, int, int, int, int, int, int, int, int, int))DummySpriteDraw);
#endif

    // 15. drawHudEvent
    plugin::Events::drawHudEvent += OnDrawHud;

    // 16. Lock on target type
    if (configs.lockOnTargetType > TARGET_DEFAULT) {
#ifdef GTA3
        plugin::patch::SetUChar(0x564D04 + 6, 0);
#else
        plugin::patch::SetUChar(0x5D5064 + 6, 0);
#endif
    }

    // 17. Menu
#ifdef GTA3
    plugin::patch::Set(0x611930 + sizeof(CMenuScreen) * 35, controllerSetup);
#else
    plugin::patch::Set(0x6D8B70 + sizeof(CMenuScreen) * MENUPAGE_CONTROLLER_PC, controllerSetup);
#endif
}

void Hooks::ApplyWeaponCycleAndDuckingPatches() {
#ifdef GTAVC
    plugin::ThiscallEvent<plugin::AddressList<0x53491B, plugin::H_CALL, 0x535326, plugin::H_CALL, 0x5355C6, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPick3N<CPed*, 0, int, 1, __int16, 2>, void(CPed*, int, __int16)> onDucking;
    plugin::ThiscallEvent<plugin::AddressList<0x534968, plugin::H_CALL, 0x535355, plugin::H_CALL, 0x5355F5, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPick2N<CPed*, 0, char, 1>, void(CPed*, char)> onClearDuck;
    plugin::ThiscallEvent<plugin::AddressList<0x534D64, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickN<CPlayerPed*, 0>, void(CPlayerPed*)> onFindingNewLockOnTarget;
#endif

#ifdef GTA3
    plugin::ThiscallEvent<plugin::AddressList<0x4F1FDB, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPick4N<CCam*, 0, int, 1, int, 2, int, 3>, void(CCam*, int, int, int)> onSet1stPersonPlayerCamMode;
    plugin::ThiscallEvent<plugin::AddressList<0x4F21B7, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickN<CPlayerPed*, 0>, void(CPlayerPed*)> onFindingLockOnTarget;
#else
    plugin::ThiscallEvent<plugin::AddressList<0x534A60, plugin::H_CALL>, plugin::PRIORITY_BEFORE, plugin::ArgPick4N<CCam*, 0, int, 1, int, 2, int, 3>, void(CCam*, int, int, int)> onSet1stPersonPlayerCamMode;
    plugin::ThiscallEvent<plugin::AddressList<0x534D64, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickN<CPlayerPed*, 0>, void(CPlayerPed*)> onFindingLockOnTarget;
#endif

    // 18. Weapon cycle
#ifdef GTA3
    plugin::patch::RedirectCall(0x4F0464, (void(__fastcall*)(CPlayerPed*, int, CPad*))Aiming::ProcessWeaponSwitch);
#else
    plugin::patch::RedirectCall(0x537DC9, (void(__fastcall*)(CPlayerPed*, int, CPad*))Aiming::ProcessWeaponSwitch);
#endif

    // 19. Weapon smooth spray
#ifdef GTA3
    plugin::patch::RedirectCall(0x4F13CC, (bool(__fastcall*)(CPlayerPed*, int))Aiming::DoWeaponSmoothSpray);
#else
    plugin::patch::RedirectCall(0x535F4C, (float(__fastcall*)(CPlayerPed*, int))Aiming::DoWeaponSmoothSpray);
#endif

    // 20. Point gun while ducking fix (VC)
#ifdef GTAVC
    onDucking += Aiming::OnDucking;
    onClearDuck += Aiming::OnClearDuck;
    onFindingNewLockOnTarget += Aiming::OnFindingNewLockOnTarget;
#endif

    // 21. Make 1st person cam mode point on front
#ifdef GTA3
    plugin::patch::Nop(0x461B61, 10);
    plugin::patch::Nop(0x4624A2, 10);
#else
    plugin::patch::Nop(0x47949C, 10);
    plugin::patch::Nop(0x47A289, 10);
#endif
    onSet1stPersonPlayerCamMode += Camera::OnSet1stPersonPlayerCamMode;

    // 22. onFindingLockOnTarget
    onFindingLockOnTarget += Aiming::OnFindingLockOnTarget;
}

void Hooks::ApplyAimingArmAndTargetVectorPatches() {
    // 23. Stories like aiming arm & weapon fixes
#ifdef GTA3
    if (configs.storiesPointingArm)
        plugin::patch::SetFloat(0x5FA008, 0.0f);

    plugin::patch::Nop(0x4E67A4, 2);
    plugin::patch::Nop(0x4E6808, 2);
    plugin::patch::Set<BYTE>(0x4E6C65, 0xEB);
    plugin::patch::Set<BYTE>(0x55C867, 0xEB);
#else
    plugin::patch::Nop(0x537454, 7);
    plugin::patch::Set<BYTE>(0x5D154C + 1, 0x85);

    if (configs.storiesPointingArm)
        plugin::patch::SetFloat(0x694720, 0.0f);

    plugin::patch::Set<BYTE>(0x5D4B85, 0xEB);
#endif

    // 24. Find3rdPersonCamTargetVector
#ifdef GTA3
    plugin::patch::RedirectJump(0x46B580, (bool(__fastcall*)(CCamera*, int, float, CVector, CVector&, CVector&))Camera::Find3rdPersonCamTargetVector);
#else
    plugin::patch::RedirectJump(0x46F890, (bool(__fastcall*)(CCamera*, int, float, CVector, CVector&, CVector&))Camera::Find3rdPersonCamTargetVector);
#endif
}

void Hooks::Init() {
    RegisterCoreEvents();
    ApplyCameraMovementAndStancePatches();
    ApplyCameraModeAndControlPatches();
    ApplyHudAndMenuPatches();
    ApplyWeaponCycleAndDuckingPatches();
    ApplyAimingArmAndTargetVectorPatches();
}
