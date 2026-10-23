#include "Hooks.h"
#include "Camera.h"
#include "Aiming.h"
#include "Crosshairs.h"
#include "Configs.h"
#include "KillEffect.h"
#include "InputHandler.h"
#include "CamNew.h"

#include "plugin.h"
#include "CMenuManager.h"
#include "CDarkel.h"
#include "RenderWare.h"

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
    Crosshairs::InitTextures();
    KillEffect::Init();

#ifdef GTA3
    Aiming::weaponInfoSet = false;
#endif
}

static const bool* pAimingIsAiming = &Aiming::isAiming;

#ifdef GTA3
static const uintptr_t InflictDamageTarget = 0x4EA426;
static const uintptr_t RocketAbortTargetIII = 0x4F2032;
static const uintptr_t RocketContinueTargetIII = 0x4F2038;
static const uintptr_t RocketNotRocketTargetIII = 0x4F1F69;
static const uintptr_t RocketSoundPlayTargetIII = 0x4F00CA;
static const uintptr_t RocketSoundSkipTargetIII = 0x4F00D8;

static __declspec(naked) void RocketFireCheckIII() {
    __asm {
        cmp edx, 8
        jnz not_rocket
        mov edx, dword ptr [pAimingIsAiming]
        cmp byte ptr [edx], 0
        jnz continue_fire
        jmp dword ptr [RocketAbortTargetIII]

    not_rocket:
        jmp dword ptr [RocketNotRocketTargetIII]

    continue_fire:
        jmp dword ptr [RocketContinueTargetIII]
    }
}

static __declspec(naked) void RocketSoundCheckIII() {
    __asm {
        cmp eax, 8
        jnz skip_sound
        mov eax, dword ptr [pAimingIsAiming]
        cmp byte ptr [eax], 0
        jnz skip_sound
        jmp dword ptr [RocketSoundPlayTargetIII]

    skip_sound:
        jmp dword ptr [RocketSoundSkipTargetIII]
    }
}
#else
static const uintptr_t InflictDamageTarget = 0x525B26;
static const uintptr_t RocketAbortTargetVC = 0x534AF3;
static const uintptr_t RocketContinueTargetVC = 0x534B00;
static const uintptr_t RocketNotRocketTargetVC = 0x534AE0;
static const uintptr_t RocketSoundPlayTargetVC = 0x5379BA;
static const uintptr_t RocketSoundSkipTargetVC = 0x5379C8;

static __declspec(naked) void RocketFireCheckVC() {
    __asm {
        cmp ecx, 1Eh
        jnz not_rocket
        mov ecx, dword ptr [pAimingIsAiming]
        cmp byte ptr [ecx], 0
        jnz continue_fire
        jmp dword ptr [RocketAbortTargetVC]

    not_rocket:
        jmp dword ptr [RocketNotRocketTargetVC]

    continue_fire:
        jmp dword ptr [RocketContinueTargetVC]
    }
}

static __declspec(naked) void RocketSoundCheckVC() {
    __asm {
        cmp esi, 1Eh
        jnz skip_sound
        mov esi, dword ptr [pAimingIsAiming]
        cmp byte ptr [esi], 0
        jnz skip_sound
        jmp dword ptr [RocketSoundPlayTargetVC]

    skip_sound:
        jmp dword ptr [RocketSoundSkipTargetVC]
    }
}
#endif

#ifdef GTA3
static const uintptr_t AddrCPedIsPlayer = 0x4D48E0;
static const uintptr_t FireProjectileNotPlayerTargetIII = 0x5619B7;
static const uintptr_t FireProjectilePlayerTargetIII = 0x561993;
static const uintptr_t FireProjectileNormalPlayerTargetIII = 0x56194F;

static __declspec(naked) void FireProjectilePlayerHookIII() {
    __asm {
        call dword ptr [AddrCPedIsPlayer]
        test al, al
        jz not_player
        mov eax, dword ptr [pAimingIsAiming]
        cmp byte ptr [eax], 0
        jz normal_player
        movzx eax, byte ptr ds:[0x6FAD6E]
        imul eax, 1A4h
        jmp dword ptr [FireProjectilePlayerTargetIII]

    normal_player:
        jmp dword ptr [FireProjectileNormalPlayerTargetIII]

    not_player:
        jmp dword ptr [FireProjectileNotPlayerTargetIII]
    }
}

#else
static const uintptr_t AddrCPedIsPlayer = 0x4F4930;
static const uintptr_t FireProjectileNotPlayerTargetVC = 0x5CD050;
static const uintptr_t FireProjectilePlayerTargetVC = 0x5CD028;
static const uintptr_t FireProjectileNormalPlayerTargetVC = 0x5CCFE4;

static __declspec(naked) void FireProjectilePlayerHookVC() {
    __asm {
        call dword ptr [AddrCPedIsPlayer]
        test al, al
        jz not_player
        mov ecx, dword ptr [pAimingIsAiming]
        cmp byte ptr [ecx], 0
        jz normal_player
        movzx eax, byte ptr ds:[0x7E46FE]
        imul eax, 1CCh
        jmp dword ptr [FireProjectilePlayerTargetVC]

    normal_player:
        jmp dword ptr [FireProjectileNormalPlayerTargetVC]

    not_player:
        jmp dword ptr [FireProjectileNotPlayerTargetVC]
    }
}
#endif

static void OnInflictDamageCheck(CPed* victim, CEntity* damagedBy, eWeaponType weaponType, float damage) {
    CPlayerPed* player;
    CWeaponInfo* info;
    bool isKill;

    if (!victim || !damagedBy)
        return;

    player = FindPlayerPed();
    if (!player || damagedBy != player || victim == player)
        return;

    if (!Aiming::IsWeaponPossiblyCompatible(player))
        return;

    if (!Aiming::isAiming && player->m_ePedState != PEDSTATE_AIMGUN && player->m_ePedState != PEDSTATE_ATTACK)
        return;

    if (weaponType != player->m_aWeapons[player->m_nCurrentWeapon].m_eWeaponType)
        return;

    info = CWeaponInfo::GetWeaponInfo(weaponType);
    if (!info || (!info->m_bCanAim && !info->m_bCanAimWithArm) || info->m_bThrow)
        return;

    isKill = (victim->m_fHealth - damage <= 0.0f) || (victim->m_ePedState == PEDSTATE_DEAD);
    Crosshairs::OnPedHit(isKill);
}

static __declspec(naked) void InflictDamageProxy() {
    __asm {
        push eax
        push ecx
        push edx

        push dword ptr [esp + 24]
        push dword ptr [esp + 24]
        push dword ptr [esp + 24]
        push ecx
        call OnInflictDamageCheck
        add esp, 16

        pop edx
        pop ecx
        pop eax

#ifdef GTA3
        fld dword ptr ds:[0x5F9AB8]
#else
        fld dword ptr ds:[0x694170]
#endif
        jmp dword ptr [InflictDamageTarget]
    }
}

static void HookedRegisterKillByPlayer(CPed* victim, eWeaponType weaponType, bool isHeadshot) {
    CPlayerPed* player;
    CWeaponInfo* info;

    player = FindPlayerPed();
    if (player && victim && victim != player) {
        if (Aiming::IsWeaponPossiblyCompatible(player)) {
            if (Aiming::isAiming || player->m_ePedState == PEDSTATE_AIMGUN || player->m_ePedState == PEDSTATE_ATTACK) {
                if (weaponType == player->m_aWeapons[player->m_nCurrentWeapon].m_eWeaponType) {
                    info = CWeaponInfo::GetWeaponInfo(weaponType);
                    if (info && (info->m_bCanAim || info->m_bCanAimWithArm) && !info->m_bThrow) {
                        Crosshairs::OnPedHit(true);
                    }
                }
            }
        }
    }

    CDarkel::RegisterKillByPlayer(victim, weaponType, isHeadshot);
}


static void OnReinitGame() {
    Camera::Clear();
    Aiming::Clear();
    Crosshairs::Clear();
    KillEffect::Clear();
}

static void OnGameProcessReload() {
    if (configs.reloadSettings != 0) {
        if (configs.reloadSettings == -1 || plugin::KeyPressed(configs.reloadSettings)) {
            configs.ReadSettings();
            if (!configs.crosshairFolderPath.empty())
                configs.ReadCrosshairInfo(configs.crosshairFolderPath);
        }
    }
}

static void OnGameProcessWeaponInfo() {
    CWeaponInfo *rifleInfo, *rocketInfo;

    if (!Aiming::weaponInfoSet) {
        if (configs.thirdPersonRocketLauncher) {
#ifdef GTA3
            rifleInfo = CWeaponInfo::GetWeaponInfo(WEAPONTYPE_M16);
            rocketInfo = CWeaponInfo::GetWeaponInfo(WEAPONTYPE_ROCKETLAUNCHER);
            if (rifleInfo && rocketInfo) {
                rocketInfo->m_nAnimToPlay = rifleInfo->m_nAnimToPlay;
                rocketInfo->m_fAnimLoopStart = rifleInfo->m_fAnimLoopStart;
                rocketInfo->m_fAnimLoopEnd = rifleInfo->m_fAnimLoopEnd;
                rocketInfo->m_fAnimFrameFire = rifleInfo->m_fAnimFrameFire;
                rocketInfo->m_fAnim2FrameFire = rifleInfo->m_fAnim2FrameFire;
                rocketInfo->m_bCanAim = 1;
                rocketInfo->m_bCanAimWithArm = 0;
                rocketInfo->m_b1stPerson = 0;
            }
#else
            rifleInfo = CWeaponInfo::GetWeaponInfo(WEAPONTYPE_M4);
            rocketInfo = CWeaponInfo::GetWeaponInfo(WEAPONTYPE_ROCKETLAUNCHER);
            if (rifleInfo && rocketInfo) {
                rocketInfo->m_nAnimToPlay = rifleInfo->m_nAnimToPlay;
                rocketInfo->m_fAnimLoopStart = rifleInfo->m_fAnimLoopStart;
                rocketInfo->m_fAnimLoopEnd = rifleInfo->m_fAnimLoopEnd;
                rocketInfo->m_fAnimFrameFire = rifleInfo->m_fAnimFrameFire;
                rocketInfo->m_fAnim2LoopStart = rifleInfo->m_fAnim2LoopStart;
                rocketInfo->m_fAnim2LoopEnd = rifleInfo->m_fAnim2LoopEnd;
                rocketInfo->m_fAnim2FrameFire = rifleInfo->m_fAnim2FrameFire;
                rocketInfo->m_bCanAim = 1;
                rocketInfo->m_bCanAimWithArm = 0;
                rocketInfo->m_b1stPerson = 0;
            }
#endif
        }
#ifdef GTA3
        memcpy(&Aiming::backupWeaponInfo, aWeaponInfo, sizeof(CWeaponInfo) * (WEAPONTYPE_HELICANNON + 1));
#endif
        Aiming::weaponInfoSet = true;
    }
}

static void OnBeforeDrawHud() {
    KillEffect::Render();
}

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

    plugin::Events::gameProcessEvent += OnGameProcessWeaponInfo;
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

    // 2b. Fps to 3rd - Rocket Launcher
    if (configs.thirdPersonRocketLauncher) {
#ifdef GTA3
        plugin::patch::RedirectJump(0x4F1F64, RocketFireCheckIII);
        plugin::patch::RedirectJump(0x4F00C5, RocketSoundCheckIII);
        plugin::patch::RedirectJump(0x561946, FireProjectilePlayerHookIII);
#else
        plugin::patch::SetChar(0x5349C5 + 2, -1);
        plugin::patch::RedirectJump(0x534ADB, RocketFireCheckVC);
        plugin::patch::RedirectJump(0x5379B5, RocketSoundCheckVC);
        plugin::patch::RedirectJump(0x5CCFDB, FireProjectilePlayerHookVC);
#endif
    }

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
    plugin::Events::drawHudEvent.before += OnBeforeDrawHud;
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

    // 17b. Inflict damage hook for hit indicator
#ifdef GTA3
    plugin::patch::RedirectJump(0x4EA420, InflictDamageProxy);
    plugin::patch::SetUChar(0x4EA420 + 5, 0x90);
    plugin::patch::RedirectCall(0x4EAD39, HookedRegisterKillByPlayer);
#else
    plugin::patch::RedirectJump(0x525B20, InflictDamageProxy);
    plugin::patch::SetUChar(0x525B20 + 5, 0x90);
    plugin::patch::RedirectCall(0x526840, HookedRegisterKillByPlayer);
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
