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

#include "qowBase.h"
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

// Returns the difference between window bounds and client area, in pixels
_QOW void GetWindowFrameMargins(HWND hwnd, afxInt32* left, afxInt32* top, afxInt32* right, afxInt32* bottom)
{
    RECT windowRect;
    RECT clientRect;
    POINT clientTopLeft = { 0, 0 };
    POINT clientBottomRight;

    // Get the full window rect (including frame, title bar, etc.)
    GetWindowRect(hwnd, &windowRect);

    // Get the client area size (in client coordinates)
    GetClientRect(hwnd, &clientRect);

    // Convert client area top-left to screen coordinates
    clientBottomRight.x = clientRect.right;
    clientBottomRight.y = clientRect.bottom;

    ClientToScreen(hwnd, &clientTopLeft);
    ClientToScreen(hwnd, &clientBottomRight);

    *left = clientTopLeft.x - windowRect.left;
    *top = clientTopLeft.y - windowRect.top;
    *right = windowRect.right - clientBottomRight.x;
    *bottom = windowRect.bottom - clientBottomRight.y;
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

_QOW afxError _AuxExtractWin32Cursor(HCURSOR cursor, int* width, int* height, unsigned* pixels_abgr, int* hotspot_x, int* hotspot_y)
{
    ICONINFO info;
    GetIconInfo(cursor, &info);
    BOOL bw_cursor = (info.hbmColor == NULL);

    BITMAP bmpinfo;
    memset(&bmpinfo, 0, sizeof(bmpinfo));
    if (bw_cursor && GetObject(info.hbmMask, sizeof(BITMAP), &bmpinfo) == 0)
    {
        DestroyCursor(cursor);
        DeleteObject(info.hbmColor);
        DeleteObject(info.hbmMask);
        return FALSE;
    }
    if (!bw_cursor && GetObject(info.hbmColor, sizeof(BITMAP), &bmpinfo) == 0)
    {
        DestroyCursor(cursor);
        DeleteObject(info.hbmColor);
        DeleteObject(info.hbmMask);
        return FALSE;
    }
    int pointer_width = bmpinfo.bmWidth;
    int pointer_height = (bmpinfo.bmHeight >= 0 ? bmpinfo.bmHeight : -bmpinfo.bmHeight) / (bw_cursor ? 2 : 1);

    if (width) *width = pointer_width;
    if (height) *height = pointer_height;
    if (hotspot_x) *hotspot_x = (int)info.xHotspot;
    if (hotspot_y) *hotspot_y = (int)info.yHotspot;
    if (!pixels_abgr)
    {
        DestroyCursor(cursor);
        DeleteObject(info.hbmColor);
        DeleteObject(info.hbmMask);
        return TRUE;
    }

    BITMAPINFOHEADER bmi;
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = bmpinfo.bmWidth;
    bmi.biHeight = -bmpinfo.bmHeight;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;
    HDC hdc = GetDC(NULL);
    if (GetDIBits(hdc, bw_cursor ? info.hbmMask : info.hbmColor, 0, (UINT)bmpinfo.bmHeight, pixels_abgr,
        (BITMAPINFO*)&bmi, DIB_RGB_COLORS) != bmpinfo.bmHeight)
    {
        DestroyCursor(cursor);
        DeleteObject(info.hbmColor);
        DeleteObject(info.hbmMask);
        ReleaseDC(NULL, hdc);
        return FALSE;
    }
    ReleaseDC(NULL, hdc);
    
    /*
        This code snippet processes an image of a pointer (a mouse cursor) and modifies its pixel data 
        depending on whether the pointer is a black-and-white cursor (bw_cursor is true) or a color cursor (bw_cursor is false). 
        It operates on a 1D array of ABGR pixels named pixels_abgr.
    */

    if (bw_cursor)
    {
        /*
            This case deals with a monochrome (black-and-white) cursor, typically represented using two bitmaps:
                The first pointer_height rows contain the AND mask (transparency).
                The next pointer_height rows contain the XOR mask (color).
            
            This essentially converts the two-mask format into a grayscale ARGB pixel, using:
                Alpha = inverse of AND mask
                RGB = grayscale from XOR mask
        */
        for (int y = 0; y < pointer_height; ++y)
        {
            for (int x = 0; x < pointer_width; ++x)
            {
                unsigned c = pixels_abgr[x + pointer_width * y]; // AND mask
                unsigned m = pixels_abgr[x + pointer_width * (pointer_height + y)]; // XOR mask
                unsigned a = 255 - (c & 0xff); // Create alpha from AND mask (c): 0 = opaque, 255 = transparent
                unsigned g = m & 0xff; // Use XOR mask as grayscale intensity
                pixels_abgr[x + pointer_width * y] = (a << 24) | (g << 16) | (g << 8) | g;
            }
        }
    }
    else
    {
        for (int y = 0; y < pointer_height; ++y)
        {
            for (int x = 0; x < pointer_width; ++x)
            {
                unsigned c = pixels_abgr[x + pointer_width * y];
                unsigned a = (c >> 24) & 0xff;
                unsigned r = (c >> 16) & 0xff;
                unsigned g = (c >> 8) & 0xff;
                unsigned b = (c) & 0xff;
                pixels_abgr[x + pointer_width * y] = (a << 24) | (b << 16) | (g << 8) | r;
            }
        }
    }

    DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
    DestroyCursor(cursor);
    return TRUE;
}

_QOW HICON _AuxCreateWin32IconFromRaster(avxRaster ras, avxRasterRegion const* rgn, afxBool cursor, afxUnit xHotspot, afxUnit yHotspot)
// Creates an RGBA icon or cursor
{
    afxError err = NIL;
    HICON handle = NIL;

    AFX_ASSERT(ras);
    AFX_ASSERT(rgn);

    avxRange whd = rgn->whd;
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
        iop.rgn = *rgn;
        //iop.rgn.whd = whd;
        iop.rowStride = whd.w * (bi.bV5BitCount / 8);
        
        if (AvxDumpRaster(ras, 1, &iop, dst, NIL, 0))
            AfxThrowError();

        afxDrawSystem dsys = AvxGetRasterHost(ras);
        AvxWaitForDrawBridges(dsys, AFX_TIMEOUT_INFINITE, 0);

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

_QOW void ShakeWindow2(afxWindow wnd)
{

    HWND hwnd = wnd->hWnd;
    if (!IsWindow(hwnd)) return;

    RECT rect;
    GetWindowRect(hwnd, &rect);

    int x = rect.left;
    int y = rect.top;

    // Small shake offset
    const int shakeDelta = 10;
    const int shakeCount = 10;
    const int delay = 20; // milliseconds

    // Play a sound with it.
    MessageBeep(MB_ICONEXCLAMATION);
    // Flash the taskbar icon if the window isn't focused.
    FLASHWINFO fi = { sizeof(FLASHWINFO), hwnd, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };
    FlashWindowEx(&fi);
    // Restore window if minimized.
    if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);

    for (int i = 0; i < shakeCount; i++)
    {
        int offset = (i % 2 == 0) ? shakeDelta : -shakeDelta;
        SetWindowPos(hwnd, NULL, x + offset, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        Sleep(delay);
    }

    // Return to original position
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

_QOW void ShakeWindow3(afxWindow wnd)
{

    HWND hwnd = wnd->hWnd;
    if (!IsWindow(hwnd)) return;

    RECT rect;
    GetWindowRect(hwnd, &rect);

    int originalX = rect.left;
    int originalY = rect.top;

    const int shakeDistance = 15;  // Larger offset = more visible
    const int shakeCount = 4;      // Fewer shakes = feels heavier
    const int delay = 50;          // Milliseconds between moves
    
    // Play a sound with it.
    MessageBeep(MB_ICONEXCLAMATION);
    // Flash the taskbar icon if the window isn't focused.
    FLASHWINFO fi = { sizeof(FLASHWINFO), hwnd, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };
    FlashWindowEx(&fi);
    // Restore window if minimized.
    if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);

    for (int i = 0; i < shakeCount; ++i)
    {
        SetWindowPos(hwnd, NULL, originalX + shakeDistance, originalY, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        Sleep(delay);

        SetWindowPos(hwnd, NULL, originalX - shakeDistance, originalY, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        Sleep(delay);
    }

    // Return to original position
    SetWindowPos(hwnd, NULL, originalX, originalY, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

}

_QOW void ShakeWindow(afxWindow wnd)
{

    HWND hwnd = wnd->hWnd;
    if (!IsWindow(hwnd)) return;

    RECT rect;
    GetWindowRect(hwnd, &rect);

    int originalX = rect.left;
    int originalY = rect.top;

    const int shakeDistance = 10;  // How far it moves (in both X and Y)
    const int shakeCount = 8;      // Number of shakes
    const int delay = 50;// 30;          // Delay between shakes (ms)

    // Play a sound with it.
    MessageBeep(MB_ICONEXCLAMATION);
    // Flash the taskbar icon if the window isn't focused.
    FLASHWINFO fi = { sizeof(FLASHWINFO), hwnd, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };
    FlashWindowEx(&fi);
    // Restore window if minimized.
    if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);

    for (int i = 0; i < shakeCount; ++i)
    {
        int offsetX = ((i % 2 == 0) ? 1 : -1) * shakeDistance;
        int offsetY = ((i % 3 == 0) ? 1 : -1) * shakeDistance;

        SetWindowPos(hwnd, NULL,
            originalX + offsetX,
            originalY + offsetY,
            0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        Sleep(delay);
    }

    // Return to original position
    SetWindowPos(hwnd, NULL, originalX, originalY, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

_QOW LRESULT WINAPI _QowWndHndlngPrcW32Callback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    afxError err = AFX_ERR_NONE;
    afxWindow wnd = (afxWindow)(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!wnd)
    {
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
    afxSession ses = AfxGetHost(wnd);
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

    afxByte buf[4096] = { 0 };
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
    case WM_SETCURSOR:
    {
        // When the cursor enters a window's client area, Windows sends WM_SETCURSOR.

        if (LOWORD(lParam) == HTCLIENT)
        {
            auxEvent ev = { 0 };
            ev.id = auxEventId_CURS_IN;
            AfxEmitEvent(wnd, &ev);

            if (wnd->hCursor)
            {
                wnd->hCursorBkp = SetCursor(wnd->hCursor);
                return TRUE;
            }
        }

        break;
    }
    case WM_MOUSEMOVE:
    {
        if (!wnd->trackingMouse)
        {
            TRACKMOUSEEVENT tme = { sizeof(tme) };
            tme.dwFlags = TME_HOVER | TME_LEAVE;
            tme.hwndTrack = hWnd;
            tme.dwHoverTime = HOVER_DEFAULT; // Or a custom time in ms

            TrackMouseEvent(&tme);
            wnd->trackingMouse = TRUE;
        }

        // handle mouse move here.

        POINTS points = MAKEPOINTS(lParam);

        afxV2d curr = { AFX_R(points.x), AFX_R(points.y) };

        AfxV2dSub(wnd->m.cursMove, wnd->m.cursPos, curr);
        AfxV2dCopy(wnd->m.cursPos, curr);

        afxV2d screen = { AFX_R(wnd->m.frameRc.w), AFX_R(wnd->m.frameRc.h) };

        AfxV2dNdc(wnd->m.cursPosNdc, wnd->m.cursPos, screen);
        AfxV2dNdc(wnd->m.cursMoveNdc, wnd->m.cursMove, screen);

        //data2->breake = TRUE;
        return 0;//break;
    }
    case WM_MOUSELEAVE:
    {
        /*
            The mouse left the client area of the window specified in a prior call to TrackMouseEvent.
            All tracking requested by TrackMouseEvent is canceled when this message is generated.
            The application must call TrackMouseEvent when the mouse reenters its window if it requires further tracking of mouse hover behavior.
        */

        auxEvent ev = { 0 };
        ev.id = auxEventId_CURS_OUT;
        AfxEmitEvent(wnd, &ev);

        // Mouse left the window; reset tracking.
        wnd->trackingMouse = FALSE;

        return 0;//break;
    }
    case WM_MOUSEHOVER:
    {
        /*
            The mouse hovered over the client area of the window for the period of time specified in a prior call to TrackMouseEvent.
            Hover tracking stops when this message is generated. The application must call TrackMouseEvent again if it requires further
            tracking of mouse hover behavior.

            WM_MOUSEHOVER is a Windows message that gets sent when the mouse hovers over a window (or control) for a specified amount of time without moving. 
            It's not sent by default; you have to request it using the TrackMouseEvent() API.

            It is used to detect when the user pauses the mouse over a window or control (e.g., for tooltips, hover effects, UI animations, etc.).
            It gives you more precise control than just WM_MOUSEMOVE, which is sent constantly as the mouse moves.
        */

        auxEvent ev = { 0 };
        ev.id = auxEventId_CURS_ON;
        AfxEmitEvent(wnd, &ev);

        // Mouse hovered without moving
        //MessageBoxW(hWnd, L"Mouse hovered!", L"Info", MB_OK);
        wnd->trackingMouse = FALSE; // Reset so it can trigger again

        return 0;//break;
    }
#if 0
    case WM_NCMOUSELEAVE:
    {
        // The same meaning as WM_MOUSELEAVE except this is for the nonclient area of the window.

        auxEvent ev = { 0 };
        ev.id = auxEventId_CURS_OUT;
        AfxEmitEvent(wnd, &ev);

        break;
    }
    case WM_NCMOUSEHOVER:
    {
        // The same meaning as WM_MOUSEHOVER except this is for the nonclient area of the window.
        break;
    }
#endif
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

        auxEvent ev = { 0 };
        ev.id = auxEventId_CLOSE;
        AfxEmitEvent(wnd, &ev);

        if (wnd->m.fullscreen)
        {
            AfxMakeWindowExclusive(wnd, FALSE);
        }

        //PostQuitMessage(0); // Send A Quit Message
        ShowWindow(hWnd, SW_MINIMIZE);
        return 0; // handled. Also prevent process at end of this function
    }
    case WM_ACTIVATE: // Watch For Window Activate Message
    {
        auxEvent ev = { 0 };
        ev.id = auxEventId_ACTIVATE;
        AfxEmitEvent(wnd, &ev);

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
    case WM_MOVING:
    {
        /*
            WM_MOVING (0x0216) is a Windows message sent while the user is dragging the window to move it, before the move is finalized.
            Sent repeatedly during a user-initiated window move operation.
            While the user drags the window frame to a new position.
            Your app to track or modify the window’s position in real-time as it moves.

            wParam: Not used.
            lParam: Pointer to a RECT structure representing the window's proposed new position in screen coordinates
            typedef struct tagRECT {
                LONG left;
                LONG top;
                LONG right;
                LONG bottom;
            } RECT;
            You can modify the RECT in lParam to constrain or snap the window’s position during the move.

            Typical Use Cases:
                Restricting the window from moving outside certain screen bounds,
                Snapping window edges to certain positions or grids,
                Tracking window position continuously while it’s being dragged,
                Implementing custom drag behaviors.

            Example:
            {
                RECT* pRect = (RECT*)lParam;

                // Example: restrict window from moving off the left edge of the screen
                if (pRect->left < 0)
                {
                    int width = pRect->right - pRect->left;
                    pRect->left = 0;
                    pRect->right = width;
                }

                return TRUE; // Indicate message was processed and RECT modified
            }
        */

        break;
    }
    case WM_MOVE:
    {
        /*
            WM_MOVE (0x0003) is a Windows message sent to a window when its position has changed (but not its size).
            Notifies the window that it has been moved to a new position. 
            Sent after the window’s position changes (e.g., user drags the window or a program moves it).

            wParam: Not used, always zero.

            lParam: Contains the new position of the window’s client area (relative to the screen), packed as:
                Low-order word (LOWORD(lParam)) = X-coordinate (left)
                High-order word (HIWORD(lParam)) = Y-coordinate (top

            Typical Use Cases:
                Tracking window position changes for saving/restoring window location,
                Updating UI elements that depend on window position,
                Triggering actions when the window moves (e.g., repositioning popups or related windows)

            Example:
            {
                int xPos = LOWORD(lParam);
                int yPos = HIWORD(lParam);

                // Respond to the new position here, e.g. update UI or save window position

                break;
            }
        */
        break;
    }
    case WM_SIZING:
    {
        /*
            WM_SIZING (0x0214) is a Windows message sent to a window while the user is resizing the window. 
            It lets your application track or modify the resizing in real-time.

            Sent repeatedly while the user is dragging the window border or sizing handles,
            during a user-initiated resize operation (before the window size is finalized).

            wParam: Indicates which edge or corner is being dragged. Possible values:
                WMSZ_LEFT (1)	Left edge
                WMSZ_RIGHT (2)	Right edge
                WMSZ_TOP (3)	Top edge
                WMSZ_TOPLEFT (4)	Top-left corner
                WMSZ_TOPRIGHT (5)	Top-right corner
                WMSZ_BOTTOM (6)	Bottom edge
                WMSZ_BOTTOMLEFT (7)	Bottom-left corner
                WMSZ_BOTTOMRIGHT (8)	Bottom-right corner

            lParam: Pointer to a RECT structure that contains the proposed new window position.
            typedef struct tagRECT {
                LONG left;
                LONG top;
                LONG right;
                LONG bottom;
            } RECT;
            You can modify the RECT in lParam to change or constrain the size or position during the resizing.

            Use Cases:
                Enforcing minimum or maximum window sizes during resizing,
                Snapping window edges to a grid or other windows,
                Implementing custom resizing behavior (e.g., aspect ratio lock),
                Providing visual feedback during resizing.

            WM_SIZING is sent only when the user is interactively resizing the window by dragging its borders or corners. When? 
            During the resize operation, repeatedly as the user drags.
            WM_WINDOWPOSCHANGING is sent whenever the window’s size, position, or Z-order is about to change (not just user resizing, 
            but also programmatic calls like SetWindowPos). When? Before the size/position change takes effect.

            Example:
            {
                RECT* pRect = (RECT*)lParam;

                // Example: enforce minimum width and height of 300x200
                int minWidth = 300;
                int minHeight = 200;

                int width = pRect->right - pRect->left;
                int height = pRect->bottom - pRect->top;

                if (width < minWidth)
                    pRect->right = pRect->left + minWidth;
                if (height < minHeight)
                    pRect->bottom = pRect->top + minHeight;

                return TRUE; // Indicate message was processed
            }
        */
#if 0 // draw while resizing
        afxRect cr = { 0 };
        // lParam já é client area, filha da puta.
        cr.w = LOWORD(lParam);
        cr.h = HIWORD(lParam);

        if (cr.w * cr.h) // don't set to zero
        {
            AfxAdjustWindow(wnd, &cr);
        }
#endif
        break;
    }
    case WM_SIZE:
    {
        /*
            WM_SIZE (0x0005) is a Windows message that is sent after a window has been resized. 
            It lets your application know that the client area dimensions have changed, 
            so you can respond accordingly (e.g., resize child controls, update layouts, redraw content).

            wParam - Resize Type: Indicates the type of resizing that occurred. wParam values:
              SIZE_RESTORED   = 0 - The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies.
              SIZE_MINIMIZED  = 1 - The window has been minimized.
              SIZE_MAXIMIZED  = 2 - The window has been maximized.
              SIZE_MAXSHOW    = 3 - Message is sent to mem pop-up windows when some other window has been restored to its former size.
              SIZE_MAXHIDE    = 4 - Message is sent to mem pop-up windows when some other window is maximized.

            lParam – New Size: Packed with the new width and height of the client area:
            LOWORD(lParam) = new width
            HIWORD(lParam) = new height
            You can extract them in C like this:
            int width  = LOWORD(lParam);
            int height = HIWORD(lParam);

            Sent after the window's size has changed, whether by user action, programmatically via SetWindowPos, 
            or by the system (e.g., when maximizing).

            Unlike WM_WINDOWPOSCHANGED, this focuses specifically on the client area's size, which is what your app typically draws in.

            Common Use Cases:
                Resize or reposition child windows or controls,
                Update layout or graphics (e.g., adjust rendering buffers),
                Recalculate UI elements based on new dimensions

            Example:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);

                switch (wParam)
                {
                    case SIZE_MINIMIZED:
                        // Handle minimize
                        break;
                    case SIZE_MAXIMIZED:
                    case SIZE_RESTORED:
                        // Handle normal or maximized resize
                        ResizeChildControls(hwnd, width, height);
                        break;
                }
                return 0;
            }
        */
#if 0
        //if (doutD->resizable)
        {

            afxRect cr = { 0 };
            // lParam já é client area, filha da puta.
            cr.w = LOWORD(lParam);
            cr.h = HIWORD(lParam);

            if (cr.w * cr.h) // don't set to zero
            {
                AfxAdjustWindow(wnd, &cr);
            }
        }
#endif
        //AfxDrawOutputProcess(dout);
        break;
    }
    case WM_WINDOWPOSCHANGING:
    {
        /*
            WM_WINDOWPOSCHANGING (0x0046) is a Windows message sent before a window's size, position, or Z-order is changed. 
            It gives your application a chance to modify or prevent those changes.

            When your window receives WM_WINDOWPOSCHANGING, the lParam contains a pointer to a modifiable WINDOWPOS structure.

            lParam: Pointer to a WINDOWPOS structure:
            typedef struct tagWINDOWPOS {
              HWND hwnd;
              HWND hwndInsertAfter;
              int  x;       // proposed new X position
              int  y;       // proposed new Y position
              int  cx;      // proposed new width
              int  cy;      // proposed new height
              UINT flags;   // behavior flags (e.g., SWP_NOMOVE, SWP_NOSIZE, etc.)
            } WINDOWPOS;

            wParam: Unused (typically zero)

            Sent when the system or an app is about to move, size, or change the Z-order of a window.
            You can modify the WINDOWPOS structure in lParam to change or block those changes.
            It's called before WM_WINDOWPOSCHANGED, which is sent after the change.

            Related Flags in WINDOWPOS->flags. Some common ones:
                SWP_NOMOVE - Don't change position
                SWP_NOSIZE - Don't change size
                SWP_NOZORDER - Don't change Z-order
                SWP_NOREDRAW - Don't redraw after change,
            You can set these flags to suppress certain behaviors.

            We might use this to:
                Enforce minimum/maximum window size,
                Restrict movement to a certain area,
                Lock Z-order,
                Prevent resizing entirely.

            Example:
            {
                LPWINDOWPOS pos = (LPWINDOWPOS)lParam;

                // Prevent window from moving off-screen
                if (pos->x < 0) pos->x = 0;
                if (pos->y < 0) pos->y = 0;

                // Prevent resizing smaller than 300x200
                if (pos->cx < 300) pos->cx = 300;
                if (pos->cy < 200) pos->cy = 200;

                return 0;
            }
        */



        break; //return 0;
    }
    case WM_WINDOWPOSCHANGED:
    {
        /*
            WM_WINDOWPOSCHANGED (0x0047) is a Windows message used in the Windows API to notify a window that its size, 
            position, or Z-order has changed.

            When a window receives WM_WINDOWPOSCHANGED, the lParam contains a pointer to a WINDOWPOS structure.

            lParam: Pointer to a WINDOWPOS structure:
            typedef struct tagWINDOWPOS {
              HWND hwnd;
              HWND hwndInsertAfter;
              int  x;
              int  y;
              int  cx;
              int  cy;
              UINT flags;
            } WINDOWPOS;
            
            wParam: Not used (typically 0)

            Sent after the window's size, position, or Z-order has changed (as a result of a SetWindowPos or similar).
            This message is sent after WM_WINDOWPOSCHANGING, which is sent before the change happens.
            You typically handle this to adjust layout, invalidate areas, or respond to movement or resizing.

            If you want to prevent changes to size or position, handle WM_WINDOWPOSCHANGING instead.
            WM_WINDOWPOSCHANGED is not meant to veto changes --- it's purely informational

            Example:
            {
                LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
                // You can inspect pos->x, pos->y, pos->cx, pos->cy, etc.
                // Useful to reposition child controls, etc.
                return 0;
            }
        */

        {

            afxRect cr = { 0 };
            GetClientRect(hWnd, &cr);

            if (cr.w * cr.h) // don't set to zero
            {
                AfxAdjustWindow(wnd, &cr);
            }
        }

        auxEvent ev = { 0 };
        ev.id = auxEventId_PLACEMENT;
        ev.wnd = wnd;
        AfxEmitEvent(wnd, &ev);

#if 0
        if (wnd->m.fullscreen)
        {
            AfxMakeWindowExclusive(wnd, FALSE);
            AfxMakeWindowExclusive(wnd, TRUE);
#if 0
            RECT wr, cr;
            GetWindowRect(hwnd, &wr);
            GetClientRect(hwnd, &cr);
            if (wr.right - wr.left == cr.right - cr.left && wr.bottom - wr.top == cr.bottom - cr.top)
            {
                if (cr.right - cr.left != fullscreen_width || cr.bottom - cr.top != fullscreen_height)
                    app_screenmode(app, APP_SCREENMODE_WINDOW);
            }
#endif
        }
#endif

        if (wnd->m.cursConfined)
        {
            afxRect r = wnd->m.cursConfinRect;
            AfxMakeWindowCursory(wnd, &r, FALSE);
            AfxMakeWindowCursory(wnd, &r, TRUE);
#if 0
            ClientToScreen(hwnd, (POINT*)&r);
            ClientToScreen(hwnd, ((POINT*)&r) + 1);
            ClipCursor(&r);
#endif
        }
        break;//return 0;
    } 
    case WM_DISPLAYCHANGE: // The WM_DISPLAYCHANGE message is sent to all windows when the display resolution has changed.
    {
        HDC dc;
        afxSurface dout = wnd->m.dout;
        AvxCallSurfaceEndpoint(dout, 0, &dc);
        avxRange const resolution = { GetDeviceCaps(dc, HORZRES), GetDeviceCaps(dc, VERTRES), GetDeviceCaps(dc, PLANES) };
        afxReal64 physAspRatio = AvxFindPhysicalAspectRatio(GetDeviceCaps(dc, HORZSIZE), GetDeviceCaps(dc, VERTSIZE));
        afxReal refreshRate = GetDeviceCaps(dc, VREFRESH);

        avxModeSetting mode = { 0 };
        AvxQuerySurfaceSettings(dout, &mode);
        mode.refreshRate = refreshRate;
        mode.wpOverHp = physAspRatio;
        mode.resolution = resolution;
        AvxChangeSurfaceSettings(dout, &mode);

        afxDesktop* dwm = wnd->m.dwm;
        dwm->wpOverHp = physAspRatio;
        dwm->refreshRate = refreshRate;
        dwm->res.w = resolution.w;
        dwm->res.h = resolution.h;
        dwm->res.d = resolution.d;
        dwm->wrOverHr = (afxReal64)dwm->res.x / (afxReal64)dwm->res.y;

        auxEvent ev = { 0 };
        ev.id = auxEventId_CHANGED;
        ev.wnd = wnd;
        AfxEmitEvent(wnd, &ev);

        break;
    }
    case WM_STYLECHANGED: // Sent to a window after the SetWindowLong function has changed one or more of the window's styles.
    {
        // Sent to a window after the SetWindowLong function has changed one or more of the window's styles. A window receives this message through its WindowProc function.
        // wParam = Indicates whether the window's styles or extended window styles have changed. This parameter can be one or more of the following values.
        
        afxUnit mleft, mtop, mright, mbottom;
        GetWindowFrameMargins(wnd->hWnd, &mleft, &mtop, &mright, &mbottom);
        wnd->m.marginL = mleft;
        wnd->m.marginT = mtop;
        wnd->m.marginR = mright;
        wnd->m.marginB = mbottom;

        auxEvent ev = { 0 };
        ev.id = auxEventId_STYLE;
        ev.wnd = wnd;
        AfxEmitEvent(wnd, &ev);

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
        AvxGetSurfaceHost(dout, &dsys);
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
            ses->m.cursHidden = FALSE;
        }
        
        if (wnd->m.cursConfined)
        {
            AFX_ASSERT(wnd == ses->m.cursCapturedOn);
            afxBool liberated = !!ClipCursor(NULL);
            ses->m.cursCapturedOn = NIL;
        }
        
        _AfxSesFocusWindowCb(ses, NIL, NIL);

        //ShakeWindow(wnd);

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
        _AfxSesFocusWindowCb(ses, wnd, NIL);

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
                    if (wnd->m.cursHidden)
                    {
                        ShowCursor(FALSE);
                        ses->m.cursHidden = TRUE;
                    }
                    else if (wnd->m.cursConfined)
                    {
                        RECT cr;
                        GetClientRect(hWnd, &cr);
                        ClientToScreen(hWnd, (POINT*)&cr.left);
                        ClientToScreen(hWnd, (POINT*)&cr.right);
                        afxBool confined = !!ClipCursor(&cr);

                        if (confined)
                            ses->m.cursCapturedOn = wnd;
                    }
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
        break;
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
        //ValidateRect(hWnd, NULL);
        //AfxRedrawWindow(wnd, NIL);
        return 1; // An application should return nonzero if it erases the background; otherwise, it should return zero.
    }
    default: break;
    }
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

_QOW afxError _QowWndChIconCb(afxWindow wnd, avxRaster ras, avxRasterRegion const* rgn)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    HICON hIcon = NIL;

    if (ras)
    {
        if (!(hIcon = _AuxCreateWin32IconFromRaster(ras, rgn, FALSE, 0, 0))) AfxThrowError();
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

_QOW afxError _QowWndChCursCb(afxWindow wnd, avxRaster ras, avxRasterRegion const* rgn, afxInt hotspotX, afxInt hotspotY)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    HCURSOR hCursor = NIL;

    if (ras)
    {
        if (!(hCursor = (HCURSOR)_AuxCreateWin32IconFromRaster(ras, rgn, TRUE, hotspotX, hotspotY))) AfxThrowError();
        else
        {
            HWND hWnd = wnd->hWnd;
            //wnd->hCursorBkp = SetCursor(hCursor);
        }
    }

    if (!err)
    {
        if (wnd->hCursor)
            DestroyCursor(wnd->hCursor);

        wnd->hCursor = hCursor;
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

_QOW afxError _QowWndAdjustCb(afxWindow wnd, afxRect const* area)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    afxDesktop* dwm = wnd->m.dwm;

    HWND hWndInsertAfter = wnd->m.alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST;
    UINT swpFlags = 0;// SWP_NOOWNERZORDER | SWP_NOZORDER;

    {
        //AFX_ASSERT2(wnd->m.frameRc.whd.w > (afxUnit)rc->origin.x, wnd->m.frameRc.whd.h > (afxUnit)rc->origin.y);
        //AFX_ASSERT4(surface->w, wnd->m.frameRc.w > (afxUnit)area->w, surface->h, wnd->m.frameRc.h > (afxUnit)surface->h);

        afxRect rc2 = *area;
        rc2.x = AFX_MIN_CASTED(rc2.x, wnd->m.frameRc.w - 1, afxUnit);
        rc2.y = AFX_MIN_CASTED(rc2.y, wnd->m.frameRc.h - 1, afxUnit);
        rc2.w = AFX_MAX(1, rc2.w/*AFX_MIN(surface->w, wnd->m.frameRc.w)*/);
        rc2.h = AFX_MAX(1, rc2.h/*AFX_MIN(surface->h, wnd->m.frameRc.h)*/);

        if ((wnd->m.areaRc.x != rc2.x) ||
            (wnd->m.areaRc.y != rc2.y) ||
            (wnd->m.areaRc.w != rc2.w) ||
            (wnd->m.areaRc.h != rc2.h))
        {
            afxInt32 extraWndWidth, extraWndHeight;
            CalcWindowValuesW32(wnd->hWnd, &extraWndWidth, &extraWndHeight);

            AFX_ASSERT2(rc2.w, rc2.h);
            wnd->m.frameRc.w = rc2.w + extraWndWidth;
            wnd->m.frameRc.h = rc2.h + extraWndHeight;
            wnd->m.frameRc.x += rc2.x;
            wnd->m.frameRc.y += rc2.y;
            wnd->m.areaRc = rc2;

            if (!SetWindowPos(wnd->hWnd, hWndInsertAfter, wnd->m.frameRc.x, wnd->m.frameRc.y, wnd->m.frameRc.w, wnd->m.frameRc.h, swpFlags))
                AfxThrowError();
        }
    }

    afxSurface dout = wnd->m.dout;    
    if (dout)
    {
        AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

        afxRect whd;
        AvxGetSurfaceArea(dout, &whd);
        whd.w = wnd->m.areaRc.w;
        whd.h = wnd->m.areaRc.h;

        if (AvxAdjustSurface(dout, &whd, wnd->m.fullscreen))
            AfxThrowError();
    }
    dout = wnd->m.frameDout;
    if (dout)
    {
        AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

        afxRect whd;
        AvxGetSurfaceArea(dout, &whd);
        whd.w = wnd->m.frameRc.w;
        whd.h = wnd->m.frameRc.h;

        if (AvxAdjustSurface(dout, &whd, wnd->m.fullscreen))
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

_QOW afxUnit _QowWndFormatTitleCb(afxWindow wnd)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));
    SetWindowTextA(wnd->hWnd, wnd->m.title.buf);
    return 0;
}

_QOW _auxWndDdi const _QOW_WND_IMPL =
{
    .redrawCb = _QowWndRedrawCb,
    .adjustCb = _QowWndAdjustCb,
    .chIconCb = _QowWndChIconCb,
    .titleCb = _QowWndFormatTitleCb,
};

_QOW afxError _QowWndDtorCb(afxWindow wnd)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wnd));

    AfxDeregisterChainedClasses(&wnd->m.classes);

    _AUX_WND_CLASS_CONFIG.dtor(wnd);

    //AfxDisposeObjects(1, &wnd->m.dout);

    AfxChangeWindowCursor(wnd, NIL, NIL, 0, 0); // detach any custom cursor
    AfxChangeWindowIcon(wnd, NIL, NIL); // detach any custom icon
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
    afxWindowConfig const* wcfg = (afxWindowConfig const*)(args[1]) + invokeNo;
    
    if (!wcfg)
    {
        AfxThrowError();
        return err;
    }

    afxClassConfig widClsCfg = _AUX_WID_CLASS_CONFIG;
    widClsCfg.fixedSiz = sizeof(AFX_OBJ(afxWidget));
    widClsCfg.ctor = (void*)_QowWidCtorCb;
    widClsCfg.dtor = (void*)_QowWidDtorCb;

    if (_AUX_WND_CLASS_CONFIG.ctor(wnd, (void*[]) { ses, (void*)wcfg, (void*)&widClsCfg }, 0))
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

    HWND hWnd = CreateWindowExA(dwExStyle, ses->wndClss.lpszClassName, ses->wndClss.lpszClassName, 
                                dwStyle, wcfg->atX, wcfg->atY, 1, 1, NIL, NIL, ses->wndClss.hInstance, NIL);

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

        afxSurface dout;
        afxSurfaceConfig scfg = { 0 };
        scfg = wcfg->dout;
        scfg.doNotClip = FALSE;

        scfg.iop.endpointNotifyObj = wnd;
        scfg.iop.endpointNotifyFn = (void*)DoutNotifyOvy;
        scfg.iop.w32.hInst = ses->wndClss.hInstance;
        scfg.iop.w32.hWnd = hWnd;

        if (AvxOpenSurface(wcfg->dsys, &scfg, &dout)) AfxThrowError();
        else
        {
            wnd->m.dout = dout;

            afxRect rc = { .x = wcfg->atX, .y = wcfg->atY, .w = wcfg->dout.ccfg.whd.w, .h = wcfg->dout.ccfg.whd.h };
            rc.w = AFX_MAX(1, rc.w);;
            rc.h = AFX_MAX(1, rc.h);
            AfxAdjustWindow(wnd, &rc);
#if !0
            if (scfg.presentAlpha && (scfg.presentAlpha != avxVideoAlpha_OPAQUE))
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
