#include "Crosshairs.h"
#include "Aiming.h"
#include "Configs.h"
#include "Utility.h"
#include "KillEffect.h"
#include "DirectImageRasterizer.h"
#include "CamNew.h"

#include "plugin.h"
#include "common.h"
#include "CWorld.h"
#include "CHud.h"
#include "CCamera.h"
#include "CGeneral.h"
#include "CTimer.h"
#include "CPad.h"
#include "CSprite.h"
#include <string>

RwV3d Crosshairs::lastLockOnPos = {};
int Crosshairs::lockOnExpireTimeMs = 0;
CRGBA Crosshairs::lastLockOnColor = { 255, 255, 255, 255 };

CSprite2d Crosshairs::sprites[TOTAL_CROSSHAIRS] = {};
bool Crosshairs::texturesLoaded = false, Crosshairs::warningShown = false, Crosshairs::hitIndicatorIsKill = false;
float Crosshairs::spreadOffset = 0.0f;
unsigned int Crosshairs::lastAmmoInClip = 0, Crosshairs::lastAmmoTotal = 0, Crosshairs::hitIndicatorExpireTimeMs = 0, Crosshairs::lastWeaponState = 0, Crosshairs::lastWeaponType = 0;

const char* const Crosshairs::spriteNames[TOTAL_CROSSHAIRS] = {
    "assault", "cross", "dot", "laserdot", "shotgun", "sitelaser",
    "siteM16", "siterocket", "sitesniper", "smg"
};

int Crosshairs::StringToSprite(const std::string& name) {
    int i;

    for (i = 0; i < TOTAL_CROSSHAIRS; i++) {
        if (_stricmp(spriteNames[i], name.c_str()) == 0)
            return i;
    }
    if (name == "sitem16") return CROSSHAIR_SITEM16;
    return CROSSHAIR_SITEM16;
}

const char* Crosshairs::SpriteToString(int sprite) {
    if (sprite >= 0 && sprite < TOTAL_CROSSHAIRS)
        return spriteNames[sprite];
    return "siteM16";
}

void Crosshairs::ShowMissingAssetsWarning() {
    if (!warningShown && configs.crosshairType != 0) {
        MessageBoxA(nullptr,
            "The custom crosshairs are missing. Mod will now use the default crosshairs. Please reinstall to use complex crosshairs.",
            "ClassicAxis",
            MB_ICONWARNING | MB_OK);
        warningShown = true;
        configs.crosshairType = 0;
    }
}

void Crosshairs::Set2DAlphaRenderStates() {
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, reinterpret_cast<void*>(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, reinterpret_cast<void*>(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, reinterpret_cast<void*>(rwFILTERLINEAR));
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, reinterpret_cast<void*>(rwTEXTUREADDRESSCLAMP));
}

void Crosshairs::InitTextures() {
    int i, p;
    bool allFound, bSuccess;
    std::string pathPrimary, pathFallback;
    const char* searchPaths[2];

    if (texturesLoaded)
        return;

    if (!DirectImageRasterizer::IsLoaded()) {
        ShowMissingAssetsWarning();
        return;
    }

    pathPrimary = GAME_PATH("charlesvercetti_mods\\ClassicAxis Remake\\crosshairs");
    pathFallback = GAME_PATH("charlesvercetti_mods\\GTA VI HUD v0.5\\crosshairs_vc");
    searchPaths[0] = pathPrimary.c_str();
    searchPaths[1] = pathFallback.c_str();

    bSuccess = false;
    for (p = 0; p < 2; p++) {
        allFound = true;
        for (i = 0; i < TOTAL_CROSSHAIRS; i++) {
            sprites[i].m_pTexture = DirectImageRasterizer::FindPNGinFolderPath(searchPaths[p], spriteNames[i], 0, 0, false);
            if (!sprites[i].m_pTexture) {
                allFound = false;
            } else {
                RwTextureSetFilterMode(sprites[i].m_pTexture, rwFILTERLINEAR);
                RwTextureSetAddressing(sprites[i].m_pTexture, rwTEXTUREADDRESSCLAMP);
            }
        }
        if (allFound) {
            bSuccess = true;
            break;
        }
    }

    if (bSuccess) {
        texturesLoaded = true;
        configs.ReadCrosshairInfo(searchPaths[p]);
    } else {
        ShowMissingAssetsWarning();
    }
}

void Crosshairs::Clear() {
    lastLockOnPos = {};
    lockOnExpireTimeMs = 0;
    lastLockOnColor = { 255, 255, 255, 255 };
    spreadOffset = 0.0f;
    lastAmmoInClip = 0;
    lastAmmoTotal = 0;
    lastWeaponState = 0;
    lastWeaponType = 0;
    hitIndicatorExpireTimeMs = 0;
    hitIndicatorIsKill = false;
}

void Crosshairs::OnPedHit(bool isKill) {
    if (configs.hitIndicator != 0) {
        if (configs.hitIndicator != 1 || isKill) {
            hitIndicatorExpireTimeMs = static_cast<unsigned int>(CTimer::m_snTimeInMilliseconds + 250);
            hitIndicatorIsKill = isKill;
        }
    }

    if (isKill && configs.killEffect) {
        KillEffect::Trigger(configs.killEffectDuration);
    }
}

void Crosshairs::DrawSprite(int spriteIdx, float centerX, float centerY, float scaleMult, const CRGBA& color) {
    float halfW, halfH;
    CRect rect;

    if (spriteIdx < 0 || spriteIdx >= TOTAL_CROSSHAIRS || !sprites[spriteIdx].m_pTexture)
        return;

    halfW = ScaleX(32.0f * scaleMult);
    halfH = ScaleY(32.0f * scaleMult);
    rect.left = centerX - halfW;
    rect.top = centerY - halfH;
    rect.right = centerX + halfW;
    rect.bottom = centerY + halfH;

    sprites[spriteIdx].Draw(rect, color, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void Crosshairs::DrawCrosshair() {
    const CrosshairConfig* pCrosshair;
    CCam cam;
    CPlayerPed* player;
    CPad* pad;
    CSprite2d* crosshair;
    CWeapon* activeWeapon;
    CWeaponInfo* weaponInfo;
    eWeaponType weaponType;
    CRGBA dotColor, crossColor;
    float x, y, baseScale, kick;
    unsigned int currentAmmoInClip, currentAmmoTotal;
    int complexSprite;
    bool isAiming, isWepCompat, isPedTargeted, firedShot;

    InitTextures();

    cam = TheCamera.m_asCams[TheCamera.m_nActiveCam];
    player = FindPlayerPed();
    pad = CPad::GetPad(0);

    if (!player || !pad || player->m_bInVehicle || pad->DisablePlayerControls || TheCamera.m_nTransitionState != 0)
        return;

    if (cam.m_nCamMode == MODE_SNIPER || cam.m_nCamMode == MODE_SNIPER_RUNABOUT)
        return;

    weaponType = player->m_aWeapons[player->m_nCurrentWeapon].m_eWeaponType;
    pCrosshair = &configs.GetCrosshair(weaponType);
    activeWeapon = &player->m_aWeapons[player->m_nCurrentWeapon];
    weaponInfo = CWeaponInfo::GetWeaponInfo(weaponType);
    currentAmmoInClip = activeWeapon->m_nAmmoInClip;
    currentAmmoTotal = activeWeapon->m_nAmmoTotal;

    x = RsGlobal.screenWidth * TheCamera.m_f3rdPersonCHairMultX;
    y = RsGlobal.screenHeight * TheCamera.m_f3rdPersonCHairMultY;

    isAiming = Aiming::isAiming && (cam.m_nCamMode == MODE_AIMWEAPON || cam.m_nCamMode == MODE_FOLLOW_PED);
    isWepCompat = Aiming::IsWeaponPossiblyCompatible(player);

    Set2DAlphaRenderStates();

    if (weaponType != static_cast<eWeaponType>(lastWeaponType)) {
        lastAmmoInClip = currentAmmoInClip;
        lastAmmoTotal = currentAmmoTotal;
        lastWeaponState = activeWeapon->m_eWeaponState;
        lastWeaponType = weaponType;
        spreadOffset = 0.0f;
    }

    if (configs.animatedCrosshairs && isAiming) {
        firedShot = (currentAmmoInClip < lastAmmoInClip) ||
                    (currentAmmoTotal < lastAmmoTotal) ||
                    (lastWeaponState != WEAPONSTATE_FIRING && activeWeapon->m_eWeaponState == WEAPONSTATE_FIRING);

        if (firedShot && (activeWeapon->m_eWeaponState == WEAPONSTATE_FIRING || activeWeapon->m_eWeaponState == WEAPONSTATE_RELOADING)) {
            kick = pCrosshair->recoilKick;
            spreadOffset += kick;
            if (spreadOffset > configs.maxRecoilKick)
                spreadOffset = configs.maxRecoilKick;
        }
        spreadOffset -= spreadOffset * configs.recoilRecoveryRate * CTimer::ms_fTimeStep;
        if (spreadOffset < 0.001f)
            spreadOffset = 0.0f;
    } else {
        spreadOffset = 0.0f;
    }
    lastAmmoInClip = currentAmmoInClip;
    lastAmmoTotal = currentAmmoTotal;
    lastWeaponState = activeWeapon->m_eWeaponState;
    lastWeaponType = weaponType;

    isPedTargeted = false;
    if (Aiming::thirdPersonMouseTarget) {
        if (player->OurPedCanSeeThisOne(Aiming::thirdPersonMouseTarget
#ifdef GTAVC
            , true
#endif
        )) {
            if (Aiming::thirdPersonMouseTarget->m_ePedState != PEDSTATE_DEAD && Aiming::thirdPersonMouseTarget->m_fHealth > 0.0f)
                isPedTargeted = true;
        }
    }

    if (configs.crosshairType == 0) {
        crosshair = &CHud::Sprites[HUD_SITEM16];
        if (crosshair && isAiming && !player->m_bHasLockOnTarget && isWepCompat) {
            crosshair->Draw(CRect(x - ScaleX(14.0f), y - ScaleY(14.0f), x + ScaleX(14.0f), y + ScaleY(14.0f)), CRGBA(255, 255, 255, 255));
        }
    } else {
        if (configs.crosshairType == 2 && isAiming && isWepCompat && !player->m_bHasLockOnTarget) {
            complexSprite = pCrosshair->sprite;
            baseScale = pCrosshair->scale;
            DrawSprite(complexSprite, x, y, baseScale + spreadOffset, CRGBA(255, 255, 255, 255));
        }

        if (!player->m_bHasLockOnTarget) {
            if (isAiming && isWepCompat) {
                dotColor = isPedTargeted ? CRGBA(255, 0, 0, 255) : CRGBA(255, 255, 255, 255);
                DrawSprite(CROSSHAIR_DOT, x, y, configs.dotScale, dotColor);
            } else if (configs.persistentCrosshairDot && !Aiming::IsTypeMelee(player) && (cam.m_nCamMode == MODE_FOLLOW_PED || cam.m_nCamMode == MODE_AIMWEAPON)) {
                dotColor = isPedTargeted ? CRGBA(255, 0, 0, 128) : CRGBA(255, 255, 255, 128);
                DrawSprite(CROSSHAIR_DOT, x, y, configs.dotScale, dotColor);
            }
        }
    }

    if (configs.hitIndicator != 0 && isAiming && isWepCompat && !player->m_bHasLockOnTarget) {
        if (static_cast<unsigned int>(CTimer::m_snTimeInMilliseconds) < hitIndicatorExpireTimeMs) {
            if (configs.hitIndicator == 2 || (configs.hitIndicator == 1 && hitIndicatorIsKill)) {
                crossColor = hitIndicatorIsKill ? CRGBA(255, 0, 0, 255) : CRGBA(255, 255, 255, 255);
                DrawSprite(CROSSHAIR_CROSS, x, y, configs.hitIndicatorScale, crossColor);
            }
        }
    }

    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(TRUE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(TRUE));
}

void Crosshairs::DrawAutoAimTarget() {
    CPlayerPed* player;
    CPed* targetPed;
    CVector in, out;
    float w, h, dist;
    CRGBA col;

    player = FindPlayerPed();
    if (!player || !player->m_pPointGunAt)
        return;

    targetPed = static_cast<CPed*>(player->m_pPointGunAt);

    in.x = targetPed->GetPosition().x;
    in.y = targetPed->GetPosition().y;
    in.z = targetPed->GetPosition().z;

    if (configs.lockOnTargetType == TARGET_SA)
        targetPed->m_PedIK.GetComponentPosition(in, 1);

    if (CSprite::CalcScreenCoors(in, &out, &w, &h, false)) {
        lastLockOnPos = out;
        lockOnExpireTimeMs = CTimer::m_snTimeInMilliseconds + 250;
    }

    dist = targetPed->m_fHealth / 100.0f;
    if (configs.animatedCrosshairs)
        dist *= (1.0f + spreadOffset * 0.8f);

    col = CRGBA(static_cast<unsigned char>((1.0f - dist) * 255), static_cast<unsigned char>(dist * 255), 0, 255);
    if (dist <= 0.0f)
        col = CRGBA(0, 0, 0, 255);

    if (lockOnExpireTimeMs > CTimer::m_snTimeInMilliseconds) {
        if (configs.lockOnTargetType == TARGET_SA)
            DrawSATarget(lastLockOnPos.x, lastLockOnPos.y, dist, 1.0f, col);
        else if (configs.lockOnTargetType == TARGET_LCS)
            DrawLCSTarget(lastLockOnPos.x, lastLockOnPos.y, dist, col);
    }
}

void Crosshairs::DrawTriangleForMouseRecruitPed() {
    CPlayerPed* player;
    CVector in, out;
    float w, h, health;
    CRGBA col;

    if (TheCamera.m_nTransitionState != 0)
        return;

    Set2DAlphaRenderStates();

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
