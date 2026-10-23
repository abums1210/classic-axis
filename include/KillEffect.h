#pragma once
#include "PluginBase.h"
#include "RenderWare.h"

struct KillEffectParams {
    float desaturation;
    float flash;
    float vignette;
    float progress;
};

class KillEffect {
public:
    static bool m_bActive;
    static unsigned int m_nStartTime, m_nDuration;
    static void* m_pPixelShader;
    static RwRaster* m_pRaster;

    static void Init();
    static void Trigger(unsigned int duration = 250);
    static void Render();
    static void Clear();
    static void Shutdown();
};
