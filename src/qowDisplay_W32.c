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

#include "../../icd_tarzgl4/src/zglUtils.h"
#include "../../icd_tarzgl4/src/zglCommands.h"
#include "../../icd_tarzgl4/src/zglObjects.h"
#include "qowBase.h"
#include "qowVideo_W32.h"

//#define _AVOID_WGL_FUNCS 1

//#define USE_SCREEN_DC 1
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "setupapi.lib")

#define COBJMACROS
#include <windows.h>
#include <dxgi.h>
#include <initguid.h>  // Required to define DXGI IIDs
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "dxgi.lib")
// Manually define IID_IDXGIFactory (for C projects)
DEFINE_GUID(IID_IDXGIFactory,
    0x7b7166ec, 0x21c7, 0x44ae, 0xb2, 0x1a, 0xc9, 0xae, 0x32, 0x1a, 0xe3, 0x69);

int ListDisplayPortsDxgi() {
    IDXGIFactory* pFactory = NULL;
    HRESULT hr = CreateDXGIFactory(&IID_IDXGIFactory, (void**)&pFactory);
    if (FAILED(hr)) {
        printf("Failed to create DXGIFactory: 0x%08x\n", hr);
        return -1;
    }

    IDXGIAdapter* pAdapter = NULL;
    UINT adapterIndex = 0;

    while (SUCCEEDED(IDXGIFactory_EnumAdapters(pFactory, adapterIndex, &pAdapter))) {
        printf("Adapter %u:\n", adapterIndex);

        IDXGIOutput* pOutput = NULL;
        UINT outputIndex = 0;

        while (SUCCEEDED(IDXGIAdapter_EnumOutputs(pAdapter, outputIndex, &pOutput))) {
            DXGI_OUTPUT_DESC desc;
            IDXGIOutput_GetDesc(pOutput, &desc);

            DXGI_MODE_DESC currentMode;
            UINT numModes = 0;

            // Get number of supported display modes for current format
            hr = IDXGIOutput_GetDisplayModeList(pOutput, DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, NULL);
            if (SUCCEEDED(hr) && numModes > 0) {
                DXGI_MODE_DESC* pModes = (DXGI_MODE_DESC*)malloc(sizeof(DXGI_MODE_DESC) * numModes);
                if (pModes) {
                    hr = IDXGIOutput_GetDisplayModeList(pOutput, DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, pModes);
                    if (SUCCEEDED(hr)) {
                        // Find the current display mode
                        for (UINT i = 0; i < numModes; ++i) {
                            if (pModes[i].ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED &&
                                pModes[i].Scaling == DXGI_MODE_SCALING_UNSPECIFIED) {
                                currentMode = pModes[i];
                                //break;
                            }
                            currentMode = pModes[i];


                            UINT refreshRate = 0;
                            if (currentMode.RefreshRate.Denominator != 0) {
                                refreshRate = currentMode.RefreshRate.Numerator / currentMode.RefreshRate.Denominator;
                            }

                            wprintf(L"  Monitor %u: %s\n", outputIndex, desc.DeviceName);
                            printf("    Resolution: %ux%u\n", currentMode.Width, currentMode.Height);
                            printf("    Refresh Rate: %u Hz\n", refreshRate);
                        }
                    }
                    free(pModes);
                }
            }

            IDXGIOutput_Release(pOutput);
            outputIndex++;
        }

        IDXGIAdapter_Release(pAdapter);
        adapterIndex++;
    }

    IDXGIFactory_Release(pFactory);
    return 0;
}

BOOL GetMonitorNameFromHMONITOR(HMONITOR hMonitor, char* outName, size_t outNameSize);

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    afxModule icd = (afxModule)dwData;

    MONITORINFOEXA mon = { 0 };
    mon.cbSize = sizeof(mon);

    GetMonitorInfoA(hMonitor, &mon);
    // Compare the HDCs to find the corresponding monitor
    //if (hdcMonitor == hdcScreen)
    {
        // This is the monitor associated with the given HDC
        // You can perform additional actions here.
    }

    DEVMODEA ddm = { 0 };
    ddm.dmSize = sizeof(ddm);

    EnumDisplaySettingsExA(mon.szDevice, ENUM_REGISTRY_SETTINGS, &ddm, NIL);

    afxDisplay dpy;
    avxDisplayInfo dpi = { 0 };

    dpi.dimWh[0] = ddm.dmPelsWidth;
    dpi.dimWh[1] = ddm.dmPelsHeight;
    dpi.resWh[0] = ddm.dmPelsWidth;
    dpi.resWh[1] = ddm.dmPelsHeight;
    dpi.dev.type = afxDeviceType_DISPLAY;
    AfxStrcpy(dpi.name, mon.szDevice);

    //dpi.ports[0].workArea = (afxRect) { .x = mon.rcWork.left, .y = mon.rcWork.top, .w = mon.rcWork.right, .h = mon.rcWork.bottom };
    //dpi.ports[0].fullArea = (afxRect) { .x = mon.rcMonitor.left, .y = mon.rcMonitor.top, .w = mon.rcMonitor.right, .h = mon.rcMonitor.bottom };

    afxString s;
    AfxMakeString(&s, ARRAY_SIZE(dpi.label), dpi.label, 0);
    AfxFormatString(&s, "%s <%s>", "OEM", mon.szDevice);
    AfxMakeString(&dpi.dev.urn, 0, s.start, s.len);

    GetMonitorNameFromHMONITOR(hMonitor, dpi.label, 128);

    _AvxRegisterDisplays(icd, 1, &dpi, &dpy);

    //dpy->ddinfo = dd;
    //dpy->ddminfo = dd2;

    HDC hDc = hdcMonitor;

    // Width, in pixels, of the screen; or for printers, the width, in pixels, of the printable area of the page.
    dpy->m.resWh[0] = GetDeviceCaps(hDc, HORZRES);
    // Height, in raster lines, of the screen; or for printers, the height, in pixels, of the printable area of the page.
    dpy->m.resWh[1] = GetDeviceCaps(hDc, VERTRES);

    // Width, in millimeters, of the physical screen.
    dpy->m.dimWh[0] = GetDeviceCaps(hDc, HORZSIZE);
    // Height, in millimeters, of the physical screen.
    dpy->m.dimWh[1] = GetDeviceCaps(hDc, VERTSIZE);

    // Number of adjacent color bits for each pixel.
    // NOTE: When nIndex is BITSPIXEL and the device has 15bpp or 16bpp, the return value is 16.
    dpy->m.bpp = GetDeviceCaps(hDc, BITSPIXEL);

    // Number of color planes.
    dpy->m.planeCnt = GetDeviceCaps(hDc, PLANES);

    // Number of entries in the device's color table, if the device has a color depth of no more than 8 bits per pixel. For devices with greater color depths, -1 is returned.
    dpy->m.paletteSiz = GetDeviceCaps(hDc, NUMCOLORS);

    // Flag that indicates the clipping capabilities of the device. If the device can clip to a rectangle, it is 1. Otherwise, it is 0.
    dpy->m.clipCapable = GetDeviceCaps(hDc, CLIPCAPS);

    // For display devices: the current vertical refresh rate of the device, in cycles per second (Hz).
    // A vertical refresh rate value of 0 or 1 represents the display hardware's default refresh rate. 
    // This default rate is typically set by switches on a display card or computer motherboard, or by a configuration program that does not use display functions such as ChangeDisplaySettings.
    dpy->m.freq = GetDeviceCaps(hDc, VREFRESH);

    // Value that indicates the raster capabilities of the device, as shown in the following table.
    /*
        RC_BANDING	Requires banding support.
        RC_BITBLT	Capable of transferring bitmaps.
        RC_BITMAP64	Capable of supporting bitmaps larger than 64 KB.
        RC_DI_BITMAP	Capable of supporting the SetDIBits and GetDIBits functions.
        RC_DIBTODEV	Capable of supporting the SetDIBitsToDevice function.
        RC_FLOODFILL	Capable of performing flood fills.
        RC_PALETTE	Specifies a palette-based device.
        RC_SCALING	Capable of scaling.
        RC_STRETCHBLT	Capable of performing the StretchBlt function.
        RC_STRETCHDIB	Capable of performing the StretchDIBits function.
    */
    //dpy->m.rasCaps = GetDeviceCaps(hDc, RASTERCAPS);


    return TRUE;
}

BOOL GetMonitorNameFromHMONITOR(HMONITOR hMonitor, char* outName, size_t outNameSize)
{
    if (!hMonitor || !outName || outNameSize == 0)
        return FALSE;

    MONITORINFOEXA targetInfo = { 0 };
    targetInfo.cbSize = sizeof(targetInfo);

    if (!GetMonitorInfoA(hMonitor, &targetInfo))
        return FALSE;

    RECT targetRect = targetInfo.rcMonitor;

    DISPLAY_DEVICEA adapter = { 0 };
    adapter.cb = sizeof(adapter);

    // Loop over all display devices (video adapters)
    for (DWORD adapterIdx = 0; EnumDisplayDevicesA(NULL, adapterIdx, &adapter, 0); adapterIdx++) {
        DISPLAY_DEVICEA monitor = { 0 };
        monitor.cb = sizeof(monitor);

        // Loop over monitors attached to this adapter
        for (DWORD monitorIdx = 0; EnumDisplayDevicesA(adapter.DeviceName, monitorIdx, &monitor, 0); monitorIdx++) {
            DEVMODEA devMode = { 0 };
            devMode.dmSize = sizeof(devMode);

            if (!EnumDisplaySettingsExA(monitor.DeviceName, ENUM_CURRENT_SETTINGS, &devMode, 0))
                continue;

            RECT monitorRect = {
                devMode.dmPosition.x,
                devMode.dmPosition.y,
                devMode.dmPosition.x + (LONG)devMode.dmPelsWidth,
                devMode.dmPosition.y + (LONG)devMode.dmPelsHeight
            };

            // Compare this monitor's rect with the HMONITOR's rect
            if (EqualRect(&targetRect, &monitorRect)) {
                // Copy friendly name
                strncpy_s(outName, outNameSize, monitor.DeviceString, _TRUNCATE);
                return TRUE;
            }
        }
    }

    return FALSE;
}

_QOW afxResult _ZglVduIoctrlCb(afxDisplay vdu, afxUnit reqCode, va_list va)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_VDU, 1, &vdu);
    afxResult rslt = 0;

    switch (reqCode)
    {
    case 0:
    {
        
        break;
    }
    case 1: // init
    {
        break;
    }
    case 2: // Terminates the device. The device must first be stopped and then closed (in that order). This function should be called before the system shuts down.
    {
        break;
    }
    case 3: // Opens the device. This function must only be called after the device has been initialized.
    {
        afxDrawSystem dsys = va_arg(va, afxDrawSystem);

        if (dsys)
        {

        }
        break;
    }
    case 4: // Closes the device. This function must only be called after the device has been stopped.
    {
        afxDrawSystem dsys = va_arg(va, afxDrawSystem);

        if (dsys)
        {

        }
        break;
    }
    default: break;
    }

    return rslt;
}

_QOW afxUnit _ZglChooseDisplayModes(afxDrawDevice ddev, afxUnit vdu, avxDisplayMode const* expected, avxDisplayMode* closest)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DDEV, 1, &ddev);
    afxUnit rslt = 0;

    DEVMODEA ddm = { 0 };

    afxUnit dmIdx = 0;
    while (EnumDisplaySettingsExA(ddev->dd.DeviceName, ENUM_REGISTRY_SETTINGS + dmIdx, &ddm, NIL))
    {
        ++dmIdx;
    }

    return rslt;
}

_QOW afxUnit _ZglEnumerateDisplayFormats(afxModule icd, afxUnit vdu, afxUnit cnt, avxFormat formats[])
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT(formats);
    AFX_ASSERT(cnt);
    afxUnit rslt = 0;

    afxDisplay dpy;
    if (!AfxEnumerateDisplays(icd, vdu, 1, &dpy))
    {
        AfxThrowError();
        return rslt;
    }
    AFX_ASSERT_OBJECTS(afxFcc_VDU, 1, &dpy);

#if USE_SCREEN_DC
    HDC hDc = GetDC(NIL);
#else
    HDC hDc = CreateDCA("DISPLAY", dpy->ddinfo.DeviceName, 0, NIL);
#endif

    if (!hDc)
    {
        AfxThrowError();
        return err;
    }

#if 0
    for (afxUnit i = 0; i < avxFormat_TOTAL; i++)
    {
        avxFormat fmt = i;

        avxFormatDescription pfd;
        AvxDescribeFormat(fmt, &pfd);
        
        // Skip non-color drawable formats.
        if ((pfd.flags & (avxFormatFlag_DEPTH | avxFormatFlag_STENCIL | avxFormatFlag_BC | avxFormatFlag_LUMA)))
            continue;

        if ((pfd.flags & avxFormatFlag_COLOR) != avxFormatFlag_COLOR)
            continue;

        int pxlAttrPairs[30][2] =
        {
            { WGL_SUPPORT_OPENGL_ARB, GL_TRUE },
            { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE },
            { WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB },
            { WGL_DOUBLE_BUFFER_ARB, GL_TRUE },
            { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB },

            { WGL_COLOR_BITS_ARB, pfd.bpp - pfd.bpc[3] },
            { WGL_RED_BITS_ARB, pfd.bpc[0] },
            { WGL_GREEN_BITS_ARB, pfd.bpc[1] },
            { WGL_BLUE_BITS_ARB, pfd.bpc[2] },
            { WGL_ALPHA_BITS_ARB, pfd.bpc[3] },
            { WGL_ALPHA_SHIFT_ARB, pfd.swizzle[3] },
            { WGL_RED_SHIFT_ARB, pfd.swizzle[0] },
            { WGL_GREEN_SHIFT_ARB, pfd.swizzle[1] },
            { WGL_BLUE_SHIFT_ARB, pfd.swizzle[2] },
            { WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE }, // WGL_COLORSPACE_LINEAR_EXT
            { NIL, NIL },
        };

        


        if ((pfd.flags & avxFormatFlag_sRGB) == avxFormatFlag_sRGB)
        {
            pxlAttrPairs[14][0] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
            pxlAttrPairs[14][1] = GL_TRUE;

            //pxlAttrPairs[14][0] = WGL_COLORSPACE_EXT;
            //pxlAttrPairs[14][1] = WGL_COLORSPACE_SRGB_EXT; // WGL_COLORSPACE_LINEAR_EXT
        }
        else
        {
            pxlAttrPairs[14][0] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
            pxlAttrPairs[14][1] = GL_FALSE;

            //pxlAttrPairs[14][0] = WGL_COLORSPACE_EXT;
            //pxlAttrPairs[14][1] = WGL_COLORSPACE_SRGB_EXT;
        }

        int pxlFmt = 0;
        afxUnit gdiPfdCnt = 0;
        if (wgl.ChoosePixelFormatARB(hDc, &pxlAttrPairs[0][0], NIL, 1, &pxlFmt, &gdiPfdCnt))
        //if (wglChooseBestPixelFormatSIG(hDc, &pxlAttrPairs[0][0], NIL, 1, &pxlFmt, &gdiPfdCnt))
        {
            PIXELFORMATDESCRIPTOR dcPfd = { 0 };
            dcPfd.nSize = sizeof(dcPfd);
            dcPfd.nVersion = 1;
            wgl.DescribePixelFormat(hDc, pxlFmt, sizeof(dcPfd), &dcPfd);

            if ((dcPfd.cBlueBits    != pfd.bpc[2]) ||
                (dcPfd.cGreenBits   != pfd.bpc[1]) ||
                (dcPfd.cRedBits     != pfd.bpc[0]) ||
                (dcPfd.cAlphaBits   != pfd.bpc[3]))
            {
                continue;
            }

            formats[rslt] = fmt;
            ++rslt;
        }
    }
#else

    DEVMODEA ddm = { 0 };
    ddm.dmSize = sizeof(ddm);
    if (!EnumDisplaySettingsExA(dpy->ddinfo.DeviceName, ENUM_REGISTRY_SETTINGS, &ddm, NIL))
    {
        AfxThrowError();
        return rslt;
    }

    afxUnit bpp = ddm.dmBitsPerPel;
    avxFormatDescription pfd = { 0 };
    pfd.flags |= avxFormatFlag_COLOR;
    pfd.rasCaps |= avxFormatUsage_BLIT | avxFormatUsage_DRAW;
    pfd.bpp = bpp;
    pfd.stride = bpp / AFX_BYTE_SIZE;
    pfd.compCnt = 3;
    //pfd.bpc[0] = bpp / pfd.compCnt;
    //pfd.bpc[1] = bpp / pfd.compCnt;
    //pfd.bpc[2] = bpp / pfd.compCnt;
    //pfd.bpc[3] = bpp / pfd.compCnt;
    pfd.swizzle[0] = avxColorSwizzle_B;
    pfd.swizzle[1] = avxColorSwizzle_G;
    pfd.swizzle[2] = avxColorSwizzle_R;
    pfd.swizzle[3] = avxColorSwizzle_A;
    afxUnit fmtCnt = AvxChooseFormats(&pfd, cnt, formats);

    //pfd.compCnt = 3;
    //fmtCnt += AvxChooseFormats(&pfd, cnt - fmtCnt, &formats[fmtCnt]);
    fmtCnt += 0; // break point

    for (afxUnit fmtIdx = 0; fmtIdx < fmtCnt; fmtIdx++)
    {
        pfd = (avxFormatDescription){ 0 };
        AvxDescribeFormats(1, &formats[fmtIdx], &pfd);

        int pxlAttrPairs[30][2] =
        {
            { WGL_SUPPORT_OPENGL_ARB, GL_TRUE },
            { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE },
            { WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB },
            { WGL_DOUBLE_BUFFER_ARB, GL_TRUE },
            { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB },

            { WGL_COLOR_BITS_ARB, pfd.bpp - pfd.bpc[3] },
            { WGL_RED_BITS_ARB, pfd.bpc[0] },
            { WGL_GREEN_BITS_ARB, pfd.bpc[1] },
            { WGL_BLUE_BITS_ARB, pfd.bpc[2] },
            { WGL_ALPHA_BITS_ARB, pfd.bpc[3] },
            { WGL_ALPHA_SHIFT_ARB, pfd.swizzle[3] },
            { WGL_RED_SHIFT_ARB, pfd.swizzle[0] },
            { WGL_GREEN_SHIFT_ARB, pfd.swizzle[1] },
            { WGL_BLUE_SHIFT_ARB, pfd.swizzle[2] },
            { WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE }, // WGL_COLORSPACE_LINEAR_EXT
            { NIL, NIL },
        };

        int pxlFmt = 0;
        int gdiPfdCnt = 0;
        
        afxBool byWgl;
        if (wglChooseBestPixelFormatSIG(hDc, &pxlAttrPairs[0][0], NIL, 1, &pxlFmt, &gdiPfdCnt, &byWgl))
        {
            PIXELFORMATDESCRIPTOR dcPfd = { 0 };
            dcPfd.nSize = sizeof(dcPfd);
            dcPfd.nVersion = 1;
#if USE_SCREEN_DC
            wgl.DescribePixelFormat(hDc, pxlFmt, sizeof(dcPfd), &dcPfd);
#else
            DescribePixelFormat(hDc, pxlFmt, sizeof(dcPfd), &dcPfd);
#endif

            if ((dcPfd.cBlueBits != pfd.bpc[2]) ||
                (dcPfd.cGreenBits != pfd.bpc[1]) ||
                (dcPfd.cRedBits != pfd.bpc[0]) ||
                (dcPfd.cAlphaBits != pfd.bpc[3]))
            {
                afxUnit i = fmtCnt;

                if (fmtCnt > 1) while (i)
                    formats[i - 1] = formats[i], i--;

                --fmtCnt;
            }

            ++rslt;
        }
    }
#endif

#if USE_SCREEN_DC
    ReleaseDC(NIL, hDc);
#else
    DeleteDC(hDc);
#endif

    return fmtCnt;
}

_QOW afxUnit _ZglEnumerateDisplayModes(afxModule icd, afxUnit vdu, avxFormat fmt, afxUnit cnt, avxDisplayMode modes[])
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT(modes);
    AFX_ASSERT(cnt);
    afxUnit rslt = 0;

    afxDisplay dpy;
    if (!AfxEnumerateDisplays(icd, vdu, 1, &dpy))
    {
        return rslt;
    }
    AFX_ASSERT_OBJECTS(afxFcc_VDU, 1, &dpy);

    avxFormatDescription pfd;
    AvxDescribeFormats(1, &fmt, &pfd);
    AFX_ASSERT(pfd.rasCaps);

    HDC hScreenDc = GetDC(NIL);

    afxUnit k = 0;
    DEVMODEA ddm = { 0 };
    ddm.dmSize = sizeof(ddm);
    ddm.dmSpecVersion = DM_SPECVERSION;
    // passing monitor's dd2.DeviceName always returned nothing
    if (EnumDisplaySettingsExA(dpy->ddinfo.DeviceName, ENUM_REGISTRY_SETTINGS + k, &ddm, NIL))
    {
        int pxlAttrPairs[][2] =
        {
            { WGL_SUPPORT_OPENGL_ARB, GL_TRUE },
            { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE },
            { WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB },
            { WGL_DOUBLE_BUFFER_ARB, GL_TRUE },
            { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB },

            { WGL_COLOR_BITS_ARB, pfd.bpp - pfd.bpc[3] },
            { WGL_RED_BITS_ARB, pfd.bpc[0] },
            { WGL_GREEN_BITS_ARB, pfd.bpc[1] },
            { WGL_BLUE_BITS_ARB, pfd.bpc[2] },
            { WGL_ALPHA_BITS_ARB, pfd.bpc[3] },
            { WGL_ALPHA_SHIFT_ARB, pfd.swizzle[3] },
            { WGL_RED_SHIFT_ARB, pfd.swizzle[0] },
            { WGL_GREEN_SHIFT_ARB, pfd.swizzle[1] },
            { WGL_BLUE_SHIFT_ARB, pfd.swizzle[2] },
            { NIL, NIL },
        };

        afxUnit gdiPfi[256];
        afxUnit gdiPfdCnt = 0;
        if (wglChoosePixelFormatARB(hScreenDc, &pxlAttrPairs[0][0], NIL, 256, &gdiPfi, &gdiPfdCnt))
        {
            for (afxUnit j = 0; j < gdiPfdCnt; j++)
            {
                avxDisplayMode* mode = &modes[j];

                mode->fmt = avxFormat_BGRA8un;
                //mode->bpp = ddm.dmBitsPerPel;
                mode->freq = ddm.dmDisplayFrequency;
                mode->width = ddm.dmPelsWidth;
                mode->height = ddm.dmPelsHeight;
                mode->scaling = NIL;

                if (ddm.dmDisplayFixedOutput == DMDFO_DEFAULT)
                    mode->scaling = avxVideoScaling_ONE_TO_ONE;
                else if (ddm.dmDisplayFixedOutput == DMDFO_CENTER)
                    mode->scaling = avxVideoScaling_ASPECT_RATIO;
                else if (ddm.dmDisplayFixedOutput == DMDFO_STRETCH)
                    mode->scaling = avxVideoScaling_STRETCH;

                if (!(ddm.dmDisplayFlags & DM_INTERLACED))
                    mode->scanlineOrdering = avxScanlining_PROGRESSIVE;

                ++rslt;
            }
        }
    }

    ReleaseDC(NIL, hScreenDc);

    return rslt;
}

BOOL CALLBACK MonitorEnumProc2(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    afxDrawDevice ddev = (afxDrawDevice)dwData;

    MONITORINFOEXA mon = { 0 };
    mon.cbSize = sizeof(mon);
    
    GetMonitorInfoA(hMonitor, &mon);
    // Compare the HDCs to find the corresponding monitor
    //if (hdcMonitor == hdcScreen)
    {
        // This is the monitor associated with the given HDC
        // You can perform additional actions here.
    }

    DEVMODEA ddm = { 0 };
    ddm.dmSize = sizeof(ddm);

    EnumDisplaySettingsExA(mon.szDevice, ENUM_REGISTRY_SETTINGS, &ddm, NIL);

    afxDisplay dpy;
    avxDisplayInfo dpi = { 0 };
    dpi.dimWh[0] = ddm.dmPelsWidth;
    dpi.dimWh[1] = ddm.dmPelsHeight;
    dpi.resWh[0] = ddm.dmPelsWidth;
    dpi.resWh[1] = ddm.dmPelsHeight;
    dpi.dev.type = afxDeviceType_DISPLAY;
    AfxStrcpy(dpi.name, mon.szDevice);

    afxString s;
    AfxMakeString(&s, ARRAY_SIZE(dpi.label), dpi.label, 0);
    AfxFormatString(&s, "%s <%s>", "OEM", mon.szDevice);
    //_AvxRegisterDisplays(ddev, 1, &dpi, &dpy);
    //dpy->ddinfo = dd;
    //dpy->ddminfo = dd2;

    HDC hDc = hdcMonitor;

    // Width, in pixels, of the screen; or for printers, the width, in pixels, of the printable area of the page.
    dpy->m.resWh[0] = GetDeviceCaps(hDc, HORZRES);
    // Height, in raster lines, of the screen; or for printers, the height, in pixels, of the printable area of the page.
    dpy->m.resWh[1] = GetDeviceCaps(hDc, VERTRES);

    // Width, in millimeters, of the physical screen.
    dpy->m.dimWh[0] = GetDeviceCaps(hDc, HORZSIZE);
    // Height, in millimeters, of the physical screen.
    dpy->m.dimWh[1] = GetDeviceCaps(hDc, VERTSIZE);

    // Number of adjacent color bits for each pixel.
    // NOTE: When nIndex is BITSPIXEL and the device has 15bpp or 16bpp, the return value is 16.
    dpy->m.bpp = GetDeviceCaps(hDc, BITSPIXEL);

    // Number of color planes.
    dpy->m.planeCnt = GetDeviceCaps(hDc, PLANES);

    // Number of entries in the device's color table, if the device has a color depth of no more than 8 bits per pixel. For devices with greater color depths, -1 is returned.
    dpy->m.paletteSiz = GetDeviceCaps(hDc, NUMCOLORS);

    // Flag that indicates the clipping capabilities of the device. If the device can clip to a rectangle, it is 1. Otherwise, it is 0.
    dpy->m.clipCapable = GetDeviceCaps(hDc, CLIPCAPS);

    // For display devices: the current vertical refresh rate of the device, in cycles per second (Hz).
    // A vertical refresh rate value of 0 or 1 represents the display hardware's default refresh rate. 
    // This default rate is typically set by switches on a display card or computer motherboard, or by a configuration program that does not use display functions such as ChangeDisplaySettings.
    dpy->m.freq = GetDeviceCaps(hDc, VREFRESH);

    // Value that indicates the raster capabilities of the device, as shown in the following table.
    /*
        RC_BANDING	Requires banding support.
        RC_BITBLT	Capable of transferring bitmaps.
        RC_BITMAP64	Capable of supporting bitmaps larger than 64 KB.
        RC_DI_BITMAP	Capable of supporting the SetDIBits and GetDIBits functions.
        RC_DIBTODEV	Capable of supporting the SetDIBitsToDevice function.
        RC_FLOODFILL	Capable of performing flood fills.
        RC_PALETTE	Specifies a palette-based device.
        RC_SCALING	Capable of scaling.
        RC_STRETCHBLT	Capable of performing the StretchBlt function.
        RC_STRETCHDIB	Capable of performing the StretchDIBits function.
    */
    //dpy->m.rasCaps = GetDeviceCaps(hDc, RASTERCAPS);


    return TRUE;
}

_QOW afxError _ZglVduDtorCb(afxDisplay vdu)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_VDU, 1, &vdu);

    AfxExhaustChainedClasses(&vdu->m.dev.classes);

    if (_AVX_VDU_CLASS_CONFIG.dtor(vdu))
        AfxThrowError();

    return err;
}

_QOW afxError _ZglVduCtorCb(afxDisplay vdu, void** args, afxUnit invokeNo)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_VDU, 1, &vdu);

    afxModule icd = args[0];
    AFX_ASSERT_OBJECTS(afxFcc_MDLE, 1, &icd);
    avxDisplayInfo const* info = (avxDisplayInfo const *)(args[1]) + invokeNo;
    AFX_ASSERT(info);

    if (_AVX_VDU_CLASS_CONFIG.ctor(vdu, (void*[]) { icd, (void*)info, args[2] }, 0))
    {
        AfxThrowError();
        return err;
    }

    /*
        EnumDisplayDevicesA()

        To query all display devices in the current session, call this function in a loop, starting with iDevNum set to 0,
        and incrementing iDevNum until the function fails. To select all display devices in the desktop, use only the
        display devices that have the DISPLAY_DEVICE_ATTACHED_TO_DESKTOP flag in the DISPLAY_DEVICE structure.

        To get information on the display adapter, call EnumDisplayDevices with lpDevice set to NULL.
        For example, DISPLAY_DEVICE.DeviceString contains the adapter name.

        To obtain information on a display monitor, first call EnumDisplayDevices with lpDevice set to NULL.
        Then call EnumDisplayDevices with lpDevice set to DISPLAY_DEVICE.DeviceName from the first call to
        EnumDisplayDevices and with iDevNum set to zero. Then DISPLAY_DEVICE.DeviceString is the monitor name.

        To query all monitor devices associated with an adapter, call EnumDisplayDevices in a loop with lpDevice set
        to the adapter name, iDevNum set to start at 0, and iDevNum set to increment until the function fails.
        Note that DISPLAY_DEVICE.DeviceName changes with each call for monitor information, so you must save the adapter name.
        The function fails when there are no more monitors for the adapter.

    */

    /*
        IMPORTANT: Research about the following implementation.

        Due to historical development of VGA from a basic screen controller to a full sidecar-computer,
        I concluded draw devices were not first class citizen, so not having any distintion of display devices.

        Things like "DISPLAY1" are logical devices installed to represent a connection to monitor. 
        Think I link the display port itself instead of monitor. This is why we can only retrieve video mode setting of it.

        A draw device is a GPU as a side-computer able to do work.
        A display device is a display port controller able to manager a specially configurated framebuffer and presentations to it.
        A monitor is just a remote endpoint receiving the display device output.
        A adapter is usually it all together.
    */

    HDC hScreenDc = GetDC(NULL);
    //EnumDisplayMonitors(hScreenDc, NULL, MonitorEnumProc2, (LPARAM)ddev);
    ReleaseDC(NULL, hScreenDc);

    if (err && _AVX_VDU_CLASS_CONFIG.dtor(vdu))
        AfxThrowError();

    return err;
}

BOOL CALLBACK MonitorEnumProc3(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    afxDisplay dpy = (afxDrawDevice)dwData;

    MONITORINFOEXA mon = { 0 };
    mon.cbSize = sizeof(mon);

    GetMonitorInfoA(hMonitor, &mon);
    // Compare the HDCs to find the corresponding monitor
    //if (hdcMonitor == hdcScreen)
    {
        // This is the monitor associated with the given HDC
        // You can perform additional actions here.
    }

    dpy->ports[dpy->m.portCnt].hMon = hMonitor;
    dpy->m.ports[dpy->m.portCnt].workArea = AFX_RECT(mon.rcWork.left, mon.rcWork.top, mon.rcWork.right, mon.rcWork.bottom);
    dpy->m.ports[dpy->m.portCnt].fullArea = AFX_RECT(mon.rcMonitor.left, mon.rcMonitor.top, mon.rcMonitor.right, mon.rcMonitor.bottom);
    dpy->m.ports[dpy->m.portCnt].prime = (mon.dwFlags & MONITORINFOF_PRIMARY);
    ++dpy->m.portCnt;

    return TRUE;
}

afxError RegisterPresentVdus(afxModule icd)
{
    afxError err;
#if 0
    // enumerates all display monitors (i.e., screens) that intersect with a given device context (DC) or a clipping rectangle.
    // Optional: drawing context (NULL = entire desktop).
    // Optional: restricts enumeration.
    // Callback function for each monitor.
    // User-defined data for callback.
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)icd);
#endif

    ListDisplayPortsDxgi();

#if !0
    afxUnit ddIdx = 0;
    DISPLAY_DEVICEA dd = { 0 };
    dd.cb = sizeof(dd);
    while (EnumDisplayDevicesA(NULL, ddIdx, &dd, NIL))
    {
        AfxReportComment("#%u = %s (%s) 0x%x; %s (%s)", ddIdx, dd.DeviceString, dd.DeviceName, dd.StateFlags, dd.DeviceID, dd.DeviceKey);
        ddIdx++;

#if 0
        {
            HDC hGfxDc = CreateDCA("DISPLAY", dd.DeviceName, NULL, NULL);
            int pxlAttrPairs[][2] =
            {
                { WGL_SUPPORT_OPENGL_ARB, GL_TRUE },
                { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE },
                { WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB },
                { WGL_DOUBLE_BUFFER_ARB, GL_TRUE },
                { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB },
                { WGL_COLOR_BITS_ARB, AFX_MIN(24, GetDeviceCaps(hGfxDc, BITSPIXEL)) },
                { WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB },
                { NIL, NIL },
            };

            DWORD pfdFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_SWAP_EXCHANGE | PFD_DIRECT3D_ACCELERATED;
            UINT formatCount;
            PIXELFORMATDESCRIPTOR dcPfd = { 0 };
            dcPfd.nSize = sizeof(dcPfd);
            dcPfd.dwFlags = pfdFlags;
            dcPfd.cColorBits = AFX_MIN(24, GetDeviceCaps(hGfxDc, BITSPIXEL));
            int dcPxlFmt = 0;

            dcPxlFmt = ChoosePixelFormat(hGfxDc, &dcPfd);
            DescribePixelFormat(hGfxDc, dcPxlFmt, sizeof(dcPfd), &dcPfd);
            AFX_ASSERT(dcPfd.dwFlags & pfdFlags);
            dcPxlFmt = ChoosePixelFormat(hGfxDc, &dcPfd);

            if (dcPxlFmt == 0)
                continue;

            //ddev->dd = dd;
#if 0
            DescribePixelFormat(hGfxDc, dcPxlFmt, sizeof(dcPfd), &dcPfd);

            AfxZero(&dcPfd, sizeof(dcPfd));
            dcPxlFmt = 0;

            if (!wgl.ChoosePixelFormatARB(hGfxDc, &pxlAttrPairs[0][0], NIL, 1, &dcPxlFmt, &formatCount)) AfxThrowError();
            else
            {
                AFX_ASSERT(dcPxlFmt);
                AFX_ASSERT(formatCount);
                AfxZero(&dcPfd, sizeof(dcPfd));

                wglDescribePixelFormatSIG(hGfxDc, dcPxlFmt, sizeof(dcPfd), &dcPfd);
                wglSetPixelFormatSIG(hGfxDc, dcPxlFmt, NULL, &dcPfd);

                AFX_ASSERT(dcPfd.dwFlags & pfdFlags);

            }
#endif
            DeleteDC(hGfxDc);
        }
#endif

        afxBool attachedToDesk = (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP);
        afxBool active = (dd.StateFlags & DISPLAY_DEVICE_ACTIVE);
        if (!active) continue;

        afxDisplay dpy;

        afxUnit dmIdx = 0;
        DEVMODEA ddm = { 0 };
        ddm.dmSize = sizeof(ddm);
        while (EnumDisplaySettingsExA(dd.DeviceName, ENUM_REGISTRY_SETTINGS + dmIdx, &ddm, NIL))
        {
            AfxReportComment("    %ux%u:%u@%u --- %u, (%i, %i)%u, %u, 0x%x", ddm.dmPelsWidth, ddm.dmPelsHeight, ddm.dmBitsPerPel, ddm.dmDisplayFrequency, ddm.dmLogPixels, ddm.dmPosition.x, ddm.dmPosition.y, ddm.dmDisplayOrientation, ddm.dmDisplayFixedOutput, ddm.dmDisplayFlags);
            
            if (dmIdx == 0) // register the ENUM_REGISTRY_SETTINGS version only.
            {
                avxDisplayInfo dpi = { 0 };
                dpi.dimWh[0] = ddm.dmPelsWidth;
                dpi.dimWh[1] = ddm.dmPelsHeight;
                dpi.resWh[0] = ddm.dmPelsWidth;
                dpi.resWh[1] = ddm.dmPelsHeight;
                dpi.dev.type = afxDeviceType_DISPLAY;
                AfxStrcpy(dpi.name, dd.DeviceName);
                afxString s;
                AfxMakeString(&s, ARRAY_SIZE(dpi.label), dpi.label, 0);

                AfxFormatString(&s, "%s <%s>", dd.DeviceName, dd.DeviceString);

                AfxMakeString(&dpi.dev.urn, 0, s.start, s.len);

                _AvxRegisterDisplays(icd, 1, &dpi, &dpy);
                //dpy->ddinfo = dd;
                //dpy->ddminfo = dd2;

                dpy->ddinfo = dd;

                HDC hDc = CreateDCA("DISPLAY", dd.DeviceName, 0, NIL);

                // Width, in pixels, of the screen; or for printers, the width, in pixels, of the printable area of the page.
                dpy->m.resWh[0] = GetDeviceCaps(hDc, HORZRES);
                // Height, in raster lines, of the screen; or for printers, the height, in pixels, of the printable area of the page.
                dpy->m.resWh[1] = GetDeviceCaps(hDc, VERTRES);

                // Width, in millimeters, of the physical screen.
                dpy->m.dimWh[0] = GetDeviceCaps(hDc, HORZSIZE);
                // Height, in millimeters, of the physical screen.
                dpy->m.dimWh[1] = GetDeviceCaps(hDc, VERTSIZE);

                // Number of adjacent color bits for each pixel.
                // NOTE: When nIndex is BITSPIXEL and the device has 15bpp or 16bpp, the return value is 16.
                dpy->m.bpp = GetDeviceCaps(hDc, BITSPIXEL);

                // Number of color planes.
                dpy->m.planeCnt = GetDeviceCaps(hDc, PLANES);

                // Number of entries in the device's color table, if the device has a color depth of no more than 8 bits per pixel. For devices with greater color depths, -1 is returned.
                dpy->m.paletteSiz = GetDeviceCaps(hDc, NUMCOLORS);

                // Flag that indicates the clipping capabilities of the device. If the device can clip to a rectangle, it is 1. Otherwise, it is 0.
                dpy->m.clipCapable = GetDeviceCaps(hDc, CLIPCAPS);

                // For display devices: the current vertical refresh rate of the device, in cycles per second (Hz).
                // A vertical refresh rate value of 0 or 1 represents the display hardware's default refresh rate. 
                // This default rate is typically set by switches on a display card or computer motherboard, or by a configuration program that does not use display functions such as ChangeDisplaySettings.
                dpy->m.freq = GetDeviceCaps(hDc, VREFRESH);

                // Value that indicates the raster capabilities of the device, as shown in the following table.
                /*
                    RC_BANDING	Requires banding support.
                    RC_BITBLT	Capable of transferring bitmaps.
                    RC_BITMAP64	Capable of supporting bitmaps larger than 64 KB.
                    RC_DI_BITMAP	Capable of supporting the SetDIBits and GetDIBits functions.
                    RC_DIBTODEV	Capable of supporting the SetDIBitsToDevice function.
                    RC_FLOODFILL	Capable of performing flood fills.
                    RC_PALETTE	Specifies a palette-based device.
                    RC_SCALING	Capable of scaling.
                    RC_STRETCHBLT	Capable of performing the StretchBlt function.
                    RC_STRETCHDIB	Capable of performing the StretchDIBits function.
                */
                //dpy->m.rasCaps = GetDeviceCaps(hDc, RASTERCAPS);

                EnumDisplayMonitors(hDc, NULL, MonitorEnumProc3, (LPARAM)dpy);
                DeleteDC(hDc);

                avxDisplayMode modes[256];
                afxUnit dmCnt = 0;// _ZglEnumerateDisplayModes(icd, monIdx, avxFormat_BGRA8un, 256, modes);

                for (afxUnit i = 0; i < dmCnt; i++)
                {
                    AfxReportMessage("fmt %d %dx%d@%d %x %x %i", modes[i].fmt, modes[i].width, modes[i].height, modes[i].freq, modes[i].scaling, modes[i].scanlineOrdering, modes[i].stereo);
                }

                avxFormat formats[256];
                afxUnit fmtCnt = 0;// _ZglEnumerateDisplayFormats(icd, monIdx, 256, formats);

                for (afxUnit i = 0; i < fmtCnt; i++)
                {
                    avxFormatDescription fd;
                    AvxDescribeFormats(1, &formats[i], &fd);
                    AfxReportMessage("fmt %.*s", AfxPushString(&fd.tag));
                }
            }
            dmIdx++;

            break;
        }

        afxUnit monIdx = 0;
        DISPLAY_DEVICEA dd2 = { 0 };
        dd2.cb = sizeof(dd2);
        while (EnumDisplayDevicesA(dd.DeviceName, monIdx, &dd2, NIL))
        {
            AfxReportComment("    #%u = %s (%s) 0x%x; %s (%s)", monIdx, dd2.DeviceString, dd2.DeviceName, dd2.StateFlags, dd2.DeviceID, dd2.DeviceKey);
            
            //ddm.dmSpecVersion = DM_SPECVERSION;
            // passing dd2.DeviceName always return nothing
            /*
                If you get something like \\.\DISPLAY2 then you should be fine, but if you get something
                like \\.\DISPLAY2\Monitor0 then you can't use it directly with EnumDisplaySettings because it expects the former.
            */
            
            afxString s;
            AfxMakeString(&s, sizeof(dpy->m.ports[monIdx].name), dpy->m.ports[monIdx].name, 0);
            AfxCatenateString(&s, dd2.DeviceName, 0);
            AfxMakeString(&s, sizeof(dpy->m.ports[monIdx].label), dpy->m.ports[monIdx].label, 0);
            AfxCatenateString(&s, dd2.DeviceString, 0);
            dpy->ports[monIdx].ddminfo = dd2;
            monIdx++;
        }
    }
#endif
}
