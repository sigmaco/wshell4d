/*
 *          ::::::::  :::       :::     :::     :::::::::  :::::::::   ::::::::
 *         :+:    :+: :+:       :+:   :+: :+:   :+:    :+: :+:    :+: :+:    :+:
 *         +:+    +:+ +:+       +:+  +:+   +:+  +:+    +:+ +:+    +:+ +:+    +:+
 *         +#+    +:+ +#+  +:+  +#+ +#++:++#++: +#+    +:+ +#++:++#:  +#+    +:+
 *         +#+  # +#+ +#+ +#+#+ +#+ +#+     +#+ +#+    +#+ +#+    +#+ +#+    +#+
 *         #+#   +#+   #+#+# #+#+#  #+#     #+# #+#    #+# #+#    #+# #+#    #+#
 *          ###### ###  ###   ###   ###     ### #########  ###    ###  ########
 *
 *                  Q W A D R O   E X E C U T I O N   E C O S Y S T E M
 *
 *                                   Public Test Build
 *                               (c) 2017 SIGMA FEDERATION
 *                             <https://sigmaco.org/qwadro/>
 */

  //////////////////////////////////////////////////////////////////////////////
 // Qwadro on Windows                                                        //
//////////////////////////////////////////////////////////////////////////////

#ifndef QOW_VIDEO_H
#define QOW_VIDEO_H

#define _CRT_SECURE_NO_WARNINGS 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <hidusage.h>

#include <intrin.h>
#include <d3d9.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxva2api.h>
#include <winddi.h>
#include <d3d11.h>
#include <d3d9.h>
#include <dxgi1_2.h>

#define _AVX_DRAW_C
#define _AVX_DISPLAY_C
#define _AVX_DISPLAY_IMPL
#define _AVX_SURFACE_C
#define _AVX_SURFACE_IMPL
#include "afx/src/draw/ddi/avxImplementation.h"
#include "../../icd_tarzgl4/src/zglDefs.h"

#include "afx/src/ux/impl/auxImplementation.h"

AFX_OBJECT(afxDisplay)
{
    AFX_OBJECT(_avxDisplay) m;

    DISPLAY_DEVICEA ddinfo;
    struct
    {
        HMONITOR hMon;
        DISPLAY_DEVICEA ddminfo;
    } ports[2];
};

AFX_OBJECT(afxSurface)
{
    AFX_OBJECT(_avxSurface) m;

    HINSTANCE               hInst;
    HWND                    hWnd;
    afxBool                 isWpp;
    HDC                     hDC;
    int                     dcPixFmt;
    PIXELFORMATDESCRIPTOR   dcPfd;
    union
    {
        struct
        {
            struct
            {
                HDC memDC; // use just one?
                HGDIOBJ oldGdiObjBkp; // required to deselect the bitmap; we can not pass NULL.
                HBITMAP hBitmap;
                void* bytemap;
            } *swaps;
        } gdi;
        struct
        {
            HGLRC       hSwapRC; // only swaps it. This is needed because hGLRC are strictly bound to a pixel format.
            HGLRC       hPrimeRC;
            int         glVerMaj;
            int         glVerMin;
            afxBool     robustCtx;
            glVmt const*gl;
            struct
            {
                GLuint  swapFbo;
                afxBool8 swapFboReady;
            } *swaps;
            afxBool swapOnWgl;
        } wgl;
        struct
        {
            ID3D11Device*   d3d11Dev;
            HANDLE          d3d11DevForGl;
            DXGI_FORMAT mSwapChainFormat;
            UINT mSwapChainFlags;
            afxBool mFirstSwap;
            IDXGISwapChain *mSwapChain;
            IDXGISwapChain1 *mSwapChain1;
            GLuint mFramebufferID;
            GLuint mColorRenderbufferID;
            HANDLE mRenderbufferBufferHandle;
            GLuint mTextureID;
            HANDLE mTextureHandle;
            afxInt mSwapInterval;
            afxInt mOrientation;
        } dxgi;
        struct
        {
            IDirect3D9Ex*       d3d9Ex;
            IDirect3DDevice9Ex* d3d9DevEx;
            HANDLE              d3d9DevExForGl;
            IDirect3DSwapChain9*d3d9Sw;
            IDirect3DTexture9* sharedTexture;
            IDirect3DSurface9* pSurface;
        } d3dsw9;
    };
};

QOW afxError _ZglVduDtorCb(afxDisplay vdu);
QOW afxError _ZglVduCtorCb(afxDisplay vdu, void** args, afxUnit invokeNo);
QOW afxResult _ZglVduIoctrlCb(afxDisplay vdu, afxUnit reqCode, va_list va);

BOOL GetMonitorNameFromHMONITOR(HMONITOR hMonitor, char* outName, size_t outNameSize);
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

afxError RegisterPresentVdus(afxModule icd);

#endif//QOW_VIDEO_H
