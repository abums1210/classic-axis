#include "Aiming.h"
#include "Camera.h"
#include "Configs.h"
#include "InputHandler.h"
#include "Utility.h"
#include "CamNew.h"

#include "CPad.h"
#include "CCamera.h"
#include "CGeneral.h"
#include "CWorld.h"
#include "CSprite.h"
#include "CAnimBlendAssociation.h"
#include "CAnimManager.h"
#include "common.h"
#include "CWeaponEffects.h"

#ifdef GTA3
#include "RpAnimBlend.h"
#endif

bool Aiming::isAiming = false;
bool Aiming::wasPointingGun = false;
bool Aiming::wasCrouching = false;
bool Aiming::ignoreRotation = false;
bool Aiming::forceWalkaroundAnim = false;
CPed* Aiming::thirdPersonMouseTarget = nullptr;

#ifdef GTA3
bool Aiming::wantsToResetWeaponInfo = false;
bool Aiming::weaponInfoSet = false;
CWeaponInfo Aiming::backupWeaponInfo[WEAPONTYPE_HELICANNON + 1] = {};
#endif

void Aiming::Clear() {
    isAiming = false;
    wasPointingGun = false;
    wasCrouching = false;
    ignoreRotation = false;
    forceWalkaroundAnim = false;
    thirdPersonMouseTarget = nullptr;

#ifdef GTA3
    wantsToResetWeaponInfo = false;
#endif
}

bool Aiming::IsAbleToAim(CPed* ped) {
    ePedState s;
    eMoveState m;
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
    CWeaponInfo* info;

    s = ped->m_ePedState;
    m = ped->m_eMoveState;
    info = CWeaponInfo::GetWeaponInfo(weaponType);

    if (weaponType == WEAPONTYPE_UNARMED
#ifdef GTAVC
        || weaponType == WEAPONTYPE_BRASSKNUCKLE
#endif
        )
        return false;

    switch (s) {
        case PEDSTATE_NONE:
        case PEDSTATE_IDLE:
        case PEDSTATE_FLEE_POSITION:
        case PEDSTATE_FLEE_ENTITY:
        case PEDSTATE_ATTACK:
        case PEDSTATE_FIGHT:
        case PEDSTATE_AIMGUN:
            return m != PEDMOVE_SPRINT && ped->IsPedInControl()
#ifdef GTAVC
                && (!ped->bIsDucking || info->m_bCrouchFire)
#endif
                ;
        default:
            return false;
    }
}

bool Aiming::IsType1stPerson(CPed* ped) {
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
    CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(weaponType);

    switch (weaponType) {
        case WEAPONTYPE_SNIPERRIFLE:
        case WEAPONTYPE_ROCKETLAUNCHER:
#ifdef GTAVC
        case WEAPONTYPE_LASERSCOPE:
#endif
            return true;
    }

    return !info->m_bCanAim && !info->m_bCanAimWithArm && info->m_b1stPerson;
}

bool Aiming::IsWeaponPossiblyCompatible(CPed* ped) {
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
    CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(weaponType);

    if (weaponType == WEAPONTYPE_UNARMED
#ifdef GTAVC
        || weaponType == WEAPONTYPE_BRASSKNUCKLE
#endif
        )
        return false;

    switch (weaponType) {
        case WEAPONTYPE_FLAMETHROWER:
#ifndef GTA3
        case WEAPONTYPE_MINIGUN:
#endif
            if (ped->bIsDucking)
                info->m_bCanAim = false;
            else
                info->m_bCanAim = true;
            break;
    }

    return (info->m_bCanAim || info->m_bCanAimWithArm) && !info->m_bThrow && !info->m_b1stPerson;
}

bool Aiming::IsTypeMelee(CPed* ped) {
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
#ifdef GTAVC
    CWeaponInfo* info;
#endif

    if (weaponType == WEAPONTYPE_UNARMED
#ifdef GTAVC
        || weaponType == WEAPONTYPE_BRASSKNUCKLE
#endif
        )
        return true;

#ifdef GTA3
    switch (weaponType) {
        case WEAPONTYPE_UNARMED:
        case WEAPONTYPE_BASEBALLBAT:
        case WEAPONTYPE_MOLOTOV:
        case WEAPONTYPE_GRENADE:
        case WEAPONTYPE_DETONATOR:
            return true;
    }
#else
    info = CWeaponInfo::GetWeaponInfo(weaponType);
    return !info->m_bCanAim && !info->m_bCanAimWithArm;
#endif

    return false;
}

bool Aiming::IsTypeTwoHanded(CPed* ped) {
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
    CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(weaponType);

    if (weaponType == WEAPONTYPE_UNARMED
#ifdef GTAVC
        || weaponType == WEAPONTYPE_BRASSKNUCKLE
#endif
        )
        return false;

    switch (weaponType) {
        case WEAPONTYPE_FLAMETHROWER:
#ifndef GTA3
        case WEAPONTYPE_MINIGUN:
#endif
            if (ped->bIsDucking)
                info->m_bCanAim = false;
            else
                info->m_bCanAim = true;
            break;
    }

    return (info->m_bCanAim || info->m_bHeavy) && !info->m_bCanAimWithArm && !info->m_b1stPerson;
}

void Aiming::RotatePlayer(CPed* ped, float angle, bool smooth) {
    float targetAngle;

    if (ignoreRotation)
        return;

    if (smooth) {
        ped->m_fHeadingGoal = angle;
        ped->m_fHeadingCurrent = CGeneral::LimitRadianAngle(ped->m_fHeadingCurrent);
        targetAngle = ped->m_fHeadingGoal;

        if (ped->m_fHeadingCurrent - M_PI > ped->m_fHeadingGoal) {
            targetAngle += 2.0f * M_PI;
        }
        else if (M_PI + ped->m_fHeadingCurrent < ped->m_fHeadingGoal) {
            targetAngle -= 2.0f * M_PI;
        }

        ped->m_fHeadingCurrent += (targetAngle - ped->m_fHeadingCurrent) * 0.02f;
    }
    else {
        ped->m_fHeadingCurrent = angle;
        ped->m_fHeadingGoal = angle;
        ped->GetMatrix().SetRotateZOnly(ped->m_fHeadingCurrent);
    }
}

void Aiming::ClearWeaponTarget(CPlayerPed* ped) {
    ped->m_bHasLockOnTarget = false;
    ped->m_pPointGunAt = NULL;
    TheCamera.ClearPlayerWeaponMode();
    gCrossHair.ClearCrossHair();
}

void Aiming::Find3rdPersonMouseTarget(CPlayerPed* ped) {
    CCam* cam;
    CWeaponInfo* info;
    CEntity* entity;
    CPed* targetPed;
    CColPoint point;
    CVector source, target;
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;

    if (InputHandler::HasPadInHands())
        return;

    cam = &TheCamera.m_asCams[TheCamera.m_nActiveCam];
    entity = NULL;
    memset(&point, 0, sizeof(CColPoint));
    info = CWeaponInfo::GetWeaponInfo(weaponType);

    if (isAiming && !ped->m_bInVehicle && (!ped->m_bHasLockOnTarget || !ped->m_pPointGunAt)) {
        TheCamera.Find3rdPersonCamTargetVector(info->m_fRange, cam->m_vecSource, source, target);
        if (CWorld::ProcessLineOfSight(source, target, point, entity, false, false, true, false, false, false, false
#ifdef GTAVC
            , false
#endif
            )) {
            if (entity->m_nType == ENTITY_TYPE_PED) {
                targetPed = static_cast<CPed*>(entity);

                if (targetPed && targetPed != ped && targetPed->m_ePedState != PEDSTATE_DEAD) {
                    if (!thirdPersonMouseTarget || thirdPersonMouseTarget != targetPed) {
                        thirdPersonMouseTarget = targetPed;

                        if (thirdPersonMouseTarget->CanSeeEntity(ped, DegToRad(60.0f) * 2.0f)) {
                            thirdPersonMouseTarget->ReactToPointGun(ped);
#ifdef GTAVC
                            ped->Say(117);
#endif
                        }
                    }
                }
            }
            return;
        }
    }

    if (thirdPersonMouseTarget) {
        thirdPersonMouseTarget = NULL;
    }
}

void Aiming::ProcessPlayerPedControl(CPlayerPed* player) {
    CPad* pad;
    CCam* cam;
    CWeapon* currentWeapon;
    CWeaponInfo* info;
    CEntity* targetEntity;
    CPed* targetPed;
    CAnimBlendAssociation* anim;
    CAnimBlendAssociation* animRel;
    CAnimBlendAssociation* assoc;
    short mode;
    float front, height, mouseX, mouseY, torsoPitch;
    eWeaponType weaponType;
    bool hasPadInHands, fastReload, disableAutoAim, transitionDone, point, relState;
    CVector diff;
    RwV3d in, out;

#ifdef GTA3
    int animToPlay2, animToPlay, groupId;
    bool duckJustDown;
    CAnimBlendAssociation* animAssoc;
#else
    int animToPlay2, animToPlay, groupId;
#endif

    pad = CPad::GetPad(0);
    cam = &TheCamera.m_asCams[TheCamera.m_nActiveCam];
    mode = cam->m_nCamMode;
    front = CGeneral::LimitRadianAngle(-TheCamera.m_fOrientation);
    height = Camera::Find3rdPersonQuickAimPitch(TheCamera.m_f3rdPersonCHairMultY);
    currentWeapon = &player->m_aWeapons[player->m_nCurrentWeapon];
    weaponType = currentWeapon->m_eWeaponType;
    info = CWeaponInfo::GetWeaponInfo(weaponType);
    hasPadInHands = InputHandler::HasPadInHands();
    fastReload = CWorld::Players[CWorld::PlayerInFocus].m_bFastReload;

    if (InputHandler::WalkKeyDown()) {
        player->m_fMoveSpeed = 0.0f;
    }

    isAiming = false;
    ignoreRotation = false;

    if (IsType1stPerson(player))
        return;

    if (!pad->DisablePlayerControls && IsAbleToAim(player) &&
#ifdef GTA3
        (!player->bIsDucking || CanDuckWithThisWeapon(weaponType)) &&
#endif
        pad->GetTarget() && (TheCamera.GetLookDirection() != 0) && IsWeaponPossiblyCompatible(player) && (mode == MODE_FOLLOW_PED || mode == MODE_AIMWEAPON)
        && currentWeapon->HasWeaponAmmoToBeUsed() && !pad->JumpJustDown() && !pad->GetSprint()) {
        isAiming = true;
        if (mode != MODE_AIMWEAPON && player->IsPedInControl() && TheCamera.m_nTransitionState == 0) {
            TheCamera.TakeControl(FindPlayerPed(), MODE_AIMWEAPON, 1, 0);
            TheCamera.m_bLookingAtPlayer = false;
            Camera::customTransitionSpeedPending = true;
            previousHorAngle = cam->m_fHorizontalAngle;
            previousVerAngle = cam->m_fVerticalAngle;

            Camera::modeBeforeAim = mode;
        }

        targetEntity = player->m_pPointGunAt;
        mouseX = pad->NewMouseControllerState.x;
        mouseY = pad->NewMouseControllerState.y;
        disableAutoAim = !hasPadInHands && !configs.forceAutoAim;

        if (!disableAutoAim) {
            if (player->m_bHasLockOnTarget && targetEntity) {
                diff = targetEntity->GetPosition() - player->GetPosition();
                front = CGeneral::GetATanOfXY(diff.x, diff.y) - M_PI_2;

                in.x = targetEntity->GetPosition().x;
                in.y = targetEntity->GetPosition().y;
                in.z = targetEntity->GetPosition().z;

                if (targetEntity->m_nType == ENTITY_TYPE_PED) {
                    targetPed = static_cast<CPed*>(targetEntity);
                    targetPed->m_PedIK.GetComponentPosition(in, 1);
                    in.z += 0.25f;
                }

                if (CSprite::CalcScreenCoors(in, &out, &in.x, &in.y, false)) {
                    height = Camera::Find3rdPersonQuickAimPitch(out.y / SCREEN_HEIGHT);
                }

                transitionDone = (TheCamera.m_nTransitionState == 0);

                if ((transitionDone && ((hasPadInHands &&
                    (pad->Mode == 4 ? pad->NewState.LeftShoulder2 < 254 : (pad->LookAroundLeftRight() || pad->LookAroundUpDown()))) || (abs(mouseX) > 1.0f || abs(mouseY) > 1.0f))) ||
                    diff.Magnitude() < 0.5f)
                    ClearWeaponTarget(player);
            }
            else {
                if (pad->Mode == 4 ? pad->NewState.LeftShoulder2 >= 254 : pad->ShiftTargetLeftJustDown() || pad->ShiftTargetRightJustDown())
                    player->FindWeaponLockOnTarget();
            }
        }

        RotatePlayer(player, front, false);
#ifdef GTA3
        player->SetLookFlag(front, true);
#else
        player->SetLookFlag(front, true, true);
#endif
        player->SetAimFlag(front);

        player->m_fFPSMoveHeading = height;
        player->m_fFPSMoveHeading = clamp(player->m_fFPSMoveHeading, -DegToRad(45.0f), DegToRad(45.0f));

        if (configs.storiesPointingArm && info->m_bCanAimWithArm)
            player->m_fFPSMoveHeading -= DegToRad(8.0f);

        torsoPitch = 0.0f;
        if (!info->m_bCanAimWithArm || player->bIsDucking) {
            torsoPitch = player->m_fFPSMoveHeading;
        }

        if (player->m_vecMoveSpeed.Magnitude() < 0.01f)
            forceWalkaroundAnim = true;

        player->m_PedIK.MoveLimb(player->m_PedIK.m_sHead, player->m_PedIK.m_sHead.m_fYaw, 0.0f, player->m_PedIK.ms_headInfo);
        player->m_PedIK.MoveLimb(player->m_PedIK.m_sTorso, 0.0f, torsoPitch, player->m_PedIK.ms_torsoInfo);
        player->m_PedIK.MoveLimb(player->m_PedIK.m_sLowerArm, 0.0f, player->m_fFPSMoveHeading, player->m_PedIK.ms_lowerArmInfo);

#ifdef GTA3
        anim = RpAnimBlendClumpGetAssociation(player->m_pRwClump, info->m_nAnimToPlay);
        animRel = NULL;
#else
        anim = RpAnimBlendClumpGetAssociation(player->m_pRwClump, ANIM_UNARMED_PUNCHR);
        animRel = RpAnimBlendClumpGetAssociation(player->m_pRwClump, ANIM_UNARMED_FIGHTPPUNCH);
#endif
        point = true;

        if (anim && anim->m_fCurrentTime < info->m_fAnimLoopEnd) {
            point = false;
        }

        relState = (currentWeapon->m_eWeaponState == WEAPONSTATE_RELOADING && !fastReload);
        if (pad->GetWeapon() && currentWeapon->m_nAmmoInClip > 0)
            point = false;

        if (anim && pad->GetWeapon() && currentWeapon->m_eWeaponState == WEAPONSTATE_RELOADING && fastReload) {
            anim->m_fCurrentTime = info->m_fAnimLoopEnd;
        }

        if (currentWeapon->m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO || relState)
            point = false;

        if (animRel)
            point = true;

        if (point) {
            if (player->m_ePedState != PEDSTATE_AIMGUN) {
                if (player->m_ePedState != PEDSTATE_ATTACK)
                    player->SetStoredState();

                player->m_ePedState = PEDSTATE_AIMGUN;
                player->bIsPointingGunAt = true;
                player->SetMoveState(PEDMOVE_STILL);

#ifdef GTA3
                animToPlay2 = ANIM_MAN_RBLOCK_CSHOOT;
                animToPlay = info->m_nAnimToPlay;
                groupId = ANIM_GROUP_MAN;
#else
                animToPlay2 = ANIM_UNARMED_KICK_FLOOR;
                animToPlay = ANIM_UNARMED_PUNCHR;
                groupId = info->m_nAnimToPlay;
#endif

                assoc = NULL;
                if (player->bCrouchWhenShooting && player->bIsDucking) {
                    assoc = RpAnimBlendClumpGetAssociation(player->m_pRwClump, animToPlay2);
                }
                else {
                    assoc = RpAnimBlendClumpGetAssociation(player->m_pRwClump, animToPlay);
                }

                if (!assoc || assoc->m_fBlendDelta < 0.0f) {
                    if (player->bCrouchWhenShooting && player->bIsDucking) {
                        assoc = CAnimManager::BlendAnimation(player->m_pRwClump, groupId, animToPlay2, 4.0f);
                    }
                    else {
                        assoc = CAnimManager::AddAnimation(player->m_pRwClump, groupId, animToPlay);
                    }

                    assoc->m_fBlendAmount = 0.0f;
                    assoc->m_fBlendDelta = 8.0f;
                }
            }
        }

        wasPointingGun = true;

        if (player->bIsDucking && !wasCrouching) {
            player->m_ePedState = PEDSTATE_FIGHT;
            wasCrouching = true;
        }

        if (!player->bIsDucking)
            wasCrouching = false;
    }

    if (!isAiming) {
        if (wasPointingGun) {
            player->ClearPointGunAt();
            player->ClearWeaponTarget();

            if (wasCrouching) {
                if (currentWeapon->m_eWeaponState != WEAPONSTATE_OUT_OF_AMMO && currentWeapon->m_eWeaponState != WEAPONSTATE_RELOADING &&
                    IsAbleToAim(player)) {
                    player->bCrouchWhenShooting = true;
#ifdef GTA3
                    CAnimManager::BlendAnimation(player->m_pRwClump, ANIM_GROUP_MAN, ANIM_MAN_DUCK_DOWN, 4.0f);
                    SetDuck(player);
#else
                    CAnimManager::BlendAnimation(player->m_pRwClump, ANIM_GROUP_MAN, ANIM_MAN_WEAPON_CROUCH, 4.0f);
                    player->SetDuck(60000, 1);
#endif
                }
                wasCrouching = false;

#ifdef GTA3
                player->RestorePreviousState();
#endif
            }

            if (mode != Camera::modeBeforeAim && !pad->DisablePlayerControls && player->IsPedInControl()) {
                TheCamera.TakeControl(FindPlayerPed(), Camera::modeBeforeAim, 1, 0);
                TheCamera.m_bLookingAtPlayer = true;
                Camera::customTransitionSpeedPending = true;
                previousHorAngle = cam->m_fHorizontalAngle;
                previousVerAngle = cam->m_fVerticalAngle;
                Camera::modeBeforeAim = mode;
            }
            wasPointingGun = false;
        }
    }

    if (player->m_ePedState == PEDSTATE_ATTACK && IsAbleToAim(player) && ((IsTypeTwoHanded(player) && !IsTypeMelee(player) && mode == MODE_FOLLOW_PED))) {
        RotatePlayer(player, front, true);
    }

#ifdef GTA3
    if (configs.crouchKey != "NULL" && !pad->DisablePlayerControls && (mode == MODE_FOLLOW_PED || mode == MODE_AIMWEAPON) && IsAbleToAim(player)) {
        duckJustDown = InputHandler::DuckKeyDown();

        if (!player->bIsDucking && duckJustDown && player->m_eMoveState != PEDMOVE_SPRINT) {
            player->bCrouchWhenShooting = true;
            SetDuck(player);
        }
        else if (player->bIsDucking && (duckJustDown || player->m_eMoveState == PEDMOVE_SPRINT || pad->GetSprint() || pad->JumpJustDown() || pad->ExitVehicleJustDown() || (pad->GetWeapon() && !CanDuckWithThisWeapon(weaponType)))) {
            ClearDuck(player);
        }

        if (player->bIsDucking && !isAiming && (!pad->GetWeapon() || player->m_ePedState != PEDSTATE_ATTACK || !player->bIsAttacking)) {
            animAssoc = RpAnimBlendClumpGetAssociation(player->m_pRwClump, ANIM_MAN_DUCK_DOWN);

            if (!animAssoc)
                CAnimManager::BlendAnimation(player->m_pRwClump, ANIM_GROUP_MAN, ANIM_MAN_DUCK_DOWN, 4.0f);

            player->ClearPointGunAt();
            player->bIsDucking = true;
        }

        if (player->bIsDucking && currentWeapon->HasWeaponAmmoToBeUsed() && player->m_ePedState == PEDSTATE_ATTACK && currentWeapon->m_nAmmoInClip <= 0) {
            currentWeapon->m_eWeaponState = WEAPONSTATE_RELOADING;
            currentWeapon->Update(player->m_nAudioEntityId);
            currentWeapon->m_eWeaponState = WEAPONSTATE_FIRING;
        }
    }
#endif
}

void __fastcall Aiming::ProcessWeaponSwitch(CPlayerPed* ped, int, CPad* pad) {
    if (ped != FindPlayerPed())
        return;

    if (!isAiming)
        ped->ProcessWeaponSwitch(pad);
}

#ifdef GTA3
bool __fastcall Aiming::DoWeaponSmoothSpray(CPlayerPed* ped, int) {
    return ped->bIsDucking;
}

bool Aiming::CanDuckWithThisWeapon(eWeaponType weapon) {
    switch (weapon) {
        case WEAPONTYPE_COLT45:
        case WEAPONTYPE_UZI:
        case WEAPONTYPE_AK47:
        case WEAPONTYPE_M16:
            return true;
        default:
            return false;
    }
}

void Aiming::SetDuck(CPlayerPed* ped) {
    CAnimBlendAssociation* duckAssoc;

    if (ped->bIsDucking || static_cast<unsigned int>(CTimer::m_snTimeInMilliseconds) <= ped->m_nDuckTimer) {
        if (static_cast<unsigned int>(CTimer::m_snTimeInMilliseconds + 60000) > ped->m_nDuckTimer)
            ped->m_nDuckTimer = CTimer::m_snTimeInMilliseconds + 60000;
        return;
    }

    duckAssoc = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, ANIM_MAN_DUCK_DOWN);
    if (!duckAssoc || duckAssoc->m_fBlendDelta < 0.0f) {
        CAnimManager::BlendAnimation(ped->m_pRwClump, ANIM_GROUP_MAN, ANIM_MAN_DUCK_DOWN, 4.0f);
        ped->bIsDucking = true;
        ped->m_nDuckTimer = CTimer::m_snTimeInMilliseconds + 60000;
    }
}

void Aiming::ClearDuck(CPlayerPed* ped) {
    CAnimBlendAssociation* animAssoc;

    animAssoc = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, ANIM_MAN_DUCK_DOWN);
    if (!animAssoc) {
        animAssoc = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, ANIM_MAN_DUCK_LOW);
    }

    if (!animAssoc) {
        animAssoc = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, ANIM_MAN_RBLOCK_CSHOOT);
    }

    if (animAssoc) {
        animAssoc->m_nFlags |= ASSOC_DELETEFADEDOUT;
        animAssoc->m_fBlendDelta = -4.0f;
    }
    ped->bIsDucking = false;

    ped->m_nDuckTimer = 0;
    ped->bCrouchWhenShooting = false;
    ped->RestorePreviousState();
}

void Aiming::AdjustWeaponInfoForCrouch(CPlayerPed* ped) {
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
    CWeaponInfo* info;

    if (!ped->bIsDucking)
        return;

    info = CWeaponInfo::GetWeaponInfo(weaponType);

    info->m_nAnimToPlay = ANIM_MAN_RBLOCK_CSHOOT;
    info->m_nAnim2ToPlay = ANIM_MAN_RBLOCK_CSHOOT;

    info->m_fAnimLoopStart = clamp(info->m_fAnimLoopStart, info->m_fAnimLoopStart, 12.0f / 30.0f);
    info->m_fAnimLoopEnd = clamp(info->m_fAnimLoopEnd, info->m_fAnimLoopEnd, 15.0f / 30.0f);

    info->m_fAnimFrameFire = clamp(info->m_fAnimFrameFire, info->m_fAnimFrameFire, 14.0f / 30.0f);
    info->m_fAnim2FrameFire = info->m_fAnimFrameFire;

    info->m_bCanAimWithArm = false;

    wantsToResetWeaponInfo = true;
}

void Aiming::ResetWeaponInfo(CPlayerPed* ped) {
    if (!wantsToResetWeaponInfo)
        return;

    memcpy(&aWeaponInfo, backupWeaponInfo, sizeof(CWeaponInfo) * WEAPONTYPE_HELICANNON);
    wantsToResetWeaponInfo = false;
}

#else
float __fastcall Aiming::DoWeaponSmoothSpray(CPlayerPed* ped, int) {
    if (ped->bIsDucking)
        return 0.00001f;
    return -1.0f;
}

void __fastcall Aiming::ClearAimFlag(CPed* playa, int) {
    if (!isAiming)
        playa->ClearAimFlag();
}

void Aiming::OnDucking(CPed* ped, int, __int16) {
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
    CWeaponInfo* info;

    if (ped != FindPlayerPed())
        return;

    info = CWeaponInfo::GetWeaponInfo(weaponType);

    if (!info->m_bCrouchFire && !IsTypeMelee(ped)) {
        ped->bCrouchWhenShooting = false;
        ped->RestorePreviousState();
    }
}

void Aiming::OnClearDuck(CPed* ped, char) {
    if (ped != FindPlayerPed())
        return;

    ped->ClearPointGunAt();
    wasCrouching = false;
}

void Aiming::OnFindingNewLockOnTarget(CPlayerPed* ped) {
    const eWeaponType weaponType = ped->m_aWeapons[ped->m_nCurrentWeapon].m_eWeaponType;
    CWeaponInfo* info;

    if (ped != FindPlayerPed())
        return;

    info = CWeaponInfo::GetWeaponInfo(weaponType);

    if (!(!ped->bIsDucking || info->m_bCrouchFire)) {
        ped->ClearPointGunAt();
        ped->ClearWeaponTarget();
    }
}
#endif

void Aiming::OnFindingLockOnTarget(CPlayerPed* ped) {
    bool disableAutoAim;

    disableAutoAim = !InputHandler::HasPadInHands() && !configs.forceAutoAim;
    if (disableAutoAim || (ped->m_pPointGunAt && !ped->m_pPointGunAt->GetIsOnScreenComplex()))
        ped->ClearWeaponTarget();
}

void Aiming::OnProcessingPlayerControlBefore(CPed* ped) {
    CPlayerPed* player = static_cast<CPlayerPed*>(ped);

#ifdef GTA3
    AdjustWeaponInfoForCrouch(player);
#endif
    ProcessPlayerPedControl(player);
    Find3rdPersonMouseTarget(player);
}

void Aiming::OnProcessingPlayerControlAfter(CPed* ped) {
    CPlayerPed* player = static_cast<CPlayerPed*>(ped);

#ifdef GTA3
    if (wantsToResetWeaponInfo)
        ResetWeaponInfo(player);
#endif
}
