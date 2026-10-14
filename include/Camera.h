#pragma once
#include "PluginBase.h"
#include "CCam.h"
#include "CCamera.h"
#include "CWeapon.h"
#include "CEntity.h"
#include "CVector.h"

class Camera {
public:
    static short modeBeforeAim, tempFiringCamMode;
    static bool customTransitionSpeedPending;

    static void Clear();
    static void __fastcall ProcessFollowPedWithMouse(CCam* cam, int, CVector const& target, float targetOrient, float, float);
    static int __fastcall GetPlayerMovementType(int, int);
    static int __fastcall GetPlayerShootingDirection(int, int);
    static void OnFiringBefore(CWeapon* weapon, CEntity* entity, CVector* vector);
    static void OnFiringAfter(CWeapon* weapon, CEntity* entity, CVector* vector);
    static bool __fastcall DisableNative3rdPersonCam(CCam* cam, int);
    static void OnStartingTransition(CCam* cam, short mode);
    static void OnProcessingCam(CCam* cam);
    static void __fastcall CamControl(int, int);
    static void OnSet1stPersonPlayerCamMode(CCam* cam, int, int, int);
    static float Find3rdPersonQuickAimPitch(float y);
    static bool __fastcall Find3rdPersonCamTargetVector(CCamera* camera, int, float dist, CVector pos, CVector& source, CVector& target);
};
