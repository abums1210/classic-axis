#pragma once
#include "PluginBase.h"
#include "CRGBA.h"
#include "CSprite2d.h"
#include "RenderWare.h"
#include "Configs.h"

class Crosshairs {
public:
    static RwV3d lastLockOnPos;
    static int lockOnExpireTimeMs;
    static CRGBA lastLockOnColor;

    static CSprite2d sprites[TOTAL_CROSSHAIRS];
    static const char* const spriteNames[TOTAL_CROSSHAIRS];
    static bool texturesLoaded, warningShown, hitIndicatorIsKill;
    static float spreadOffset;
    static unsigned int lastAmmoInClip, lastAmmoTotal, hitIndicatorExpireTimeMs, lastWeaponState, lastWeaponType;

    static void ShowMissingAssetsWarning();
    static void Set2DAlphaRenderStates();
    static void InitTextures();
    static void Clear();
    static void DrawCrosshair();
    static void DrawAutoAimTarget();
    static void DrawTriangleForMouseRecruitPed();
    static void DrawSprite(int spriteIdx, float centerX, float centerY, float scaleMult, const CRGBA& color);
    static void OnPedHit(bool isKill);

    static int StringToSprite(const std::string& name);
    static const char* SpriteToString(int sprite);

    static void DrawSATarget(float x, float y, float dist, float rotationMult, const CRGBA& col);
    static void DrawLCSTarget(float x, float y, float dist, const CRGBA& col);
    static void DrawSATriangleForMouseRecruit(float x, float y, float dist, const CRGBA& col);
};

