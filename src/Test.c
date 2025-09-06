/*
 *           ::::::::    :::::::::::    ::::::::    ::::     ::::       :::
 *          :+:    :+:       :+:       :+:    :+:   +:+:+: :+:+:+     :+: :+:
 *          +:+              +:+       +:+          +:+ +:+:+ +:+    +:+   +:+
 *          +#++:++#++       +#+       :#:          +#+  +:+  +#+   +#++:++#++:
 *                 +#+       +#+       +#+   +#+#   +#+       +#+   +#+     +#+
 *          #+#    #+#       #+#       #+#    #+#   #+#       #+#   #+#     #+#
 *           ########    ###########    ########    ###       ###   ###     ###
 *
 *                     S I G M A   T E C H N O L O G Y   G R O U P
 *
 *                                 Public Test Build
 *                             (c) 2017 SIGMA FEDERATION
 *                      Open sourced under the Qwadro License.
 */

#include "qwadro/afxQwadro.h"


int main(int argc, char const* argv[])
{
    afxError err = AFX_ERR_NONE;

    // Boot up the Qwadro (if necessary)

    afxSystemConfig sysc = { 0 };
    AfxConfigureSystem(&sysc);
    AfxBootstrapSystem(&sysc);

    // Set up the draw system

    afxUnit drawIcd = 0;
    afxDrawSystem dsys;
    afxDrawSystemConfig dsyc = { 0 };
    AvxConfigureDrawSystem(drawIcd, &dsyc);
    dsyc.caps = afxDrawFn_DRAW | afxDrawFn_PRESENT;
    dsyc.accel = afxAcceleration_DPU;
    dsyc.exuCnt = 1;
    AvxEstablishDrawSystem(drawIcd, &dsyc, &dsys);
    AFX_ASSERT_OBJECTS(afxFcc_DSYS, 1, &dsys);

    // Set up the mix system

    afxUnit mixIcd = 0;
    afxMixSystem msys;
    afxMixSystemConfig msyc = { 0 };
    msyc.dsys = dsys; // integrate our draw system
    msyc.exuCnt = 1;
    AfxConfigureMixSystem(mixIcd, &msyc);
    AmxEstablishMixSystem(mixIcd, &msyc, &msys);
    AFX_ASSERT_OBJECTS(afxFcc_MSYS, 1, &msys);

    // Open a session

    afxUnit shIcd = 0;
    afxSession ses;
    afxSessionConfig scfg = { 0 };
    scfg.dsys = dsys; // integrate our draw system
    scfg.msys = msys; // integrate our mix system
    AfxAcquireSession(shIcd, &scfg, &ses);
    AFX_ASSERT_OBJECTS(afxFcc_SES, 1, &ses);
    AfxOpenSession(ses, NIL, NIL, NIL);

    // Acquire a drawable surface

    afxWindow wnd;
    afxWindowConfig wrc = { 0 };
    wrc.dsys = dsys;
    AfxConfigureWindow(&wrc, NIL, AFX_V3D(0.5, 0.5, 1));
    AfxAcquireWindow(&wrc, &wnd);
    //AfxAdjustWindowFromNdc(window, NIL, AFX_V3D(0.5, 0.5, 1));
    
    afxSurface dout;
    AfxGetWindowSurface(wnd, &dout);
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    // Run

    afxClock startClock, lastClock;
    AfxGetClock(&startClock);
    lastClock = startClock;
    afxBool readyToRender = TRUE;

    while (1)
    {
        AfxPollInput(0, AFX_TIMEOUT_INFINITE);

        if (!AfxSystemIsExecuting())
            break;

        afxClock currClock;
        AfxGetClock(&currClock);
        afxReal64 ct = AfxGetSecondsElapsed(&startClock, &currClock);
        afxReal64 dt = AfxGetSecondsElapsed(&lastClock, &currClock);
        lastClock = currClock;

        if (!readyToRender)
            continue;

    }

    AfxDisposeObjects(1, &wnd);
    AfxDisposeObjects(1, &msys);
    AfxDisposeObjects(1, &dsys);

    AfxDoSystemShutdown(0);
    AfxYield();
    return 0;
}
