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

#define _AFX_SYSTEM_C
#define _AUX_UX_C
#define _AUX_SHELL_C
#define _AUX_SHELL_IMPL
#define _AUX_WINDOW_C
#define _AUX_WINDOW_IMPL
//#include "qwadro/../_luna/luna_vm.h"
//#include "qwadro/../_luna/luna.h"
#include "AuxOverWin32.h"

_QOW afxTime _QowSshPump(afxShell ssh)
{
    afxError err = AFX_ERR_NONE;

    afxTime timeout = 0;

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
            AfxAssertObjects(1, &wnd, afxFcc_WND);
            ses = AfxGetProvider(wnd);
            AfxAssertObjects(1, &ses, afxFcc_SES);
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

                if (GetKeyState(VK_F1))
                {
                    AfxLogAssistence("User input requested support.");
                    AfxLogAssistence("Get help at https://sigmaco.org/");
                    system("start https://sigmaco.org");
                }
                else if (GetKeyState(VK_F4))
                {
                    //AfxRequestThreadInterruption(thr);
                    AfxLogAdvertence("User input requested application break.");
                    AfxRequestShutdown(0);
                }
                else if (GetKeyState(VK_F5))
                {
                    AfxLogAdvertence("User input requested application reboot.");
                    //_AfxInterruptionAllApplications();
                    //opcode = AFX_OPCODE_REBOOT;
                }
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

_QOW afxError afxIcdHook(afxModule icd, afxUri const* manifest)
{
    afxError err = AFX_ERR_NONE;
    AfxAssertObjects(1, &icd, afxFcc_MDLE);

    afxClassConfig widClsCfg = _AuxWidStdImplementation;
    widClsCfg.fixedSiz = sizeof(AFX_OBJ(afxWidget));
    widClsCfg.ctor = (void*)_QowWidCtorCb;
    widClsCfg.dtor = (void*)_QowWidDtorCb;

    afxClassConfig wndClsCfg = _AuxWndStdImplementation;
    wndClsCfg.fixedSiz = sizeof(AFX_OBJ(afxWindow));
    wndClsCfg.ctor = (void*)_QowWndCtorCb;
    wndClsCfg.dtor = (void*)_QowWndDtorCb;

    afxClassConfig sesClsCfg = _AuxSesStdImplementation;
    sesClsCfg.fixedSiz = sizeof(AFX_OBJ(afxSession));
    sesClsCfg.ctor = (void*)_QowSesCtorCb;
    sesClsCfg.dtor = (void*)_QowSesDtorCb;

    afxShellInfo shInfos[] =
    {
        {
            .dev.urn = AfxString("dwm"),
            .dev.type = afxDeviceType_SHELL,

            //.dev.procCb = (void*)_ZclDdevProcCb,

            .widClsCfg = &widClsCfg,
            .wndClsCfg = &wndClsCfg,
            .sesClsCfg = &sesClsCfg,
        }
    };

    afxShell shells[ARRAY_SIZE(shInfos)];

    if (_AuxRegisterShells(icd, ARRAY_SIZE(shInfos), shInfos, shells)) AfxThrowError();
    else
    {
        shells[0]->pumpCb = _QowSshPump;

        afxHidInfo hidInfos[] =
        {
            {
                .dev.urn = AfxString("kbd"),
                .dev.type = afxDeviceType_HID,
                .type = afxHidType_KEYBOARD
            },
            {
                .dev.urn = AfxString("mse"),
                .dev.type = afxDeviceType_HID,
                .type = afxHidType_MOUSE
            },
            {
                .dev.urn = AfxString("gpad"),
                .dev.type = afxDeviceType_HID,
                .type = afxHidType_GAMEPAD
            }
        };
        afxHid hidevices[ARRAY_SIZE(hidInfos)];

        if (_AuxRegisterHids(icd, ARRAY_SIZE(hidInfos), hidInfos, hidevices)) AfxThrowError();
        else
        {
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
    }

    return err;
}
