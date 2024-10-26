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

_QOW afxError _QowWidCtorCb(afxWidget wid, void** args, afxUnit invokeNo)
{
    afxError err = AFX_ERR_NONE;
    AfxAssertObjects(1, &wid, afxFcc_WID);

    afxSession ses = args[0];
    AfxAssertObjects(1, &ses, afxFcc_SES);
    afxWidgetConfig const* cfg = args[1];
    
    if (_AuxWidStdImplementation.ctor(wid, args, invokeNo)) AfxThrowError();
    else
    {
        
    }
    return err;
}

_QOW afxError _QowWidDtorCb(afxWidget wid)
{
    afxError err = AFX_ERR_NONE;
    AfxAssertObjects(1, &wid, afxFcc_WID);

    AFX_ASSERT(AfxGetTid() == AfxGetObjectTid(wid));

    _AuxWidStdImplementation.dtor(wid);

    return err;
}
