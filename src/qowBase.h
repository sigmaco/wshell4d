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

#ifndef QOW_DEFS_H
#define QOW_DEFS_H

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

#define _AFX_CORE_C
#define _AFX_DEVICE_C
#define _AFX_CONTEXT_C

#define _AUX_UX_C
#define _AUX_SHELL_C
#define _AUX_SESSION_C
#define _AUX_SESSION_IMPL
#define _AUX_WINDOW_C
#define _AUX_WINDOW_IMPL
#define _AUX_WIDGET_C
#define _AUX_WIDGET_IMPL

#define _AUX_HID_C
#define _AUX_HID_IMPL

#include "afx/src/ux/impl/auxImplementation.h"

#ifndef AUX_DRV_SRC
#   ifdef _DEBUG
#       define QOW DLLIMPORT extern 
#       define QOWINL DLLIMPORT EMBED
#   else
#       define QOW DLLIMPORT extern 
#       define QOWINL DLLIMPORT EMBED
#   endif
#else
#   ifdef _DEBUG
#       define _QOW DLLEXPORT
#       define QOW DLLEXPORT extern 
#       define _QOWINL DLLEXPORT INLINE
#       define QOWINL DLLEXPORT EMBED
#   else
#       define _QOW DLLEXPORT
#       define QOW DLLEXPORT extern 
#       define _QOWINL DLLEXPORT INLINE
#       define QOWINL DLLEXPORT EMBED
#   endif
#endif

//#define QOW_PLUGIN_ID AFX_MAKE_FCC('q', 'o', 'w', '\0')

#ifdef _AUX_SESSION_C
AFX_OBJECT(afxSession)
{
    AFX_OBJ(_auxSession)    m;
    WNDCLASSEX              wndClss;
};
#endif

#ifdef _AUX_WINDOW_C
AFX_OBJECT(afxWindow)
{
    AFX_OBJ(_auxWindow) m;
    HWND                hWnd;
    HDC                 hFrameDc;
    HDC                 hSurfaceDc;
    DWORD               dwStyle;
    DWORD               dwExStyle;
    RECT                bkpRc;
    HICON               hIcon;
    HICON               hCursor;
    HICON               hCursorBkp; // cached only
    afxAtomic           lastBufIdx;
    afxBool             swap;
    afxBool             trackingMouse;
};
#endif

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT

#define NK_INCLUDE_FONT_BAKING
#include "../../guis/nuklear/demo/qwadro/nuklear_qwadro.h"

#ifdef _AUX_WIDGET_C
AFX_OBJECT(afxWidget)
{
    AFX_OBJ(_auxWidget) m;
    struct nk_afx idd;
    struct nk_context *ctx;
};
#endif

QOW afxKey const _win32VkToQwadro[256];
QOW afxResult _QowProcessSystemInputMessageWin32(MSG* msg, afxSession ses, afxWindow wnd);

QOW afxError _QowSesCtorCb(afxSession ses, void** args, afxUnit invokeNo);
QOW afxError _QowSesDtorCb(afxSession ses);

QOW afxError _QowWndCtorCb(afxWindow wnd, void** args, afxUnit invokeNo);
QOW afxError _QowWndDtorCb(afxWindow wnd);

QOW afxError _QowWidCtorCb(afxWidget wid, void** args, afxUnit invokeNo);
QOW afxError _QowWidDtorCb(afxWidget wid);

QOW afxError _QowSesCtorCb(afxSession ses, void** args, afxUnit invokeNo);
QOW afxError _QowSesDtorCb(afxSession ses);

QOW afxBool _QowPlaceFseSurfaceW32(HWND hwnd);

#endif//QOW_DEFS_H
