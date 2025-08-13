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

QOW LRESULT WINAPI _QowWndHndlngPrcW32Callback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
extern afxKey const Scan1MakeToQwadroDereferenceMap[afxKey_TOTAL];

_QOW afxTime _QowSesPump(afxSession ses, afxFlags flags, afxTime timeout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    //afxTime timeout = 0;

    afxTime first, last, dt;
    AfxGetTime(&first);

    SHORT escSt = 0, lastEscSt = 0;

    MSG msg;
    afxResult msgCnt = 0;
    while (PeekMessageA(&msg, NIL, 0, 0, PM_REMOVE | PM_NOYIELD))
    {
        ++msgCnt;

        afxWindow wnd = (afxWindow)(GetWindowLongPtr(msg.hwnd, GWLP_USERDATA));
        afxSession ses = NIL;

        if (wnd)
        {
            AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
            ses = AfxGetProvider(wnd);
            AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);
        }

        if (msg.message == WM_QUIT) // PostQuitMessage()
        {
            if (AfxIsPrimeThread())
                AfxRequestShutdown(0);
        }
        else if (msg.message == WM_USER)
        {
            void(*event)(void* udd) = (void*)msg.wParam;

            if (event)
                event((void*)msg.lParam);
        }
        else
        {
            if (AfxIsPrimeThread())
            {

                lastEscSt = escSt;
                escSt = (GetKeyState(VK_ESCAPE) & 0x8000);

#if 0
                if (GetKeyState(VK_F1))
                {
                    AfxReportHint("User input requested support.");
                    AfxReportHint("Get help at https://sigmaco.org/");
                    system("start https://sigmaco.org");
                }
                else if (GetKeyState(VK_F4))
                {
                    //AfxRequestThreadInterruption(thr);
                    AfxReportWarn("User input requested application break.");
                    AfxRequestShutdown(0);
                }
                else if (GetKeyState(VK_F5))
                {
                    AfxReportWarn("User input requested application reboot.");
                    //_AfxInterruptionAllApplications();
                    //opcode = AFX_OPCODE_REBOOT;
                }
#endif

#if 0
                else if (lastEscSt && !(escSt))
                {
                    {
                        afxString cur;
                        AfxMakeString(&cur, 0, "\n$ ", 0);
                        afxString2048 buf;
                        AfxMakeString2048(&buf, NIL);
                        AfxPrompt(&cur, &buf.str);
                        AfxPrint(0, buf.buf);
                        AfxPrint(0, "\n");
                    }
                }
#endif
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    dt = (AfxGetTime(&last) - first);
    return dt;
}

#if 0

// Confines the cursor to a rectangular area on the screen. 
// If a subsequent cursor position (set by the SetCursorPos function or the mouse) lies outside the rectangle, 
// the system automatically adjusts the position to keep the cursor inside the rectangular area.

_QOW afxBool AfxConfineCursor(afxWindow wnd)
{
    afxError err = AFX_ERR_NONE;
    afxSystem sys;
    AfxGetSystem(&sys);
    AFX_ASSERT_OBJECTS(afxFcc_SYS, 1, &sys);
    afxBool rslt = FALSE;

    if (!wnd)
    {
        rslt = !!ClipCursor(NULL);
    }
    else
    {
        AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
        HWND hWnd = wnd->hWnd;

        RECT cr;
        GetClientRect(hWnd, &cr);
        ClientToScreen(hWnd, (POINT*)&cr.left);
        ClientToScreen(hWnd, (POINT*)&cr.right);
        rslt = !!ClipCursor(&cr);
    }
    TheShell.m.curCapturedOn = wnd;
    return rslt;
}

_QOW afxBool AfxLiberateCursor(afxWindow wnd)
{
    afxError err = AFX_ERR_NONE;
    afxSystem sys;
    AfxGetSystem(&sys);
    AFX_ASSERT_OBJECTS(afxFcc_SYS, 1, &sys);
    afxBool rslt = FALSE;

    if (!wnd || (wnd == TheShell.m.curCapturedOn))
        rslt = AfxConfineCursor(NIL);

    return rslt;
}

_QOW afxBool AfxIsCursorOnSurface(afxWindow wnd)
{
    afxError err = AFX_ERR_NONE;
    afxSystem sys;
    AfxGetSystem(&sys);
    AFX_ASSERT_OBJECTS(afxFcc_SYS, 1, &sys);
    AFX_ASSERT_OBJECTS(afxFcc_WND, 1, &wnd);
    HWND hWnd = wnd->hWnd;
    afxBool rslt = FALSE;
    POINT pos;

    if (GetCursorPos(&pos))
    {
        if (hWnd == WindowFromPoint(pos))
        {
            RECT cr;
            GetClientRect(hWnd, &cr);
            ClientToScreen(hWnd, (POINT*)&cr.left);
            ClientToScreen(hWnd, (POINT*)&cr.right);
            rslt = PtInRect(&cr, pos);
        }
    }
    return rslt;
}
#endif

_QOW afxError CopyIntoClipboard(afxSession ses, afxString const* text)
{
    afxError err = NIL;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    if (!OpenClipboard(NIL))
    {
        AfxThrowError();
        return err;
    }

    if (!EmptyClipboard())
    {
        AfxThrowError();
        CloseClipboard();
        return err;
    }

    // Create a Win32-compatible string
    size_t stringSize = (text->len + 1) * sizeof(WCHAR);
    HANDLE stringHandle = GlobalAlloc(GMEM_MOVEABLE, stringSize);

    if (stringHandle)
    {
        LPVOID p = GlobalLock(stringHandle);
        // Perform the conversion from UTF-8 to UTF-16
        MultiByteToWideChar(CP_UTF8, 0, text->start, text->len, p, stringSize);
        GlobalUnlock(stringHandle);
        SetClipboardData(CF_UNICODETEXT, stringHandle);
    }
    CloseClipboard();
}

_QOW afxBool HasClipboardContent(afxSession ses)
{
    afxError err = NIL;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        AfxThrowError();
        return err;
    }
}

_QOW afxError CopyOutClipboard(afxSession ses, afxString* buf)
{
    afxError err = NIL;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        AfxThrowError();
        return err;
    }

    if (!OpenClipboard(NIL))
    {
        AfxThrowError();
        return err;
    }

    HANDLE clipboardHandle = GetClipboardData(CF_UNICODETEXT);

    if (!clipboardHandle)
    {
        AfxThrowError();
        CloseClipboard();
        return err;
    }

    LPVOID p = GlobalLock(clipboardHandle);
    size_t stringSize = wcslen(p);
    WideCharToMultiByte(CP_UTF8, 0, p, stringSize, buf->start, buf->cap, 0, 0);
    GlobalUnlock(clipboardHandle);

    CloseClipboard();
    return err;
}

_QOW void setMousePosition(afxSession ses, afxWindow wnd, afxInt const position[2])
{
    HWND hWnd = wnd ? wnd->hWnd : 0;

    if (hWnd) // relative?
    {
        POINT point = { position[0], position[1] };
        ClientToScreen(hWnd, &point);
        SetCursorPos(point.x, point.y);
    }
    else
    {
        SetCursorPos(position[0], position[1]);
    }
}

_QOW afxBool _QowGetFocusedWindow(afxSession ses, afxWindow* window)
{
    afxError err = 0;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);
    afxWindow wnd = ses->m.focusedWnd;
    HWND hWndFocused = GetForegroundWindow();

    if (wnd)
    {
        AFX_ASSERT(wnd->hWnd == hWndFocused);
        *window = wnd;
    }
    else
    {
        AFX_ASSERT(NIL == hWndFocused);
        *window = NIL;
    }
    return !!wnd;
}

_QOW afxError _QowPromoteWindow(afxSession ses, afxWindow wnd)
{
    afxError err = 0;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    // Win32: We can only promote windows in same process.

    DWORD thisPid = 0;
    DWORD foregroundPid = 0;
    GetWindowThreadProcessId(wnd->hWnd, &thisPid);
    GetWindowThreadProcessId(GetForegroundWindow(), &foregroundPid);

    if (thisPid == foregroundPid) SetForegroundWindow(wnd->hWnd);
    else
    {
        // If our process is not focused, we just flash the window as we cant steal focus for it.
        FLASHWINFO info = { 0 };
        info.cbSize = sizeof(info);
        info.hwnd = wnd->hWnd;
        info.dwFlags = FLASHW_TRAY;
        info.dwTimeout = 0;
        info.uCount = 3;

        FlashWindowEx(&info);
    }
}

_QOW afxBool getMousePosition(afxSession ses, afxRect* rc, afxWindow wnd, afxRect* onFrame, afxRect* onSurface)
{
    // Get the cursor width and height using GetSystemMetrics
    // The nominal width and height of a cursor, in pixels.
    int w = GetSystemMetrics(SM_CXCURSOR);
    int h = GetSystemMetrics(SM_CYCURSOR);
    POINT cursorPos;
    BOOL rslt = GetCursorPos(&cursorPos);
    ses->m.cursRect = AVX_RECT(cursorPos.x, cursorPos.y, w, h);

    if (rc)
        *rc = ses->m.cursRect;

    if (wnd)
    {
        if (onFrame)
        {
            // Get the window's rectangle
            RECT wr;
            GetWindowRect(wnd->hWnd, &wr);

            // Check if the cursor is inside the rectangle
            if (cursorPos.x >= wr.left && cursorPos.x <= wr.right &&
                cursorPos.y >= wr.top && cursorPos.y <= wr.bottom)
            {
                // Cursor is inside the window's area
                *onFrame = AVX_RECT(cursorPos.x - wr.left, cursorPos.y - wr.top, w, h);
            }
            else rslt = FALSE;
        }

        if (onSurface)
        {
            // Convert the cursor position to the client area of the window
            rslt = ScreenToClient(wnd->hWnd, &cursorPos);

            // Get the window's client rectangle
            RECT cr;
            GetClientRect(wnd->hWnd, &cr);

            // Check if the cursor is inside the client rectangle
            if (cursorPos.x >= cr.left && cursorPos.x <= cr.right &&
                cursorPos.y >= cr.top && cursorPos.y <= cr.bottom)
            {
                // Cursor is inside the window's client area
                *onSurface = AVX_RECT(cursorPos.x, cursorPos.y, w, h);
            }
            else rslt = FALSE;
        }
    }
    return rslt;
}

void setTracking(afxWindow wnd, afxBool track)
{
    TRACKMOUSEEVENT mouseEvent = { 0 };
    mouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
    mouseEvent.dwFlags = track ? TME_LEAVE : TME_CANCEL;
    mouseEvent.hwndTrack = wnd->hWnd;
    mouseEvent.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&mouseEvent);
}

afxError grabCursor(afxSession ses, afxWindow wnd, afxBool grabbed)
{
    afxError err = NIL;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    if (grabbed)
    {
        RECT rect;
        GetClientRect(wnd->hWnd, &rect);
        MapWindowPoints(wnd->hWnd, NIL, (LPPOINT)&rect, 2);
        ClipCursor(&rect);
        ses->m.cursCapturedOn = wnd;
    }
    else
    {
        ClipCursor(NIL);
        ses->m.cursCapturedOn = NIL;
    }
    return err;
}

_QOW afxError immergeWindow(afxSession ses, afxWindow wnd, afxBool fullscreen)
{
    afxError err = NIL;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    if (wnd)
    {
        HWND hWnd = wnd->hWnd;
        RECT bkpRc;

        if (fullscreen)
        {
            //
            // Save the window position.
            //
            if (!GetWindowRect(hWnd, &bkpRc))
            {
                AfxThrowError();
            }

            if (!SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS))
            {
                AfxThrowError();
            }

            if (!SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST))
            {
                AfxThrowError();
            }

#if 0
            DEVMODE devMode = { 0 };
            devMode.dmSize = sizeof(devMode);
            devMode.dmPelsWidth = resW;
            devMode.dmPelsHeight = resY;
            devMode.dmBitsPerPel = resBpp;
            devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

            if (ChangeDisplaySettingsW(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            {
                AfxThrowError();
            }
#endif

            if (!SetWindowPos(hWnd,
                HWND_TOPMOST,
                0,
                0,
                GetSystemMetrics(SM_CXSCREEN),
                GetSystemMetrics(SM_CYSCREEN),
                SWP_FRAMECHANGED))
            {
                AfxThrowError();
                return err;
            }
            wnd->bkpRc = bkpRc;
            wnd->m.fullscreen = TRUE;
        }
        else
        {
            bkpRc = wnd->bkpRc;
            wnd->m.fullscreen = FALSE;

            if (!SetWindowLongPtr(hWnd, GWL_STYLE, wnd->dwStyle | WS_VISIBLE))
            {
                AfxThrowError();
                return err;
            }

            if (!SetWindowLongPtr(hWnd, GWL_EXSTYLE, wnd->dwExStyle))
            {
                AfxThrowError();
                return err;
            }

            // Restore the window position.

            if (!SetWindowPos(hWnd,
                HWND_NOTOPMOST,
                bkpRc.left,
                bkpRc.top,
                bkpRc.right - bkpRc.left,
                bkpRc.bottom - bkpRc.top,
                SWP_FRAMECHANGED))
            {
                AfxThrowError();
                return err;
            }
        }
    }
    return err;
}

_QOW _auxSesImpl const _AUX_SES_IMPL =
{
    .pumpCb = (void*)_QowSesPump,
    .getClipboardCb = (void*)CopyOutClipboard,
    .setClipboardCb = (void*)CopyIntoClipboard,
    .hasClipboardCb = (void*)HasClipboardContent,
    .getCurs = (void*)getMousePosition,
    .fseCb = (void*)immergeWindow,
    .promote = (void*)_QowPromoteWindow,
    .grabCursorCb = (void*)grabCursor
};

_QOW afxError _QowSesDtorCb(afxSession ses)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(ses));

    AfxDeregisterChainedClasses(&ses->m.classes);

    _AUX_SES_CLASS_CONFIG.dtor(ses);

    UnregisterClassA(ses->wndClss.lpszClassName, ses->wndClss.hInstance);

    {
        // remove HIDs

        static RAWINPUTDEVICE const rid[] =
        {
            {
                .usUsagePage = HID_USAGE_PAGE_GENERIC, // ((USAGE)0x01)
                .usUsage = HID_USAGE_GENERIC_KEYBOARD, // ((USAGE)0x06)
                .dwFlags = RIDEV_REMOVE, // ignores legacy keyboard messages
                .hwndTarget = NIL
            },
            {
                .usUsagePage = HID_USAGE_PAGE_GENERIC, // ((USAGE)0x01)
                .usUsage = HID_USAGE_GENERIC_MOUSE, // ((USAGE) 0x02)
                .dwFlags = RIDEV_REMOVE,
                .hwndTarget = NIL
            },
            {
                .usUsagePage = HID_USAGE_PAGE_GENERIC, // ((USAGE)0x01)
                .usUsage = HID_USAGE_GENERIC_GAMEPAD, // ((USAGE) 0x05)
                .dwFlags = RIDEV_REMOVE,
                .hwndTarget = NIL
            }
        };

        if (!(RegisterRawInputDevices(rid, ARRAY_SIZE(rid), sizeof(rid[0]))))
            AfxThrowError();

    }

    return err;
}

_QOW afxError _QowSesCtorCb(afxSession ses, void** args, afxUnit invokeNo)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);

    afxModule icd = args[0];
    AFX_ASSERT_OBJECTS(afxFcc_MDLE, 1, &icd);
    _auxSessionAcquisition const* cfg = AFX_CAST(_auxSessionAcquisition const*, args[1]) + invokeNo;
    AFX_ASSERT(cfg);
    
    _auxSessionAcquisition cfg2 = { 0 };
    cfg2 = *cfg;

    afxClassConfig wndClsCfg = _AUX_WND_CLASS_CONFIG;
    wndClsCfg.fixedSiz = sizeof(AFX_OBJ(afxWindow));
    wndClsCfg.ctor = (void*)_QowWndCtorCb;
    wndClsCfg.dtor = (void*)_QowWndDtorCb;

    cfg2.wndClsCfg = &wndClsCfg;

    if (_AUX_SES_CLASS_CONFIG.ctor(ses, (void*[]) { icd, &cfg2 }, 0))
    {
        AfxThrowError();
        return err;
    }

    ses->wndClss.cbSize = sizeof(ses->wndClss); // only on EX
    ses->wndClss.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    ses->wndClss.lpfnWndProc = _QowWndHndlngPrcW32Callback;
    ses->wndClss.cbClsExtra = sizeof(afxObject);
    ses->wndClss.cbWndExtra = sizeof(afxObject);
    ses->wndClss.hInstance = GetModuleHandleA(NULL);
    ses->wndClss.hIcon = LoadIconA(NULL, IDI_SHIELD);
    ses->wndClss.hCursor = LoadCursorA(NULL, IDC_ARROW);
    ses->wndClss.hbrBackground = NULL;
    ses->wndClss.lpszMenuName = NULL;
    ses->wndClss.lpszClassName = "Multimedia UX Infrastructure --- Qwadro Execution Ecosystem (c) 2017 SIGMA --- Public Test Build";
    ses->wndClss.hIconSm = LoadIconA(NULL, IDI_SHIELD);

    if (!(RegisterClassExA(&ses->wndClss))) AfxThrowError();
    else
    {
        afxUnit dwmCnt = 1;

        for (afxUnit i = 0; i < dwmCnt; i++)
        {
            afxDesktop* dwm = &ses->m.dwm;

            dwm->res.w = GetSystemMetrics(SM_CXSCREEN);
            dwm->res.h = GetSystemMetrics(SM_CYSCREEN);
            dwm->res.d = 1;
            dwm->dout = NIL;
            dwm->refreshRate = 1;
        }

        ses->m.pimpl = &_AUX_SES_IMPL;

        {
            // attach HIDs

            static afxBool hasRidBeenRegistered = FALSE;

            if (hasRidBeenRegistered == FALSE)
            {
                static RAWINPUTDEVICE const rid[] =
                {
                    {
                        .usUsagePage = HID_USAGE_PAGE_GENERIC, // ((USAGE)0x01)
                        .usUsage = HID_USAGE_GENERIC_KEYBOARD, // ((USAGE)0x06)
                        .dwFlags = /*RIDEV_NOLEGACY | RIDEV_NOHOTKEYS |*/ RIDEV_DEVNOTIFY/* | RIDEV_APPKEYS*/, // ignores legacy keyboard messages
                        .hwndTarget = NIL
                    },
                    {
                        .usUsagePage = HID_USAGE_PAGE_GENERIC, // ((USAGE)0x01)
                        .usUsage = HID_USAGE_GENERIC_MOUSE, // ((USAGE) 0x02)
                        .dwFlags = /*RIDEV_NOLEGACY |*/ RIDEV_DEVNOTIFY, // ignores legacy mouse messages
                        .hwndTarget = NIL
                    },
                    {
                        .usUsagePage = HID_USAGE_PAGE_GENERIC, // ((USAGE)0x01)
                        .usUsage = HID_USAGE_GENERIC_GAMEPAD, // ((USAGE) 0x05)
                        .dwFlags = RIDEV_DEVNOTIFY,
                        .hwndTarget = NIL
                    }
                };

                if (!(hasRidBeenRegistered = RegisterRawInputDevices(rid, ARRAY_SIZE(rid), sizeof(rid[0]))))
                    AfxThrowError();
            }
        }

        if (err)
            UnregisterClassA(ses->wndClss.lpszClassName, ses->wndClss.hInstance);
    }

    if (err)
    {
        AfxDeregisterChainedClasses(&ses->m.classes);

        _AUX_SES_CLASS_CONFIG.dtor(ses);
    }
    return err;
}
