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

#if 0

// Confines the cursor to a rectangular area on the screen. 
// If a subsequent cursor position (set by the SetCursorPos function or the mouse) lies outside the rectangle, 
// the system automatically adjusts the position to keep the cursor inside the rectangular area.

_QOW afxBool AfxConfineCursor(afxWindow wnd)
{
    afxError err = AFX_ERR_NONE;
    afxSystem sys;
    AfxGetSystem(&sys);
    AfxAssertObjects(1, &sys, afxFcc_SYS);
    afxBool rslt = FALSE;

    if (!wnd)
    {
        rslt = !!ClipCursor(NULL);
    }
    else
    {
        AfxAssertObjects(1, &wnd, afxFcc_WND);
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
    AfxAssertObjects(1, &sys, afxFcc_SYS);
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
    AfxAssertObjects(1, &sys, afxFcc_SYS);
    AfxAssertObjects(1, &wnd, afxFcc_WND);
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

_QOW afxError _QowSesCtorCb(afxSession ses, void** args, afxUnit invokeNo)
{
    afxError err = AFX_ERR_NONE;
    AfxAssertObjects(1, &ses, afxFcc_SES);

    afxShell ssh = args[0];
    AfxAssertObjects(1, &ssh, afxFcc_SSH);
    afxSessionConfig const* cfg = args[1];
    
    if (_AuxSesStdImplementation.ctor(ses, args, invokeNo)) AfxThrowError();
    else
    {
        afxClassConfig clsCfg;

        clsCfg = ssh->widClsCfg;
        AfxRegisterClass(&ses->m.widCls, NIL, &ses->m.classes, &clsCfg);

        clsCfg = ssh->wndClsCfg;
        AfxRegisterClass(&ses->m.wndCls, NIL, &ses->m.classes, &clsCfg);
        
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
        ses->wndClss.lpszClassName = "Multimedia User Experience Infrastructure --- Qwadro Execution Ecosystem (c) 2017 SIGMA --- Public Test Build";
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

            if (err)
                UnregisterClassA(ses->wndClss.lpszClassName, ses->wndClss.hInstance);
        }

        if (err)
        {
            AfxDeregisterChainedClasses(&ses->m.classes);

            _AuxSesStdImplementation.dtor(ses);
        }
    }
    return err;
}

_QOW afxError _QowSesDtorCb(afxSession ses)
{
    afxError err = AFX_ERR_NONE;
    AfxAssertObjects(1, &ses, afxFcc_SES);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(ses));

    AfxDeregisterChainedClasses(&ses->m.classes);

    UnregisterClassA(ses->wndClss.lpszClassName, ses->wndClss.hInstance);

    _AuxSesStdImplementation.dtor(ses);

    return err;
}
