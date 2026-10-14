#include "Crosshairs.h"
#include "Aiming.h"
#include "Configs.h"
#include "InputHandler.h"
#include "Utility.h"
#include "CamNew.h"
#include "common.h"
#include "CCamera.h"
#include "CPlayerPed.h"
#include "CPad.h"
#include "CHud.h"
#include "CSprite2d.h"
#include "CSprite.h"
#include "CTimer.h"

RwV3d Crosshairs::lastLockOnPos = {};
int Crosshairs::lockOnExpireTimeMs = 0;
CRGBA Crosshairs::lastLockOnColor = { 255, 255, 255, 255 };

void Crosshairs::Clear() {
    lastLockOnPos = {};
    lockOnExpireTimeMs = 0;
    lastLockOnColor = { 255, 255, 255, 255 };
}

void Crosshairs::DrawCrosshair() {
    CCam cam;
    CPlayerPed* player;
    CPad* pad;
    CSprite2d* crosshair;
    eWeaponType weaponType;
    char mode;
    float x, y;

    cam = TheCamera.m_asCams[TheCamera.m_nActiveCam];
    player = FindPlayerPed();
    pad = CPad::GetPad(0);

    if (cam.m_nCamMode != MODE_AIMWEAPON || TheCamera.m_nTransitionState != 0 || !player)
        return;

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, reinterpret_cast<void*>(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, reinterpret_cast<void*>(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(FALSE));

    weaponType = player->m_aWeapons[player->m_nCurrentWeapon].m_eWeaponType;
    mode = cam.m_nCamMode;
    x = RsGlobal.screenWidth * TheCamera.m_f3rdPersonCHairMultX;
    y = RsGlobal.screenHeight * TheCamera.m_f3rdPersonCHairMultY;

    crosshair = &CHud::Sprites[HUD_SITEM16];
    if (crosshair && Aiming::isAiming) {
        if (!player->m_bInVehicle && (mode == MODE_FOLLOW_PED || mode == MODE_AIMWEAPON) && !pad->DisablePlayerControls) {
            if (!player->m_bHasLockOnTarget && Aiming::IsWeaponPossiblyCompatible(player))
                crosshair->Draw(CRect(x - ScaleX(14.0f), y - ScaleY(14.0f), x + ScaleX(14.0f), y + ScaleY(14.0f)), CRGBA(255, 255, 255, 255));
        }
    }
}

void Crosshairs::DrawAutoAimTarget() {
    CPlayerPed* player;
    CPed* targetPed;
    CEntity* targetEntity;
    CRGBA col;
    int targetMode;
    float health, w, h, rotMult, dist;
    RwV3d in, out;

    if (configs.lockOnTargetType == TARGET_DEFAULT)
        return;

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, reinterpret_cast<void*>(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, reinterpret_cast<void*>(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(FALSE));

    player = FindPlayerPed();
    if (!player)
        return;

    targetEntity = player->m_pPointGunAt;
    col = { 0, 255, 0, 255 };
    targetMode = 0;

    if (targetEntity) {
        lastLockOnPos.x = targetEntity->GetPosition().x;
        lastLockOnPos.y = targetEntity->GetPosition().y;
        lastLockOnPos.z = targetEntity->GetPosition().z;

        if (targetEntity->m_nType == ENTITY_TYPE_PED) {
            targetPed = static_cast<CPed*>(targetEntity);
            if (targetPed) {
                targetPed->m_PedIK.GetComponentPosition(lastLockOnPos, 1);
                health = targetPed->m_fHealth / 100.0f;

                if (configs.lockOnTargetType == TARGET_SA)
                    col = CRGBA(static_cast<unsigned char>((1.0f - health) * 255), static_cast<unsigned char>(health * 255), 0, 255);
                else
                    col = CRGBA(0, static_cast<unsigned char>(health * 255), 0, 255);

                if (health <= 0.0f)
                    col = CRGBA(0, 0, 0, 255);
            }
        }

        lastLockOnPos.z += 0.25f;

        if (player->m_bHasLockOnTarget && targetEntity) {
            targetMode = 1;
            lockOnExpireTimeMs = 250 + CTimer::m_snTimeInMilliseconds;
            lastLockOnColor = col;
        }
    }

    in = lastLockOnPos;
    rotMult = 0.5f;
    dist = 1.0f;

    if (lockOnExpireTimeMs > CTimer::m_snTimeInMilliseconds) {
        if (CSprite::CalcScreenCoors(in, &out, &w, &h, false)) {
            switch (configs.lockOnTargetType) {
                case TARGET_SA:
                    dist = (w / 128.0f) * static_cast<float>((lockOnExpireTimeMs / (250 + CTimer::m_snTimeInMilliseconds)));
                    if (targetMode == 0)
                        rotMult = 3.0f;
                    DrawSATarget(out.x, out.y, dist, rotMult, lastLockOnColor);
                    break;
                case TARGET_LCS:
                    dist = w / 128.0f;
                    col.a = 150;
                    if (targetMode == 1)
                        DrawLCSTarget(out.x, out.y, dist, col);
                    break;
            }
        }
    }
    else {
        lastLockOnColor = col;
    }
}

void Crosshairs::DrawTriangleForMouseRecruitPed() {
    CPlayerPed* player;
    RwV3d in, out;
    float w, h, health;
    CRGBA col;

    if (TheCamera.m_nTransitionState != 0)
        return;

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, reinterpret_cast<void*>(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, reinterpret_cast<void*>(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(FALSE));

    player = FindPlayerPed();
    if (!player)
        return;

    if (Aiming::thirdPersonMouseTarget && player->OurPedCanSeeThisOne(Aiming::thirdPersonMouseTarget
#ifdef GTAVC
        , true
#endif
    )) {
        in.x = Aiming::thirdPersonMouseTarget->GetPosition().x;
        in.y = Aiming::thirdPersonMouseTarget->GetPosition().y;
        in.z = Aiming::thirdPersonMouseTarget->GetPosition().z;
        Aiming::thirdPersonMouseTarget->m_PedIK.GetComponentPosition(in, 1);
        in.z += 1.0f;

        health = Aiming::thirdPersonMouseTarget->m_fHealth / 100.0f;
        col = CRGBA(static_cast<unsigned char>((1.0f - health) * 255), static_cast<unsigned char>(health * 255), 0, 150);

        if (health <= 0.0f)
            col = CRGBA(0, 0, 0, 255);

        if (CSprite::CalcScreenCoors(in, &out, &w, &h, false))
            DrawSATriangleForMouseRecruit(out.x, out.y, w / 128.0f, col);
    }
}

void Crosshairs::DrawLCSTarget(float x, float y, float dist, const CRGBA& col) {
    float offset;

    dist = clamp(dist, 0.1f, 1.0f);
    offset = 32.0f * dist;

    x += ScaleY(-offset);
    DrawTriangle(x, y, ScaleY(9.0f * 0.6f), ScaleY(9.0f), CVector2D(0.0f, 0.0f), DegToRad(55.0f), CRGBA(0, 0, 0, col.a));
    DrawTriangle(x, y, ScaleY(8.0f * 0.6f), ScaleY(8.0f), CVector2D(0.0f, 0.0f), DegToRad(55.0f), col);
    x += ScaleY(offset);

    x += ScaleY(offset);
    DrawTriangle(x, y, ScaleY(9.0f * 0.6f), ScaleY(9.0f), CVector2D(0.0f, 0.0f), DegToRad(-55.0f), CRGBA(0, 0, 0, col.a));
    DrawTriangle(x, y, ScaleY(8.0f * 0.6f), ScaleY(8.0f), CVector2D(0.0f, 0.0f), DegToRad(-55.0f), col);
    x += ScaleY(-offset);

    y += ScaleY(offset * 1.5f);
    DrawTriangle(x, y, ScaleY(9.0f * 0.6f), ScaleY(9.0f), CVector2D(0.0f, 0.0f), DegToRad(180.0f), CRGBA(0, 0, 0, col.a));
    DrawTriangle(x, y, ScaleY(8.0f * 0.6f), ScaleY(8.0f), CVector2D(0.0f, 0.0f), DegToRad(180.0f), col);
}

void Crosshairs::DrawSATarget(float x, float y, float dist, float rotationMult, const CRGBA& col) {
    static float angle = 0.0f;
    float b, s;

    dist = clamp(dist, 0.6f, 1.0f);
    angle += ((0.2f * rotationMult) * CTimer::ms_fTimeStep);

    b = ScaleY(10.0f * dist);
    s = ScaleY(9.8f * dist);
    DrawTriangle(x, y, b * 0.8f, b, CVector2D(0.0f, dist + 1.0f), DegToRad(60.0f) - angle, CRGBA(0, 0, 0, col.a));
    DrawTriangle(x, y, s * 0.8f, s, CVector2D(0.0f, dist + 1.0f), DegToRad(60.0f) - angle, col);
    DrawTriangle(x, y, b * 0.8f, b, CVector2D(0.0f, dist + 1.0f), DegToRad(-60.0f) - angle, CRGBA(0, 0, 0, col.a));
    DrawTriangle(x, y, s * 0.8f, s, CVector2D(0.0f, dist + 1.0f), DegToRad(-60.0f) - angle, col);
    DrawTriangle(x, y, b * 0.8f, b, CVector2D(0.0f, dist + 1.0f), DegToRad(180.0f) - angle, CRGBA(0, 0, 0, col.a));
    DrawTriangle(x, y, s * 0.8f, s, CVector2D(0.0f, dist + 1.0f), DegToRad(180.0f) - angle, col);
}

void Crosshairs::DrawSATriangleForMouseRecruit(float x, float y, float dist, const CRGBA& col) {
    dist = clamp(dist, 0.0f, 1.0f);
    DrawTriangle(x, y, ScaleY(20.0f * dist), ScaleY(10.0f * dist), CVector2D(0.0f, 0.0f), DegToRad(180.0f), col);
}
