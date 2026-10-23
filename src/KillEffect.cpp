#include "KillEffect.h"
#include "Configs.h"
#include "CCamera.h"
#include "CTimer.h"
#include "plugin.h"
#include "rwd3d9.h"
#include "kill_effect_shader.h"
#include <d3dx9.h>
#include <string>

bool KillEffect::m_bActive = false;
unsigned int KillEffect::m_nStartTime = 0;
unsigned int KillEffect::m_nDuration = 250;
void* KillEffect::m_pPixelShader = NULL;
RwRaster* KillEffect::m_pRaster = NULL;

static void InitShader() {
    ID3DXBuffer *shaderBuffer, *errorBuffer;
    IDirect3DDevice9* device;
    std::string shaderPath;
    HRESULT hr;

    shaderBuffer = NULL;
    errorBuffer = NULL;
    device = NULL;

    if (KillEffect::m_pPixelShader != NULL)
        return;

    shaderPath = GAME_PATH("charlesvercetti_mods\\ClassicAxis Remake\\shaders\\kill_effect.hlsl");

    hr = D3DXCompileShaderFromFileA(shaderPath.c_str(), NULL, NULL, "PS", "ps_3_0", 0, &shaderBuffer, &errorBuffer, NULL);
    if (SUCCEEDED(hr) && shaderBuffer != NULL) {
        device = (IDirect3DDevice9*)RwD3D9GetCurrentD3DDevice();
        if (device != NULL)
            device->CreatePixelShader((DWORD*)shaderBuffer->GetBufferPointer(), (IDirect3DPixelShader9**)&KillEffect::m_pPixelShader);

        shaderBuffer->Release();
        if (errorBuffer != NULL)
            errorBuffer->Release();

        if (KillEffect::m_pPixelShader != NULL)
            return;
    }

    if (errorBuffer != NULL)
        errorBuffer->Release();

    RwD3D9CreatePixelShader((const RwUInt32*)killEffectShaderBytecode, &KillEffect::m_pPixelShader);
}

void KillEffect::Init() {
    m_bActive = false;
    m_nStartTime = 0;
    m_nDuration = 250;
    m_pPixelShader = NULL;
    m_pRaster = NULL;

    LoadLibraryA("rwd3d9.dll");
    InitShader();
}

void KillEffect::Trigger(unsigned int duration) {
    m_bActive = true;
    m_nStartTime = static_cast<unsigned int>(CTimer::m_snTimeInMilliseconds);
    m_nDuration = duration;
}

void KillEffect::Clear() {
    m_bActive = false;
    m_nStartTime = 0;
}

void KillEffect::Shutdown() {
    IDirect3DPixelShader9* pShader;

    pShader = (IDirect3DPixelShader9*)m_pPixelShader;
    m_bActive = false;

    if (pShader != NULL) {
        pShader->Release();
        m_pPixelShader = NULL;
    }

    if (m_pRaster != NULL) {
        RwRasterDestroy(m_pRaster);
        m_pRaster = NULL;
    }
}

void KillEffect::Render() {
    RwCamera* cam;
    RwIm2DVertex vertices[4];
    RwImVertexIndex indices[6] = { 0, 1, 2, 0, 2, 3 };
    KillEffectParams params;
    float progress, fade, desat, flash, vignette;
    float minX, minY, maxX, maxY, maxU, maxV;
    int fbW, fbH, w, h;
    unsigned int currentTime;

    if (!m_bActive || !configs.killEffect)
        return;

    currentTime = static_cast<unsigned int>(CTimer::m_snTimeInMilliseconds);
    if (currentTime >= m_nStartTime + m_nDuration) {
        m_bActive = false;
        return;
    }

    if (m_pPixelShader == NULL)
        InitShader();

    if (m_pPixelShader == NULL)
        return;

    cam = TheCamera.m_pRwCamera;
    if (cam == NULL || cam->frameBuffer == NULL)
        return;

    fbW = cam->frameBuffer->width;
    fbH = cam->frameBuffer->height;
    if (fbW <= 0 || fbH <= 0)
        return;

    if (m_pRaster == NULL || m_pRaster->width < fbW || m_pRaster->height < fbH) {
        if (m_pRaster != NULL) {
            RwRasterDestroy(m_pRaster);
            m_pRaster = NULL;
        }

        w = 1;
        h = 1;
        while (w < fbW) w <<= 1;
        while (h < fbH) h <<= 1;

        m_pRaster = RwRasterCreate(w, h, 0, rwRASTERTYPECAMERATEXTURE);
        if (m_pRaster == NULL)
            return;
    }

    RwRasterPushContext(m_pRaster);
    RwRasterRenderFast(cam->frameBuffer, 0, 0);
    RwRasterPopContext();

    progress = static_cast<float>(currentTime - m_nStartTime) / static_cast<float>(m_nDuration);
    if (progress < 0.0f)
        progress = 0.0f;
    else if (progress > 1.0f)
        progress = 1.0f;

    fade = 1.0f - progress;
    flash = fade * fade;
    desat = fade * 0.85f;
    vignette = fade * 0.60f;

    params.desaturation = desat;
    params.flash = flash;
    params.vignette = vignette;
    params.progress = fade;

    minX = -0.5f;
    minY = -0.5f;
    maxX = static_cast<float>(fbW) - 0.5f;
    maxY = static_cast<float>(fbH) - 0.5f;
    maxU = static_cast<float>(fbW) / static_cast<float>(m_pRaster->width);
    maxV = static_cast<float>(fbH) / static_cast<float>(m_pRaster->height);

    // Top-Left (0)
    RwIm2DVertexSetScreenX(&vertices[0], minX);
    RwIm2DVertexSetScreenY(&vertices[0], minY);
    RwIm2DVertexSetScreenZ(&vertices[0], 0.0f);
    RwIm2DVertexSetRecipCameraZ(&vertices[0], 1.0f);
    RwIm2DVertexSetIntRGBA(&vertices[0], 255, 255, 255, 255);
    RwIm2DVertexSetU(&vertices[0], 0.0f, 1.0f);
    RwIm2DVertexSetV(&vertices[0], 0.0f, 1.0f);

    // Bottom-Left (1)
    RwIm2DVertexSetScreenX(&vertices[1], minX);
    RwIm2DVertexSetScreenY(&vertices[1], maxY);
    RwIm2DVertexSetScreenZ(&vertices[1], 0.0f);
    RwIm2DVertexSetRecipCameraZ(&vertices[1], 1.0f);
    RwIm2DVertexSetIntRGBA(&vertices[1], 255, 255, 255, 255);
    RwIm2DVertexSetU(&vertices[1], 0.0f, 1.0f);
    RwIm2DVertexSetV(&vertices[1], maxV, 1.0f);

    // Bottom-Right (2)
    RwIm2DVertexSetScreenX(&vertices[2], maxX);
    RwIm2DVertexSetScreenY(&vertices[2], maxY);
    RwIm2DVertexSetScreenZ(&vertices[2], 0.0f);
    RwIm2DVertexSetRecipCameraZ(&vertices[2], 1.0f);
    RwIm2DVertexSetIntRGBA(&vertices[2], 255, 255, 255, 255);
    RwIm2DVertexSetU(&vertices[2], maxU, 1.0f);
    RwIm2DVertexSetV(&vertices[2], maxV, 1.0f);

    // Top-Right (3)
    RwIm2DVertexSetScreenX(&vertices[3], maxX);
    RwIm2DVertexSetScreenY(&vertices[3], minY);
    RwIm2DVertexSetScreenZ(&vertices[3], 0.0f);
    RwIm2DVertexSetRecipCameraZ(&vertices[3], 1.0f);
    RwIm2DVertexSetIntRGBA(&vertices[3], 255, 255, 255, 255);
    RwIm2DVertexSetU(&vertices[3], maxU, 1.0f);
    RwIm2DVertexSetV(&vertices[3], 0.0f, 1.0f);

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)m_pRaster);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDZERO);

    RwD3D9SetIm2DPixelShader(m_pPixelShader);
    RwD3D9SetPixelShaderConstant(0, &params, 1);

    RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, vertices, 4, indices, 6);

    RwD3D9SetIm2DPixelShader(NULL);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)NULL);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    DefinedState();
}
