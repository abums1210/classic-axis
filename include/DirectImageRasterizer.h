#pragma once

#include <windows.h>
#include <RenderWare.h>
#include <cstdint>

class DirectImageRasterizer {
private:
    static HMODULE GetModule() {
        static HMODULE hMod = nullptr;
        if (!hMod) {
            hMod = GetModuleHandleA("DirectImageRasterizer.VC.asi");
            if (!hMod) hMod = GetModuleHandleA("DirectImageRasterizer.asi");
            if (!hMod) hMod = GetModuleHandleA("DirectImageRasterizer.SA.asi");
            if (!hMod) hMod = GetModuleHandleA("DirectImageRasterizer.III.asi");
            if (!hMod) hMod = GetModuleHandleA("DirectImageRasterizer.VC.dll");
            if (!hMod) hMod = GetModuleHandleA("DirectImageRasterizer.dll");
            if (!hMod) hMod = LoadLibraryA("scripts\\DirectImageRasterizer.VC.asi");
            if (!hMod) hMod = LoadLibraryA("scripts\\DirectImageRasterizer.asi");
            if (!hMod) hMod = LoadLibraryA("scripts\\DirectImageRasterizer.SA.asi");
            if (!hMod) hMod = LoadLibraryA("scripts\\DirectImageRasterizer.III.asi");
            if (!hMod) hMod = LoadLibraryA("DirectImageRasterizer.VC.asi");
            if (!hMod) hMod = LoadLibraryA("DirectImageRasterizer.asi");
            if (!hMod) hMod = LoadLibraryA("DirectImageRasterizer.SA.asi");
            if (!hMod) hMod = LoadLibraryA("DirectImageRasterizer.III.asi");
        }
        return hMod;
    }

private:
    template<typename Fn>
    static Fn GetProc(const char* procName) {
        HMODULE hMod = GetModule();
        if (hMod)
            return reinterpret_cast<Fn>(GetProcAddress(hMod, procName));
        return nullptr;
    }

public:
    typedef RwTexture* (*LoadSVGToRwTextureFn)(const char*, uint32_t, uint32_t, bool, uint32_t);
    typedef RwTexture* (*LoadPNGToRwTextureFn)(const char*, uint32_t, uint32_t, bool, uint32_t);
    typedef RwTexture* (*FindSVGinFolderPathFn)(const char*, const char*, uint32_t, uint32_t, bool, uint32_t);
    typedef RwTexture* (*FindPNGinFolderPathFn)(const char*, const char*, uint32_t, uint32_t, bool, uint32_t);

    static bool IsLoaded() {
        return GetModule() != nullptr;
    }

    static RwTexture* LoadSVGToRwTexture(const char* filePath, uint32_t width = 0, uint32_t height = 0, bool generateMipmaps = true, uint32_t mipLevels = 0) {
        static LoadSVGToRwTextureFn fn = nullptr;
        if (!fn) fn = GetProc<LoadSVGToRwTextureFn>("LoadSVGToRwTexture");
        return fn ? fn(filePath, width, height, generateMipmaps, mipLevels) : nullptr;
    }

    static RwTexture* LoadPNGToRwTexture(const char* filePath, uint32_t width = 0, uint32_t height = 0, bool generateMipmaps = true, uint32_t mipLevels = 0) {
        static LoadPNGToRwTextureFn fn = nullptr;
        if (!fn) fn = GetProc<LoadPNGToRwTextureFn>("LoadPNGToRwTexture");
        return fn ? fn(filePath, width, height, generateMipmaps, mipLevels) : nullptr;
    }

    static RwTexture* FindSVGinFolderPath(const char* folderPath, const char* fileName, uint32_t width = 0, uint32_t height = 0, bool generateMipmaps = true, uint32_t mipLevels = 0) {
        static FindSVGinFolderPathFn fn = nullptr;
        if (!fn) fn = GetProc<FindSVGinFolderPathFn>("FindSVGinFolderPath");
        return fn ? fn(folderPath, fileName, width, height, generateMipmaps, mipLevels) : nullptr;
    }

    static RwTexture* FindPNGinFolderPath(const char* folderPath, const char* fileName, uint32_t width = 0, uint32_t height = 0, bool generateMipmaps = true, uint32_t mipLevels = 0) {
        static FindPNGinFolderPathFn fn = nullptr;
        if (!fn) fn = GetProc<FindPNGinFolderPathFn>("FindPNGinFolderPath");
        return fn ? fn(folderPath, fileName, width, height, generateMipmaps, mipLevels) : nullptr;
    }
};
