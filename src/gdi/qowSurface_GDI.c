/*
 *             :::::::::::     :::     :::::::::   ::::::::      :::
 *                 :+:       :+: :+:   :+:    :+: :+:    :+:   :+: :+:
 *                 +:+      +:+   +:+  +:+    +:+ +:+         +:+   +:+
 *                 +#+     +#++:++#++: +#++:++#:  :#:        +#++:++#++:
 *                 +#+     +#+     +#+ +#+    +#+ +#+   +#+# +#+     +#+
 *                 #+#     #+#     #+# #+#    #+# #+#    #+# #+#     #+#
 *                 ###     ###     ### ###    ###  ########  ###     ###
 *
 *                  Q W A D R O   E X E C U T I O N   E C O S Y S T E M
 *
 *                                   Public Test Build
 *                               (c) 2017 SIGMA FEDERATION
 *                             <https://sigmaco.org/qwadro/>
 */

#pragma comment(lib,"dwmapi.lib")

#include "../AuxOverWin32.h"

_QOW afxUnit _QowDoutIsSuspended(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);
    AfxLockFutex(&dout->m.suspendSlock, TRUE);
    afxUnit suspendCnt = dout->m.suspendCnt;
    AfxUnlockFutex(&dout->m.suspendSlock, TRUE);
    return suspendCnt;
}

_QOW afxUnit _QowDoutSuspendFunction(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);
    AfxLockFutex(&dout->m.suspendSlock, FALSE);
    afxUnit suspendCnt = ++dout->m.suspendCnt;
    AfxUnlockFutex(&dout->m.suspendSlock, FALSE);
    return suspendCnt;
}

_QOW afxUnit _QowDoutResumeFunction(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);
    AfxLockFutex(&dout->m.suspendSlock, FALSE);
    afxUnit suspendCnt = --dout->m.suspendCnt;
    AfxUnlockFutex(&dout->m.suspendSlock, FALSE);
    return suspendCnt;
}

_QOW afxError _QowDoutPresentCb_GDI(afxDrawQueue dque, avxPresentation* ctrl)
{
    afxError err = AFX_ERR_NONE;

    afxSurface dout = ctrl->dout;
    afxUnit bufIdx = ctrl->bufIdx;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    AFX_ASSERT(dout->m.presentingBufIdx == bufIdx); // must be called by _AvxDoutPresent() to handle sync.

    afxRect rc;
    avxRaster buf;
    avxCanvas canv;
    AvxGetSurfaceBuffer(dout, bufIdx, &buf);
    AvxGetSurfaceCanvas(dout, bufIdx, &canv, &rc);
    AFX_ASSERT_OBJECTS(afxFcc_CANV, 1, &canv);
    AFX_ASSERT_OBJECTS(afxFcc_RAS, 1, &buf);

    if (dout->gdi.swaps[bufIdx].memDC)
    {
        // render into framebuffer
        BitBlt(dout->hDC, 0, 0, rc.w, rc.h, dout->gdi.swaps[bufIdx].memDC, 0, 0, SRCCOPY);

        if (dout->m.presentAlpha && (dout->m.presentAlpha != avxVideoAlpha_OPAQUE))
        {
            if (!dout->m.fse)
                DwmFlush(); // always?
        }
    }

    return err;
}

void _QowPlaceWindowedSurfaceW32(HWND hwnd, afxRect const* rect)
{
    if (!rect || !IsWindow(hwnd)) return;

    // Copy desired client rectangle
    RECT desiredClientRect = { 0, 0, (LONG)rect->w, (LONG)rect->h };

    // Get window style
    DWORD style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);

    // Adjust window rect so the client area is exactly w x h
    AdjustWindowRectEx(&desiredClientRect, style, FALSE, exStyle);

    int winWidth = desiredClientRect.right - desiredClientRect.left;
    int winHeight = desiredClientRect.bottom - desiredClientRect.top;

    // Place the window so that the top-left of the client area is at (x, y)
    // But since the window includes borders and title bars, we need to offset the window
    int offsetX = rect->x + desiredClientRect.left;
    int offsetY = rect->y + desiredClientRect.top;

    SetWindowPos(hwnd,
        HWND_TOP,
        offsetX,
        offsetY,
        winWidth,
        winHeight,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}

void _QowPlaceSurfaceW32(HWND hwnd, afxRect const* area)
{
    // Get the monitor where the window is (or closest to)
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    // Get monitor info
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!GetMonitorInfo(hMonitor, &monitorInfo))
        return;

    // Use rcMonitor for full monitor (including taskbar), or rcWork for work area
    RECT monitorRect = monitorInfo.rcMonitor;

    int screenWidth = monitorRect.right - monitorRect.left;
    int screenHeight = monitorRect.bottom - monitorRect.top;

    // Define desired client area (full screen)
    RECT desiredClientRect = { area->x, area->y, area->w, area->h };

    // Adjust the window rect to account for window styles (title bar, borders, etc.)
    DWORD style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);

    AdjustWindowRectEx(&desiredClientRect, style, FALSE, exStyle);

    // Calculate width and height including decorations
    int winWidth = desiredClientRect.right - desiredClientRect.left;
    int winHeight = desiredClientRect.bottom - desiredClientRect.top;

    // Move and resize the window so that the client area fills the screen
    SetWindowPos(hwnd,
        HWND_TOP,
        area->x, area->y,                    // Top-left corner of the screen
        winWidth,
        winHeight,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}

void _QowPlaceFseSurfaceW32(HWND hwnd)
{
    // Get the monitor where the window is (or closest to)
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    // Get monitor info
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!GetMonitorInfo(hMonitor, &monitorInfo))
        return;

    // Use rcMonitor for full monitor (including taskbar), or rcWork for work area
    RECT monitorRect = monitorInfo.rcMonitor;

    int screenWidth = monitorRect.right - monitorRect.left;
    int screenHeight = monitorRect.bottom - monitorRect.top;

    // Desired client area is full monitor
    RECT desiredClientRect = { 0, 0, screenWidth, screenHeight };

    // Get window style
    DWORD style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);

    // Adjust window rect to match client area
    AdjustWindowRectEx(&desiredClientRect, style, FALSE, exStyle);

    int winWidth = desiredClientRect.right - desiredClientRect.left;
    int winHeight = desiredClientRect.bottom - desiredClientRect.top;

    // Move the window to monitor origin
    SetWindowPos(hwnd,
        HWND_TOP,
        monitorRect.left,
        monitorRect.top,
        winWidth,
        winHeight,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}

_QOW afxError _QowDoutAdjustCb_GDI(afxSurface dout, afxRect const* area, afxBool fse)
{
    afxError err = AFX_ERR_NONE;

    _AvxDoutImplAdjustCb(dout, area, fse);

    return err;
}

_QOW afxError _QowDoutRegenerateCb_GDI(afxSurface dout, afxBool build)
{
    afxError err = AFX_ERR_NONE;

    if (!dout->gdi.swaps)
        return err;

    for (afxUnit i = 0; i < dout->m.bufCnt; i++)
    {
        if (dout->gdi.swaps[i].memDC)
        {
            SelectObject(dout->gdi.swaps[i].memDC, dout->gdi.swaps[i].oldGdiObjBkp);
            DeleteObject(dout->gdi.swaps[i].hBitmap);
            dout->gdi.swaps[i].hBitmap = NIL;
            dout->gdi.swaps[i].bytemap = NIL;
            DeleteDC(dout->gdi.swaps[i].memDC);
            dout->gdi.swaps[i].memDC = NIL;
        }

        // prevent regeneration
        if (!build)
            continue;

        if (!dout->gdi.swaps[i].memDC)
        {
            BITMAPINFO bmi = { 0 };
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = dout->m.ccfg.whd.w;
            bmi.bmiHeader.biHeight = -dout->m.ccfg.whd.h; // top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            void* framebuffer = NULL;
            HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &framebuffer, NULL, 0);
            HDC memDC = CreateCompatibleDC(dout->hDC);
            dout->gdi.swaps[i].memDC = memDC;

            dout->gdi.swaps[i].oldGdiObjBkp = GetCurrentObject(memDC, OBJ_BITMAP);
            SelectObject(memDC, hBitmap);
            dout->gdi.swaps[i].hBitmap = hBitmap;

            dout->gdi.swaps[i].bytemap = framebuffer;
        }
    }

    // TODO: Create rasters and canvases...

    return err;
}

_QOW afxError _QowRelinkDoutCb_GDI(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    afxDrawSystem dsys = AvxGetSurfaceSystem(dout);
    AFX_ASSERT_OBJECTS(afxFcc_DSYS, 1, &dsys);

    afxDrawBridge dexu;
    AvxGetDrawBridges(dsys, 0, 1, &dexu);
    afxDrawDevice ddev = AvxGetBridgedDrawDevice(dexu, NIL);
    AFX_ASSERT_OBJECTS(afxFcc_DDEV, 1, &ddev);

    if (dout->hDC)
    {
        _QowDoutRegenerateCb_GDI(dout, FALSE);
        ReleaseDC(dout->hWnd, dout->hDC);
        dout->hDC = NIL;
    }

    dout->hDC = GetDC(dout->hWnd);

    if (!dout->dcPixFmt)
    {
        avxFormatDescription pfd;
        AvxDescribeFormats(1, &dout->m.ccfg.slots[0].fmt, &pfd);

        PIXELFORMATDESCRIPTOR wpfd = { 0 };
        wpfd.nSize = sizeof(wpfd);
        
        wpfd.cColorBits = pfd.bpp - pfd.bpc[3]; // WGL_COLOR_BITS_ARB must not include alpha bits.
        wpfd.cAlphaBits = pfd.bpc[3];
#if !0
        wpfd.cRedBits = pfd.bpc[0];
        wpfd.cGreenBits = pfd.bpc[1];
        wpfd.cBlueBits = pfd.bpc[2];
#endif
#if !0
        wpfd.cAlphaShift = pfd.swizzle[3];
        wpfd.cRedShift = pfd.swizzle[0];
        wpfd.cGreenShift = pfd.swizzle[1];
        wpfd.cBlueShift = pfd.swizzle[2];
#endif
        wpfd.iPixelType = PFD_TYPE_RGBA;
        wpfd.dwFlags |= PFD_DRAW_TO_WINDOW;
        
        if (dout->m.presentAlpha && (dout->m.presentAlpha != avxVideoAlpha_OPAQUE))
            wpfd.dwFlags |= PFD_SUPPORT_COMPOSITION;

        if (!(dout->dcPixFmt = ChoosePixelFormat(dout->hDC, &wpfd))) AfxThrowError();
        else
        {
            AFX_ASSERT(dout->dcPixFmt);
            
            if (!DescribePixelFormat(dout->hDC, dout->dcPixFmt, sizeof(dout->dcPfd), &dout->dcPfd))
                AfxThrowError();
            else if (!SetPixelFormat(dout->hDC, dout->dcPixFmt, &dout->dcPfd))
                AfxThrowError();

            //AFX_ASSERT(dout->dcPfd.dwFlags & pfdFlags);
        }
    }

    avxRange const screenRes =
    {
        GetDeviceCaps(dout->hDC, HORZRES),
        GetDeviceCaps(dout->hDC, VERTRES),
        GetDeviceCaps(dout->hDC, PLANES)
    };
    afxReal64 physAspRatio = (afxReal64)GetDeviceCaps(dout->hDC, HORZSIZE) / (afxReal64)GetDeviceCaps(dout->hDC, VERTSIZE);
    afxReal refreshRate = GetDeviceCaps(dout->hDC, VREFRESH);
    avxModeSetting mode = { 0 };
    AvxQuerySurfaceSettings(dout, &mode);
    mode.refreshRate = refreshRate;
    mode.wpOverHp = physAspRatio;
    mode.resolution = screenRes;
    AvxChangeSurfaceSettings(dout, &mode);

    return err;
}

_QOW afxError _QowDoutIoctlCb_GDI(afxSurface dout, afxUnit code, va_list ap)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    switch (code)
    {
    case 0:
    {
        *va_arg(ap, HDC*) = dout->hDC;
        break;
    }
    case 1:
    {
        *va_arg(ap, int*) = dout->dcPixFmt;
        break;
    }
    default: break;
    }
    return err;
}

_QOW _avxDoutDdi const _QOW_DOUT_DDI =
{
    .ioctlCb = _QowDoutIoctlCb_GDI,
    .adjustCb = _QowDoutAdjustCb_GDI,
    .lockCb = NIL,
    .unlockCb = NIL,
    .presentCb = _QowDoutPresentCb_GDI,
    .regenCb = _QowDoutRegenerateCb_GDI
};

_QOW afxError _QowDoutDtorCb_GDI(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    _QowDoutSuspendFunction(dout);

    if (dout->m.idd)
    {
        AfxDeallocate((void**)&dout->m.idd, AfxHere());
        dout->m.idd = NIL;
    }

    _QowDoutRegenerateCb_GDI(dout, FALSE);
    if (AfxDeallocate((void**)&dout->gdi.swaps, AfxHere()))
        AfxThrowError();

    AFX_ASSERT(!dout->hDC); // disconnect should have released it.

    if (dout->hDC)
        ReleaseDC(dout->hWnd, dout->hDC);

    if (_AVX_DOUT_CLASS_CONFIG.dtor(dout))
        AfxThrowError();

    return err;
}

_QOW afxError _QowDoutCtorCb_GDI(afxSurface dout, void** args, afxUnit invokeNo)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    afxDrawSystem dsys = args[0];
    AFX_ASSERT_OBJECTS(afxFcc_DSYS, 1, &dsys);
    afxSurfaceConfig const* cfg = ((afxSurfaceConfig const *)args[1]) + invokeNo;
    AFX_ASSERT(cfg);

    if (_AVX_DOUT_CLASS_CONFIG.ctor(dout, (void*[]) { dsys, (void*)cfg }, 0))
    {
        AfxThrowError();
        return err;
    }

    dout->m.pimpl = &_QOW_DOUT_DDI;

    dout->hInst = cfg->iop.w32.hInst;
    dout->hWnd = cfg->iop.w32.hWnd;
    dout->hDC = NIL;
    dout->dcPixFmt = 0;
    dout->isWpp = FALSE;

    if (AfxAllocate(dout->m.bufCnt * sizeof(dout->gdi.swaps[0]), 0, AfxHere(), (void**)&dout->gdi.swaps))
        AfxThrowError();

    for (afxUnit i = 0; i < dout->m.bufCnt; i++)
    {
        dout->gdi.swaps[i].memDC = NULL;
        dout->gdi.swaps[i].hBitmap = NULL;
        dout->gdi.swaps[i].oldGdiObjBkp = NULL;
        dout->gdi.swaps[i].bytemap = NIL;
    }

    if (_QowRelinkDoutCb_GDI(dout))
        AfxThrowError();

    HWND hWnd = cfg->iop.w32.hWnd;

    if (err && _AVX_DOUT_CLASS_CONFIG.dtor(dout))
        AfxThrowError();

    return err;
}
