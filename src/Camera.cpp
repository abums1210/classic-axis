#include "Camera.h"
#include "Aiming.h"
#include "Configs.h"
#include "CamNew.h"
#include "Utility.h"
#include "common.h"
#include "CWorld.h"
#include "CGeneral.h"
#include "CMenuManager.h"
#include "CDraw.h"
#include "CPad.h"

short Camera::modeBeforeAim = 0;
short Camera::tempFiringCamMode = -1;
bool Camera::customTransitionSpeedPending = false;

void Camera::Clear() {
    customTransitionSpeedPending = false;
    tempFiringCamMode = -1;

    if (!CamNew)
        CamNew = std::make_unique<CCamNew>();

    modeBeforeAim = CamNew->cam->m_nCamMode;
}

void __fastcall Camera::ProcessFollowPedWithMouse(CCam* cam, int, CVector const& target, float targetOrient, float, float) {
    if (CamNew)
        CamNew->Process_FollowPed(target, targetOrient, 0.0f, 0.0f);
}

int __fastcall Camera::GetPlayerMovementType(int, int) {
    int moveType = TYPE_WALKAROUND;

    if (Aiming::forceWalkaroundAnim) {
        Aiming::forceWalkaroundAnim = false;
        return static_cast<int>(TYPE_WALKAROUND);
    }

    if (Aiming::isAiming && !Aiming::ignoreRotation)
        moveType = TYPE_STRAFE;

    return moveType;
}

int __fastcall Camera::GetPlayerShootingDirection(int, int) {
    int moveType = TYPE_WALKAROUND;

    if (Aiming::isAiming && !CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_bHasLockOnTarget)
        moveType = TYPE_STRAFE;

    return moveType;
}

void Camera::OnFiringBefore(CWeapon*, CEntity*, CVector*) {
    if (tempFiringCamMode == -1) {
        tempFiringCamMode = TheCamera.m_asCams[TheCamera.m_nActiveCam].m_nCamMode;
        TheCamera.m_asCams[TheCamera.m_nActiveCam].m_nCamMode = MODE_FOLLOW_PED;
    }
}

void Camera::OnFiringAfter(CWeapon*, CEntity*, CVector*) {
    if (tempFiringCamMode != -1) {
        TheCamera.m_asCams[TheCamera.m_nActiveCam].m_nCamMode = tempFiringCamMode;
        tempFiringCamMode = -1;
    }
}

bool __fastcall Camera::DisableNative3rdPersonCam(CCam*, int) {
    return false;
}

void Camera::OnStartingTransition(CCam* cam, short mode) {
    const int transitionDuration = 500;

    if (mode == MODE_AIMWEAPON || mode == MODE_FOLLOW_PED) {
        if (customTransitionSpeedPending) {
            TheCamera.m_nTransitionDuration = transitionDuration;
#ifdef GTAVC
            TheCamera.m_nTransitionDurationTargetCoors = transitionDuration;
#endif
            TheCamera.m_fFractionInterToStopMoving = 0.1f;
            TheCamera.m_fFractionInterToStopCatchUp = 0.9f;

            customTransitionSpeedPending = false;
            camUseCurrentAngle = true;
        }
    }
}

void Camera::OnProcessingCam(CCam* cam) {
    CEntity* entity;
    CVector target;
    float targetOrient;

    entity = cam->m_pCamTargetEntity;
    target = entity->GetPosition();
    CMatrix& mat = entity->GetMatrix();

    if (mat.at.x == 0.0f && mat.at.y == 0.0f)
        targetOrient = 0.0f;
    else
        targetOrient = CGeneral::GetATanOfXY(mat.at.x, mat.at.y);

    switch (cam->m_nCamMode) {
        case MODE_AIMWEAPON:
            if (CamNew)
                CamNew->Process_AimWeapon(target, targetOrient, 0.0f, 0.0f);
            break;
    }
}

void __fastcall Camera::CamControl(int, int) {
    TheCamera.m_bUseMouse3rdPerson = true;
    FrontEndMenuManager.m_nControlMethod = 0;
    TheCamera.m_f3rdPersonCHairMultX = configs.cameraCrosshairMultX;
    TheCamera.m_f3rdPersonCHairMultY = configs.cameraCrosshairMultY;

    TheCamera.CamControl();
}

void Camera::OnSet1stPersonPlayerCamMode(CCam* cam, int, int, int) {
    CPlayerPed* player;
    float angle;

    if (TheCamera.m_asCams[TheCamera.m_nActiveCam].m_nCamMode == MODE_FOLLOW_PED) {
        player = FindPlayerPed();
        angle = CGeneral::LimitRadianAngle(-TheCamera.m_fOrientation);
        Aiming::RotatePlayer(player, angle, false);
    }
}

float Camera::Find3rdPersonQuickAimPitch(float y) {
    CCam* cam;
    float fov;

    cam = &TheCamera.m_asCams[TheCamera.m_nActiveCam];

    fov = tan(cam->m_fFOV * 0.5f * 0.01403292f);
    fov *= GetAspectRatio() / (4.0f / 3.0f);

    return -(atan2(
        fov
        * (1.0f - 2.0f * TheCamera.m_f3rdPersonCHairMultY)
        * (1.0f / GetAspectRatio()),
        1.0f)
        + cam->m_fVerticalAngle);
}

bool __fastcall Camera::Find3rdPersonCamTargetVector(CCamera* camera, int, float dist, CVector pos, CVector& source, CVector& target) {
    CCam* cam;
    float fov, x, y;

    cam = &camera->m_asCams[camera->m_nActiveCam];

    if (CPad::GetPad(0)->GetLookBehindForPed()) {
        source = pos;
        CMatrix& mat = cam->m_pCamTargetEntity->GetMatrix();
        target = dist * mat.up + source;
        return false;
    }
    else {
        fov = tan(CDraw::ms_fFOV * 0.5f * (M_PI / 180.0f));
        fov *= GetAspectRatio() / (4.0f / 3.0f);
        x = fov * 2.0f * (camera->m_f3rdPersonCHairMultX - 0.5f);
        y = fov * 2.0f * (0.5f - camera->m_f3rdPersonCHairMultY);
        y *= (1.0f / GetAspectRatio());

        source = cam->m_vecSource;
        target = cam->m_vecFront;
        target += cam->m_vecUp * y;
        target += CrossProduct(cam->m_vecFront, cam->m_vecUp) * x;
        target.Normalize();
        source += DotProduct(pos - source, target) * target;
        target = dist * target + source;
        return true;
    }
}
