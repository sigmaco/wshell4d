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

#include "AuxOverWin32.h"
#pragma comment(lib,"dwmapi.lib")

extern afxKey const Scan1MakeToQwadroDereferenceMap[afxKey_TOTAL];

_QOW void CalcWindowValuesW32(HWND window, afxInt* out_extra_width, afxInt32* out_extra_height)
{
    RECT r, c;
    GetWindowRect(window, &r);
    GetClientRect(window, &c);
    *out_extra_width = (r.right - r.left) - (c.right - c.left);
    *out_extra_height = (r.bottom - r.top) - (c.bottom - c.top);
}

_QOW void CalcWindowMarginsW32(HWND window, afxUnit* left, afxUnit* top, afxUnit* right, afxUnit* bottom)
{
    RECT r, c;
    GetWindowRect(window, &r);
    GetClientRect(window, &c);
    *left = c.left - r.left;
    *top = c.top - r.top;
    *right = r.right - c.right;
    *bottom = r.bottom - c.bottom;
}

#if 0
_QOW HICON _AuxCreateWin32Icon(afxTarga const* tga, afxUnit xHotspot, afxUnit yHotspot, afxBool icon)
// Creates an RGBA icon or cursor
{
    afxError err = NIL;
    HICON handle = NIL;

    BITMAPV5HEADER bi = { 0 };
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = tga->whd[0];
    bi.bV5Height = -((LONG)tga->whd[1]);
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5AlphaMask = 0xff000000; // ARGB
    bi.bV5RedMask   = 0x00ff0000;
    bi.bV5GreenMask = 0x0000ff00;
    bi.bV5BlueMask  = 0x000000ff;

    afxByte* dst = NIL;
    HDC dc = GetDC(NULL);
    HBITMAP color = CreateDIBSection(dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&dst, NULL, (DWORD)0);
    ReleaseDC(NULL, dc);

    if (!color) AfxThrowError();
    else
    {
        HBITMAP mask;

        if (!(mask = CreateBitmap(tga->whd[0], tga->whd[1], 1, 1, NULL))) AfxThrowError();
        else
        {
            afxByte const* src = tga->data.src;

            for (afxUnit i = 0; i < tga->whd[0] * tga->whd[1]; i++)
            {
                dst[0] = src[2];
                dst[1] = src[1];
                dst[2] = src[0];
                dst[3] = src[3];
                dst += 4;
                src += 4;
            }

            ICONINFO ii = { 0 };
            ii.fIcon = icon;
            ii.xHotspot = xHotspot;
            ii.yHotspot = yHotspot;
            ii.hbmMask = mask;
            ii.hbmColor = color;

            if (!(handle = CreateIconIndirect(&ii)))
                AfxThrowError();

            DeleteObject(mask);
        }
        DeleteObject(color);
    }
    return handle;
}
#endif

_QOW HICON _AuxCreateWin32IconFromRaster(avxRaster ras, afxBool cursor, afxUnit xHotspot, afxUnit yHotspot)
// Creates an RGBA icon or cursor
{
    afxError err = NIL;
    HICON handle = NIL;

    avxRange whd = AvxGetRasterExtent(ras, 0);
    avxFormat fmt = AvxGetRasterFormat(ras);
    AFX_ASSERT(fmt == avxFormat_BGRA8un);

    BITMAPV5HEADER bi = { 0 };
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = whd.w;
    bi.bV5Height = -((LONG)whd.h);
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5AlphaMask = 0xff000000; // ARGB
    bi.bV5RedMask = 0x00ff0000;
    bi.bV5GreenMask = 0x0000ff00;
    bi.bV5BlueMask = 0x000000ff;

    afxByte* dst = NIL;
    HDC dc = GetDC(NULL);
    HBITMAP color = CreateDIBSection(dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&dst, NULL, (DWORD)0);
    ReleaseDC(NULL, dc);

    if (!color) AfxThrowError();
    else
    {
        avxRasterIo iop = { 0 };
        iop.rowStride = whd.w * (bi.bV5BitCount / 8);
        iop.rgn.extent = whd;

        if (AvxDumpRaster(ras, dst, 1, &iop, 0))
            AfxThrowError();

        afxDrawSystem dsys = AfxGetProvider(ras);
        AvxWaitForDrawBridges(dsys, 0, 0);

        HBITMAP mask;

        if (!(mask = CreateBitmap(whd.w, whd.h, 1, 1, NULL))) AfxThrowError();
        else
        {
            ICONINFO ii = { 0 };
            ii.fIcon = !cursor;
            ii.xHotspot = xHotspot;
            ii.yHotspot = yHotspot;
            ii.hbmMask = mask;
            ii.hbmColor = color;

            if (!(handle = CreateIconIndirect(&ii)))
                AfxThrowError();

            DeleteObject(mask);
        }
        DeleteObject(color);
    }
    return handle;
}

_QOW LRESULT WINAPI _QowWndHndlngPrcW32Callback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    afxError err = AFX_ERR_NONE;
    afxWindow wnd = (afxWindow)(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (wnd)
    {
        AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
        afxSession ses = AfxGetProvider(wnd);
        AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

        if (wnd->m.redrawFrameRequested)
        {
            wnd->m.redrawFrameRequested = FALSE;
            SetWindowTextA(hWnd, wnd->m.title.buf);
            RedrawWindow(hWnd, NIL, NIL, RDW_FRAME);
        }

        if (wnd->m.redrawSurfaceRequested)
        {

        }

        static afxByte buf[4096] = { 0 };
        afxUnit bufSiz = sizeof(buf);

        afxUnit32 tid = AfxGetTid();
        afxUnit32 doutTid = AfxGetObjectTid(wnd);
        AFX_ASSERT(doutTid == tid);
        
        AFX_ASSERT(wnd->hWnd == hWnd);
#if 0
        HDC dc = wnd->m.w32.hDc;

        if (dc)
        {
            SwapBuffers(dc); // deadlocks all
        }
#endif

        switch (message)
        {
        case WM_INPUT:
        case WM_INPUT_DEVICE_CHANGE:
        {
            MSG msg = { 0 };
            msg.hwnd = hWnd;
            msg.lParam = lParam;
            msg.message = message;
            msg.wParam = wParam;
            _QowProcessSystemInputMessageWin32(&msg, ses, wnd);  // we need this at focus loss to gain a last chance to release all keys.

            // RIM_INPUT --- Input occurred while the application was in the foreground.
            // The application must call DefWindowProc so the system can perform cleanup.
            break;
        }
        case WM_SYSCOMMAND: // Intercept System Commands
        {
            switch (wParam & 0xfff0)
            {
            case SC_SCREENSAVE: // screensaver trying to start
            case SC_MONITORPOWER: // monitor trying to enter powersave mode
            {
                if (wnd->m.fullscreen)
                {
                    // We are running in full screen mode, so disallow screen saver and screen blanking
                    return 0;
                }
                break;
            }
            case SC_KEYMENU: // user trying to access window menu (using ALT key)
            {
                //if (!wnd->m.w32.keymenu)
                  //  return 0;

                break;
            }
            }
            break;
        }
        case WM_CLOSE: // Did We Receive A Close Message?
        {
            // Sent as a signal that a window or an application should terminate.
            // wParam = This parameter is not used.
            // lParam = This parameter is not used.
            // If an application processes this message, it should return zero.

            //PostQuitMessage(0); // Send A Quit Message
            ShowWindow(hWnd, SW_MINIMIZE);
            return 0; // handled. Also prevent process at end of this function
        }
        case WM_ACTIVATE: // Watch For Window Activate Message
        {
            if (!HIWORD(wParam)) // Check Minimization State
            {
                wnd->m.active = TRUE; // Program Is Active
            }
            else
            {
                wnd->m.active = FALSE; // Program Is No Longer Active
            }
            return 0; // Return To The Message Loop
        }
        case WM_SHOWWINDOW:
        {
            //UpdateWindow(dout->idd->wnd);
            break;
        }
        case WM_SIZE:
        {
            //if (doutD->resizable)
            {
                /*
                    WM_SIZE
                        Sent to a window after its size has changed.

                    wParam --- The type of resizing requested. This parameter canv be one of the following values.
                        SIZE_MAXHIDE = 4 --- Message is sent to mem pop-up windows when some other window is maximized.
                        SIZE_MAXIMIZED = 2 --- The window has been maximized.
                        SIZE_MAXSHOW = 3 --- Message is sent to mem pop-up windows when some other window has been restored to its former size.
                        SIZE_MINIMIZED = 1 --- The window has been minimized.
                        SIZE_RESTORED = 0 --- The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies.

                    lParam
                        The low-order word of lParam specifies the new width of the client area.
                        The high-order word of lParam specifies the new height of the client area.

                */

                // lParam já é client area, filha da puta.

                afxRect cr = { 0 };
                cr.w = LOWORD(lParam);
                cr.h = HIWORD(lParam);

                if (cr.w * cr.h) // don't set to zero
                {
                    AfxAdjustWindow(wnd, NIL, &cr);
                }
            }
            //AfxDrawOutputProcess(dout);
            break;
        }
        case WM_DISPLAYCHANGE: // The WM_DISPLAYCHANGE message is sent to all windows when the display resolution has changed.
        {
            HDC dc;
            afxDrawOutput dout = wnd->m.dout;
            AvxCallDrawOutput(dout, 0, 0, NIL, sizeof(dc), &dc, NIL, NIL);
            avxRange const resolution = { GetDeviceCaps(dc, HORZRES), GetDeviceCaps(dc, VERTRES), GetDeviceCaps(dc, PLANES) };
            afxReal64 physAspRatio = AvxFindPhysicalAspectRatio(GetDeviceCaps(dc, HORZSIZE), GetDeviceCaps(dc, VERTSIZE));
            afxReal refreshRate = GetDeviceCaps(dc, VREFRESH);
            AvxModifyDrawOutputSettings(dout, physAspRatio, refreshRate, resolution, FALSE);

            afxDesktop* dwm = wnd->m.dwm;
            dwm->wpOverHp = physAspRatio;
            dwm->refreshRate = refreshRate;
            dwm->res.w = resolution.w;
            dwm->res.h = resolution.h;
            dwm->res.d = resolution.d;
            dwm->wrOverHr = (afxReal64)dwm->res.x / (afxReal64)dwm->res.y;
            
            break;
        }
        case WM_STYLECHANGED: // Sent to a window after the SetWindowLong function has changed one or more of the window's styles.
        {
            // Sent to a window after the SetWindowLong function has changed one or more of the window's styles. A window receives this message through its WindowProc function.
            // wParam = Indicates whether the window's styles or extended window styles have changed. This parameter can be one or more of the following values.
            
            afxUnit mleft, mtop, mright, mbottom;
            CalcWindowMarginsW32(wnd->hWnd, &mleft, &mtop, &mright, &mbottom);
            wnd->m.marginL = mleft;
            wnd->m.marginT = mtop;
            wnd->m.marginR = mright;
            wnd->m.marginB = mbottom;

            break;
        }
        case WM_MOUSEMOVE:
        {
            POINTS points = MAKEPOINTS(lParam);

            afxV2d curr = { AFX_R(points.x), AFX_R(points.y) };

            AfxV2dSub(wnd->m.cursMove, wnd->m.cursPos, curr);
            AfxV2dCopy(wnd->m.cursPos, curr);

            afxV2d screen = { AFX_R(wnd->m.frameRect.w), AFX_R(wnd->m.frameRect.h) };

            AfxV2dNdc(wnd->m.cursPosNdc, wnd->m.cursPos, screen);
            AfxV2dNdc(wnd->m.cursMoveNdc, wnd->m.cursMove, screen);

            //data2->breake = TRUE;
            break;
        }
        case WM_MOUSELEAVE:
        {
            /*
                The mouse left the client area of the window specified in a prior call to TrackMouseEvent. 
                All tracking requested by TrackMouseEvent is canceled when this message is generated. 
                The application must call TrackMouseEvent when the mouse reenters its window if it requires further tracking of mouse hover behavior.
            */
            break;
        }
        case WM_MOUSEHOVER:
        {
            /*
                The mouse hovered over the client area of the window for the period of time specified in a prior call to TrackMouseEvent. 
                Hover tracking stops when this message is generated. The application must call TrackMouseEvent again if it requires further 
                tracking of mouse hover behavior.
            */
            break;
        }
        case WM_NCMOUSELEAVE:
        {
            // The same meaning as WM_MOUSELEAVE except this is for the nonclient area of the window.
            break;
        }
        case WM_NCMOUSEHOVER:
        {
            // The same meaning as WM_MOUSEHOVER except this is for the nonclient area of the window.
            break;
        }
#if 0
        case WM_DROPFILES: // Sent when the user drops a file on the window of an application that has registered itself as a recipient of dropped files.
        {
            // TODO build a kind of handler for draw input to deal with it.

            afxFileDrop fdrop;
            AFX_ZERO(&fdrop);

            POINT ppt;
            HDROP hDrop = (HDROP)wParam;
            DragQueryPoint(hDrop, &ppt);

            fdrop.x = ppt.x;
            fdrop.y = ppt.y;
            afxUnit cnt = DragQueryFileA(hDrop, 0xFFFFFFFF, NIL, NIL);

            AfxMakeArray(&fdrop.files, sizeof(afxChar const*), 0, NIL, 0);

            afxChar* name = NIL;
            afxUnit len = 0;
            afxUnit i;

            afxDrawSystem dsys;
            AvxGetDrawOutputContext(dout, &dsys);
            AfxAssertType(dctxD, afxFcc_DSYS);
            afxMmu mmu = AfxGetDrawSystemMmu(dsys);
            AFX_ASSERT_OBJECTS(afxFcc_MMU, 1, &mmu);

            for (i = 0; i < cnt; i++)
            {
                len = DragQueryFileA(hDrop, i, NIL, 0);

                if (!(name = AfxAllocate(mmu, len + 1, 1, 0, AfxHere()))) AfxReportError("");
                else
                {
                    DragQueryFileA(hDrop, i, name, len + 1);
                    afxUnit arrIdx;
                    void *arrel = AfxPushArrayUnit(&fdrop.files, &arrIdx);
                    AfxCopy2(1, sizeof(name), name, arrel);
                }
            }

            //afxEvent ev;
            //AfxEventDeploy(&ev, AFX_EVENT_DOUT_DRAGNDROP, &doutD->obj, &fdrop);
            //AfxNotifyObject(&doutD->obj, &ev);

            for (i = 0; i < AfxGetArrayPop(&fdrop.files); i++)
            {
                AfxReportMessage("%s", *(afxChar const**)AfxGetArrayUnit(&fdrop.files, i));
            }

            for (i = 0; i < AfxGetArrayPop(&fdrop.files); i++)
            {
                AfxDeallocate(mmu, *(afxChar**)AfxGetArrayUnit(&fdrop.files, i));
            }

            AfxEmptyArray(&fdrop.files, FALSE, FALSE);

            DragFinish(hDrop);
            //data2->breake = TRUE;
            //return 0;
            break;
        }
#endif
        case WM_KILLFOCUS: // Sent to a window immediately before it loses the keyboard focus.
        {
            if (wnd->m.cursHidden)
            {
                ShowCursor(TRUE);
                0;//AfxEnableCursor(wnd);
            }
            else if (wnd->m.cursConfined)
            {
                AFX_ASSERT(wnd == ses->m.cursCapturedOn);
                afxBool liberated = !!ClipCursor(NULL);
                ses->m.cursCapturedOn = NIL;
            }
            wnd->m.focused = FALSE;
            ses->m.focusedWnd = NIL;

            {
                // GAMBIARRA
                // we need this at focus loss to gain a last chance to release all keys.
                MSG msg = { 0 };
                msg.hwnd = hWnd;
                msg.message = message;
                msg.lParam = lParam;
                msg.wParam = wParam;
                _QowProcessSystemInputMessageWin32(&msg, ses, wnd);
            }
            break;
        }
        case WM_SETFOCUS: // Sent to a window after it has gained the keyboard focus.
        {
            wnd->m.focused = TRUE;
            ses->m.focusedWnd = wnd;

            POINT cursorPos;
            if (GetCursorPos(&cursorPos))
            {
                // Convert the cursor position to the client area of the window
                ScreenToClient(hWnd, &cursorPos);
                // Get the window's client rectangle
                RECT clientRect;
                if (GetClientRect(hWnd, &clientRect))
                {
                    // Check if the cursor is inside the client rectangle
                    if (cursorPos.x >= clientRect.left && cursorPos.x <= clientRect.right &&
                        cursorPos.y >= clientRect.top && cursorPos.y <= clientRect.bottom)
                    {
                        // Cursor is inside the window's client area

                    }
                }
            }
            
#if 0
            if (!AfxIsCursorOnSurface(wnd)) break; // Don't handle frame interaction; just handle cursor in surface.
            else
            {
                if (wnd->m.cursorDisabled)
                {
                    ShowCursor(FALSE);
                    0;//AfxDisableCursor(wnd);
                }
                else if (wnd->m.cursorConfined)
                {
                    RECT cr;
                    GetClientRect(hWnd, &cr);
                    ClientToScreen(hWnd, (POINT*)&cr.left);
                    ClientToScreen(hWnd, (POINT*)&cr.right);
                    afxBool confined = !!ClipCursor(&cr);

                    if (confined)
                        ses->m.curCapturedOn = wnd;
                }
                return 0;
            }
#endif
        }
        case WM_PAINT:
        {
            ValidateRect(hWnd, NULL);
            //SetWindowTextA(wnd->m.wnd, AfxGetStringData(&wnd->m.caption.str, 0));
            AfxRedrawWindow(wnd, NIL);
            return 0; // An application returns zero if it processes this message.
        }
        case WM_ERASEBKGND:
        {
            // Flicker is usually caused by interference via WM_ERASEBKGND. 
            // If you haven't already, intercept WM_ERASEBKGND and do nothing in the regions where you are displaying OpenGL content.            
            ValidateRect(hWnd, NULL);
            AfxRedrawWindow(wnd, NIL);
            return 1; // An application should return nonzero if it erases the background; otherwise, it should return zero.
        }
        default: break;
        }
    }
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

#ifdef AFX_OS_WIN10
_QOW BOOL CALLBACK FindShellWorkerWindowW32(HWND hwnd, LPARAM lParam)
{
    HWND* found = (HWND*)lParam;

    if (FindWindowExA(hwnd, NULL, "SHELLDLL_DefView", NULL))
        *found = FindWindowExA(NULL, hwnd, "WorkerW", NULL);

    return TRUE;
}
#endif

_QOW HWND FindShellBackgroundWindowW32(void)
{
    HWND hwnd = 0;
#ifdef AFX_OS_WIN10
    // Windows 10 Method

    SendMessageTimeoutA(FindWindowA("ProgMan", NULL), 0x052C, 0, 0, SMTO_NORMAL, 1000, NULL);
    hwnd = 0;
    EnumWindows(FindShellWorkerWindowW32, (LPARAM)&(hwnd));
#else
    // Windows 7 Method
    HWND p = FindWindowA("ProgMan", NULL);
    HWND s = FindWindowExA(p, NULL, "SHELLDLL_DefView", NULL);
    hwnd = FindWindowExA(s, NULL, "SysListView32", NULL);
#endif
    return hwnd;
}

_QOW afxError _QowWndChIconCb(afxWindow wnd, avxRaster ras)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    HICON hIcon = NIL;

    if (ras)
    {
        if (!(hIcon = _AuxCreateWin32IconFromRaster(ras, FALSE, 0, 0))) AfxThrowError();
        else
        {
            
            HWND hWnd = wnd->hWnd;
            //Change both icons to the same icon handle.
            SendMessageA(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SendMessageA(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            //This will ensure that the application icon gets changed too.
            SendMessageA(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SendMessageA(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        }
    }

    if (!err)
    {
        if (wnd->hIcon)
            DestroyIcon(wnd->hIcon);

        wnd->hIcon = hIcon;
    }
    return err;
}

_QOW afxBool DoutNotifyOvy(afxWindow wnd, afxUnit bufIdx)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
    wnd->lastBufIdx = bufIdx;
    wnd->swap = 1;

    return 1;
}

_QOW afxError _QowWndRedrawCb(afxWindow wnd, afxRect const* rc)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    HDC dc = wnd->hSurfaceDc;
    
    if (dc)
    {
        if (wnd->swap)
        {
            wnd->swap = FALSE;
            //SwapBuffers(dc);
            //DwmFlush();

            afxUnit rate = 0;
            AvxGetDrawOutputRate(wnd->m.dout, &rate);
            //AfxFormatWindowTitle(wnd, "%u --- Draw I/O System --- Qwadro Execution Ecosystem (c) 2017 SIGMA Technology --- Public Test Build", rate);
            SetWindowTextA(wnd->hWnd, AfxGetStringData(&wnd->m.title.s, 0));
        }
    }
    return err;
}

#if 0
_QOW afxBool AfxTraceScreenToSurface(afxWindow wnd, afxUnit const screenPos[2], afxUnit surfPos[2])
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
    AFX_ASSERT(screenPos);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    POINT point = { .x = screenPos[0],.y = screenPos[1] };
    afxBool rslt = !!ScreenToClient(wnd->hWnd, &point);
    AFX_ASSERT(surfPos);
    surfPos[0] = point.x;
    surfPos[1] = point.y;
    return rslt;
}

_QOW afxBool AfxTraceSurfaceToScreen(afxWindow wnd, afxUnit const surfPos[2], afxUnit screenPos[2])
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
    AFX_ASSERT(surfPos);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    POINT point = { .x = surfPos[0], .y = surfPos[1] };
    afxBool rslt = !!ClientToScreen(wnd->hWnd, &point);
    AFX_ASSERT(screenPos);
    screenPos[0] = point.x;
    screenPos[1] = point.y;
    return rslt;
}
#endif

_QOW afxError _QowWndAdjustCb(afxWindow wnd, afxBool frame, afxRect const* rc)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    afxDesktop* dwm = wnd->m.dwm;

    if (frame)
    {
        afxRect rc2;
        rc2.x = AfxMinu(rc->x, dwm->res.w - 1);
        rc2.y = AfxMinu(rc->y, dwm->res.h - 1);
        rc2.w = AFX_MAX(1, AFX_MIN(rc->w, dwm->res.w));
        rc2.h = AFX_MAX(1, AFX_MIN(rc->h, dwm->res.h));

        if ((wnd->m.frameRect.x != rc2.x) ||
            (wnd->m.frameRect.y != rc2.y) ||
            (wnd->m.frameRect.w != rc2.w) ||
            (wnd->m.frameRect.h != rc2.h))
        {
            afxInt32 extraWndWidth, extraWndHeight;
            CalcWindowValuesW32(wnd->hWnd, &extraWndWidth, &extraWndHeight);

            AFX_ASSERT2(rc2.w, rc2.h);
            wnd->m.frameRect = rc2;

            wnd->m.surfaceRect.x = wnd->m.marginL;
            wnd->m.surfaceRect.y = wnd->m.marginT;
            wnd->m.surfaceRect.w = wnd->m.frameRect.w - wnd->m.marginR - wnd->m.marginL;
            wnd->m.surfaceRect.h = wnd->m.frameRect.h - wnd->m.marginB - wnd->m.marginT;

            if (!SetWindowPos(wnd->hWnd, NULL, wnd->m.frameRect.x, wnd->m.frameRect.y, wnd->m.frameRect.w, wnd->m.frameRect.h, SWP_NOOWNERZORDER | SWP_NOZORDER))
                AfxThrowError();
        }
    }
    else
    {
        //AFX_ASSERT2(wnd->m.frameRect.extent.w > (afxUnit)rc->origin.x, wnd->m.frameRect.extent.h > (afxUnit)rc->origin.y);
        //AFX_ASSERT4(surface->w, wnd->m.frameRect.w > (afxUnit)area->w, surface->h, wnd->m.frameRect.h > (afxUnit)surface->h);

        afxRect rc2;
        rc2.x = AfxMinu(rc->x, wnd->m.frameRect.w - 1);
        rc2.y = AfxMinu(rc->y, wnd->m.frameRect.h - 1);
        rc2.w = AFX_MAX(1, rc->w/*AFX_MIN(surface->w, wnd->m.frameRect.w)*/);
        rc2.h = AFX_MAX(1, rc->h/*AFX_MIN(surface->h, wnd->m.frameRect.h)*/);

        if ((wnd->m.surfaceRect.x != rc2.x) ||
            (wnd->m.surfaceRect.y != rc2.y) ||
            (wnd->m.surfaceRect.w != rc2.w) ||
            (wnd->m.surfaceRect.h != rc2.h))
        {
            afxInt32 extraWndWidth, extraWndHeight;
            CalcWindowValuesW32(wnd->hWnd, &extraWndWidth, &extraWndHeight);

            AFX_ASSERT2(rc2.w, rc2.h);
            wnd->m.frameRect.w = rc2.w + extraWndWidth;
            wnd->m.frameRect.h = rc2.h + extraWndHeight;
            wnd->m.frameRect.x += rc2.x;
            wnd->m.frameRect.y += rc2.y;
            wnd->m.surfaceRect = rc2;

            if (!SetWindowPos(wnd->hWnd, NULL, wnd->m.frameRect.x, wnd->m.frameRect.y, wnd->m.frameRect.w, wnd->m.frameRect.h, SWP_NOOWNERZORDER | SWP_NOZORDER))
                AfxThrowError();
        }
    }

    afxDrawOutput dout = wnd->m.dout;    
    if (dout)
    {
        AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

        avxRange whd;
        AvxQueryDrawOutputExtent(dout, &whd, NIL);
        whd.w = wnd->m.surfaceRect.w;
        whd.h = wnd->m.surfaceRect.h;

        if (AvxAdjustDrawOutput(dout, whd))
            AfxThrowError();
    }
    dout = wnd->m.frameDout;
    if (dout)
    {
        AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

        avxRange whd;
        AvxQueryDrawOutputExtent(dout, &whd, NIL);
        whd.w = wnd->m.frameRect.w;
        whd.h = wnd->m.frameRect.h;

        if (AvxAdjustDrawOutput(dout, whd))
            AfxThrowError();
    }
    return err;
}

_QOW afxBool _QowWndChangeVisibility(afxWindow wnd, afxBool visible)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
    return !!ShowWindow(wnd->hWnd, visible ? SW_SHOW : SW_HIDE);
}

_QOW _auxWndDdi const _QOW_WND_IMPL =
{
    .redrawCb = _QowWndRedrawCb,
    .adjustCb = _QowWndAdjustCb,
    .chIconCb = _QowWndChIconCb,
};

_QOW afxError _QowWndDtorCb(afxWindow wnd)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    AfxDeregisterChainedClasses(&wnd->m.classes);

    _AUX_WND_CLASS_CONFIG.dtor(wnd);

    //AfxDisposeObjects(1, &wnd->m.dout);

    AfxChangeWindowIcon(wnd, NIL); // detach any custom icon
    AFX_ASSERT(!wnd->hIcon);

    DragAcceptFiles(wnd->hWnd, FALSE);
    DestroyWindow(wnd->hWnd);

    return err;
}

_QOW afxError _QowWndCtorCb(afxWindow wnd, void** args, afxUnit invokeNo)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    afxSession ses = args[0];
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);
    afxWindowConfig const* cfg = (afxWindowConfig const*)(args[1]) + invokeNo;
    
    if (!cfg)
    {
        AfxThrowError();
        return err;
    }

    afxClassConfig widClsCfg = _AUX_WID_CLASS_CONFIG;
    widClsCfg.fixedSiz = sizeof(AFX_OBJ(afxWidget));
    widClsCfg.ctor = (void*)_QowWidCtorCb;
    widClsCfg.dtor = (void*)_QowWidDtorCb;

    if (_AUX_WND_CLASS_CONFIG.ctor(wnd, (void*[]) { ses, &cfg, (void*)&widClsCfg }, 0))
    {
        AfxThrowError();
        return err;
    }

    wnd->m.pimpl = &_QOW_WND_IMPL;

    DWORD dwExStyle = WS_EX_APPWINDOW; // Window Extended Style
    DWORD dwStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS; // Window Style

    //dwExStyle |= WS_EX_CONTEXTHELP;
    //dwExStyle |= WS_EX_NOREDIRECTIONBITMAP;

    if (wnd->m.fullscreen)
    {
        dwExStyle |= WS_EX_TOPMOST;
        dwStyle |= WS_POPUP;
        //ShowCursor(FALSE);
    }
    else
    {
        if (wnd->m.floating)
            dwExStyle |= WS_EX_TOPMOST;

        dwStyle |= WS_SYSMENU | WS_MINIMIZEBOX;

        if (!wnd->m.decorated) dwStyle |= WS_POPUP;
        else
        {
            dwStyle |= WS_CAPTION;

            if (wnd->m.resizable)
                dwStyle |= WS_MAXIMIZEBOX | WS_THICKFRAME;
        }
    }

    wnd->dwExStyle = dwExStyle;
    wnd->dwStyle = dwStyle;

    HWND hWnd = CreateWindowExA(dwExStyle, ses->wndClss.lpszClassName, ses->wndClss.lpszClassName, dwStyle, 0, 0, 1, 1, NIL, NIL, ses->wndClss.hInstance, NIL);

    if (!hWnd) AfxThrowError();
    else
    {
        wnd->hIcon = NIL;
        wnd->hFrameDc = NIL;
        wnd->hSurfaceDc = GetDC(hWnd);
        wnd->hWnd = hWnd;
        SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)wnd);

#if 0
        if (!wnd->m.fullscreen)
        {
            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hFileMenu, "File");
            AppendMenu(hFileMenu, MF_STRING, /*ID_FILE_EXIT*/0, "New");
            AppendMenu(hFileMenu, MF_STRING, /*ID_FILE_EXIT*/1, "Open");
            AppendMenu(hFileMenu, MF_STRING, /*ID_FILE_EXIT*/2, "Save");
            AppendMenu(hFileMenu, MF_STRING, /*ID_FILE_EXIT*/3, "Import");
            AppendMenu(hFileMenu, MF_STRING, /*ID_FILE_EXIT*/4, "Export");
            AppendMenu(hFileMenu, MF_STRING, /*ID_FILE_EXIT*/5, "Quit");

            HMENU hEditMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hEditMenu, "Edit");

            HMENU hRenderMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hRenderMenu, "Render");
            AppendMenu(hRenderMenu, MF_STRING, /*ID_FILE_EXIT*/0, "Wireframe");

            HMENU hWindowMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hWindowMenu, "Window");
            AppendMenu(hWindowMenu, MF_STRING, /*ID_FILE_EXIT*/0, "Spawn new");
            AppendMenu(hWindowMenu, MF_STRING, /*ID_FILE_EXIT*/1, "Go fullscreen");
            AppendMenu(hWindowMenu, MF_STRING, /*ID_FILE_EXIT*/1, "Print screenshot");

            // Set the menu to the window
            SetMenu(hWnd, hMenu);
        }
#endif

        DragAcceptFiles(hWnd, TRUE);

        afxDrawOutput dout;
        afxDrawOutputConfig doutCfg = { 0 };
        doutCfg = cfg->surface;
        doutCfg.bufUsage[0] |= avxRasterUsage_DRAW;
        doutCfg.bufUsage[1] |= avxRasterUsage_DRAW;
        doutCfg.bufUsage[2] |= avxRasterUsage_DRAW;
        doutCfg.doNotClip = FALSE;

        doutCfg.endpointNotifyObj = wnd;
        doutCfg.endpointNotifyFn = (void*)DoutNotifyOvy;
        doutCfg.w32.hInst = ses->wndClss.hInstance;
        doutCfg.w32.hWnd = hWnd;

        if (AvxOpenDrawOutput(cfg->dsys, &doutCfg, &dout)) AfxThrowError();
        else
        {
            wnd->m.dout = dout;

            afxRect rc = cfg->rc;
            rc.w = AFX_MAX(1, rc.w);;
            rc.h = AFX_MAX(1, rc.h);
            AfxAdjustWindow(wnd, FALSE, &rc);
#if !0
            if (doutCfg.presentAlpha && (doutCfg.presentAlpha != avxVideoAlpha_OPAQUE))
            {
                DWM_BLURBEHIND bb = { 0 };
                bb.dwFlags = DWM_BB_ENABLE;
                bb.fEnable = TRUE;
                DwmEnableBlurBehindWindow(wnd->hWnd, &(bb)); // ausente no chad Windows XP
            }
#endif

            ShowWindow(hWnd, SHOW_OPENWINDOW);
        }

        if (err)
            DestroyWindow(wnd->hWnd);
    }

    if (err)
        _AUX_WND_CLASS_CONFIG.dtor(wnd);

    return err;
}
