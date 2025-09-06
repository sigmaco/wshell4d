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
#include "qowBase.h"
#include "qowVideo_W32.h"
#include "qowAudio_W32.h"
#include "../../afx/src/draw/ddi/avxImpl_Surface.h"
//#include "../../afx/src/mix/ddi/avxImpl_Sink.h"

QOW afxError _QowDoutDtorCb_GDI(afxSurface dout);
QOW afxError _QowDoutCtorCb_GDI(afxSurface dout, void** args, afxUnit invokeNo);

QOW afxError _ZglDoutCtorCb(afxSurface dout, void** args, afxUnit invokeNo);
QOW afxError _ZglDoutDtorCb(afxSurface dout);

afxError getInteropDoutCls(afxDrawSystem dsys, afxString const* tool, afxClassConfig* cfg)
{
    afxError err = AFX_ERR_NONE;

    if (0 == AfxCompareString(tool, 0, "wgl", 0, FALSE))
    {
        afxClassConfig doutClsCfg = _AVX_DOUT_CLASS_CONFIG;
        doutClsCfg.fixedSiz = sizeof(AFX_OBJ(afxSurface));
        doutClsCfg.ctor = (void*)_ZglDoutCtorCb;
        doutClsCfg.dtor = (void*)_ZglDoutDtorCb;
        *cfg = doutClsCfg;
    }
    else if (   (0 == AfxCompareString(tool, 0, "w32", 0, FALSE)) || 
                (0 == AfxCompareString(tool, 0, "gdi", 0, FALSE)) ||
                (0 == AfxCompareString(tool, 0, "soft", 0, FALSE)) ||
                (0 == AfxCompareString(tool, 0, "sw", 0, FALSE)) ||
                (0 == AfxCompareString(tool, 0, "", 0, FALSE)))
    {
        afxClassConfig doutClsCfg = _AVX_DOUT_CLASS_CONFIG;
        doutClsCfg.fixedSiz = sizeof(AFX_OBJ(afxSurface));
        doutClsCfg.ctor = (void*)_QowDoutCtorCb_GDI;
        doutClsCfg.dtor = (void*)_QowDoutDtorCb_GDI;
        *cfg = doutClsCfg;
    }
    else
    {
        *cfg = (afxClassConfig) { 0 };
        err = afxError_UNSUPPORTED;
    }

    return err;
}

afxError getInteropSinkCls(afxMixSystem msys, afxString const* tool, afxClassConfig* cfg)
{
    afxError err = AFX_ERR_NONE;

    if ((0 == AfxCompareString(tool, 0, "w32", 0, FALSE)) ||
        (0 == AfxCompareString(tool, 0, "wasapi", 0, FALSE)) ||
        (0 == AfxCompareString(tool, 0, "soft", 0, FALSE)) ||
        (0 == AfxCompareString(tool, 0, "sw", 0, FALSE)) ||
        (0 == AfxCompareString(tool, 0, "", 0, FALSE)))
    {
        afxClassConfig sinkClsCfg = _AMX_ASIO_CLASS_CONFIG;
        sinkClsCfg.fixedSiz = sizeof(AFX_OBJ(afxSink));
        sinkClsCfg.ctor = (void*)_QowAsioCtorCb;
        sinkClsCfg.dtor = (void*)_QowAsioDtorCb;
        *cfg = sinkClsCfg;
    }
    else
    {
        *cfg = (afxClassConfig) { 0 };
        err = afxError_UNSUPPORTED;
    }
    return err;
}

_QOW afxError afxIcdHook(afxModule icd, afxUri const* manifest)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_MDLE, 1, &icd);

    afxClassConfig sesClsCfg = _AUX_SES_CLASS_CONFIG;
    sesClsCfg.fixedSiz = sizeof(AFX_OBJ(afxSession));
    sesClsCfg.ctor = (void*)_QowSesCtorCb;
    sesClsCfg.dtor = (void*)_QowSesDtorCb;

    afxClassConfig vduClsCfg = _AVX_VDU_CLASS_CONFIG;
    vduClsCfg.fixedSiz = sizeof(AFX_OBJ(afxDisplay));
    vduClsCfg.ctor = (void*)_ZglVduCtorCb;
    vduClsCfg.dtor = (void*)_ZglVduDtorCb;

    _afxShellImplementation impl = { 0 };
    impl.vduCls = vduClsCfg;
    impl.sesCls = sesClsCfg;
    impl.getInteropDoutCls = getInteropDoutCls;
    impl.getInteropSinkCls = getInteropSinkCls;

    _AuxImplementShell(icd, &impl);

    RegisterPresentVdus(icd);

    return err;
}
