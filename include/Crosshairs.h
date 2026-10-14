#pragma once
#include "PluginBase.h"
#include "CRGBA.h"
#include "RenderWare.h"

class Crosshairs {
public:
    static RwV3d lastLockOnPos;
    static int lockOnExpireTimeMs;
    static CRGBA lastLockOnColor;

    static void Clear();
    static void DrawCrosshair();
    static void DrawAutoAimTarget();
    static void DrawTriangleForMouseRecruitPed();

    static void DrawSATarget(float x, float y, float dist, float rotationMult, const CRGBA& col);
    static void DrawLCSTarget(float x, float y, float dist, const CRGBA& col);
    static void DrawSATriangleForMouseRecruit(float x, float y, float dist, const CRGBA& col);
};
