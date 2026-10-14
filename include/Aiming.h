#pragma once
#include "PluginBase.h"
#include "CPed.h"
#include "CPlayerPed.h"
#include "CWeaponInfo.h"
#include "CPad.h"

class Aiming {
public:
    static bool isAiming, wasPointingGun, wasCrouching;
    static bool ignoreRotation, forceWalkaroundAnim;
    static CPed* thirdPersonMouseTarget;

#ifdef GTA3
    static bool wantsToResetWeaponInfo, weaponInfoSet;
    static CWeaponInfo backupWeaponInfo[WEAPONTYPE_HELICANNON + 1];
#endif

    static void Clear();
    static bool IsAbleToAim(CPed* ped);
    static bool IsType1stPerson(CPed* ped);
    static bool IsWeaponPossiblyCompatible(CPed* ped);
    static bool IsTypeMelee(CPed* ped);
    static bool IsTypeTwoHanded(CPed* ped);
    static void RotatePlayer(CPed* ped, float angle, bool smooth);
    static void ClearWeaponTarget(CPlayerPed* ped);
    static void Find3rdPersonMouseTarget(CPlayerPed* ped);
    static void ProcessPlayerPedControl(CPlayerPed* playa);

    static void ProcessWeaponSwitch(CPlayerPed* ped, int, CPad* pad);
#ifdef GTA3
    static bool DoWeaponSmoothSpray(CPlayerPed* ped, int);
    static bool CanDuckWithThisWeapon(eWeaponType weapon);
    static void SetDuck(CPlayerPed* ped);
    static void ClearDuck(CPlayerPed* ped);
    static void AdjustWeaponInfoForCrouch(CPlayerPed* ped);
    static void ResetWeaponInfo(CPlayerPed* ped);
#else
    static float DoWeaponSmoothSpray(CPlayerPed* ped, int);
    static void ClearAimFlag(CPed* playa, int);
    static void OnDucking(CPed* ped, int, __int16);
    static void OnClearDuck(CPed* ped, char);
    static void OnFindingNewLockOnTarget(CPlayerPed* ped);
#endif

    static void OnFindingLockOnTarget(CPlayerPed* ped);
    static void OnProcessingPlayerControlBefore(CPed* ped);
    static void OnProcessingPlayerControlAfter(CPed* ped);
};
