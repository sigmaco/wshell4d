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

#pragma comment(lib,"dwmapi.lib")

#include "../../icd_tarzgl4/src/zglUtils.h"
#include "../../icd_tarzgl4/src/zglCommands.h"
#include "../../icd_tarzgl4/src/zglObjects.h"
#include "qowBase.h"
#include "qowVideo_W32.h"

_QOW afxUnit _ZglDoutIsSuspended(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);
    AfxLockFutex(&dout->m.suspendSlock, TRUE);
    afxUnit suspendCnt = dout->m.suspendCnt;
    AfxUnlockFutex(&dout->m.suspendSlock, TRUE);
    return suspendCnt;
}

_QOW afxUnit _ZglDoutSuspendFunction(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);
    AfxLockFutex(&dout->m.suspendSlock, FALSE);
    afxUnit suspendCnt = ++dout->m.suspendCnt;
    AfxUnlockFutex(&dout->m.suspendSlock, FALSE);
    return suspendCnt;
}

_QOW afxUnit _ZglDoutResumeFunction(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);
    AfxLockFutex(&dout->m.suspendSlock, FALSE);
    afxUnit suspendCnt = --dout->m.suspendCnt;
    AfxUnlockFutex(&dout->m.suspendSlock, FALSE);
    return suspendCnt;
}

_QOW afxError _ZglDoutPresent_WGL(afxDrawQueue dque, avxPresentation* ctrl)
{
    afxError err = AFX_ERR_NONE;

    afxSurface dout = ctrl->dout;
    afxUnit bufIdx = ctrl->bufIdx;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    //AFX_ASSERT(dout->m.presentingBufIdx == bufIdx); // must be called by _AvxDoutPresent() to handle sync.

    HDC bkpHdc = wglGetCurrentDCWIN();
    HGLRC bkpGlrc = wglGetCurrentContextWIN();
    afxBool mustRestoreCtx = FALSE;

    if (bkpHdc != dout->hDC)
    {
        if (!wglMakeCurrentWIN(dout->hDC, dout->wgl.hSwapRC))
        {
            AfxThrowError();
            return err;
        }
        mustRestoreCtx = TRUE;
    }

    glVmt const* gl = dout->wgl.gl;

    afxRect rc;
    avxRaster buf;
    avxCanvas canv;
    AvxGetSurfaceBuffer(dout, bufIdx, &buf);
    AvxGetSurfaceCanvas(dout, bufIdx, &canv, &rc);
    AFX_ASSERT_OBJECTS(afxFcc_CANV, 1, &canv);
    AFX_ASSERT_OBJECTS(afxFcc_RAS, 1, &buf);

    if (buf->glHandle)
    {
        if (!dout->wgl.swaps[bufIdx].swapFboReady)
        {
            // deferred regen because we need a context.
            if (!dout->wgl.swaps[bufIdx].swapFbo)
            {
                gl->GenFramebuffers(1, &dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
                gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
                AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
            }
            else
            {
                AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
                gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
            }
            _ZglBindFboAttachment(gl, GL_READ_FRAMEBUFFER, NIL, GL_COLOR_ATTACHMENT0, buf->glTarget, buf->glHandle, 0, 0, FALSE);
            dout->wgl.swaps[bufIdx].swapFboReady = TRUE;
        }
        else
        {
            AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
            gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
        }

        static GLenum const invBufs[] =
        {
            GL_COLOR,
            GL_DEPTH,
            GL_STENCIL
        };
        static GLbitfield const clearBitmask = GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT*/;

        //gl->InvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, ARRAY_SIZE(invBufs), invBufs); _ZglThrowErrorOccuried();
        gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); _ZglThrowErrorOccuried();
        //gl->InvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, ARRAY_SIZE(invBufs), invBufs); _ZglThrowErrorOccuried();
        gl->Clear(clearBitmask);  _ZglThrowErrorOccuried();
        //gl->ClearBufferfv(GL_COLOR, /*GL_DRAW_BUFFER0 +*/ 0, AFX_V4D_ZERO); _ZglThrowErrorOccuried();

        afxInt x = (dout->m.presentTransform & avxVideoTransform_MIRROR) ? rc.w : 0;
        afxInt y = (dout->m.presentTransform & avxVideoTransform_FLIP) ? rc.h : 0;
        afxInt w = (dout->m.presentTransform & avxVideoTransform_MIRROR) ? 0 : rc.w;
        afxInt h = (dout->m.presentTransform & avxVideoTransform_FLIP) ? 0 : rc.h;

        gl->BlitFramebuffer(0, 0, rc.w, rc.h, x, y, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST); _ZglThrowErrorOccuried();
        //gl->BindFramebuffer(GL_READ_FRAMEBUFFER, 0); _ZglThrowErrorOccuried(); _ZglThrowErrorOccuried();
        //gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); _ZglThrowErrorOccuried(); _ZglThrowErrorOccuried();
        //gl->Flush();
#if !0
        if (dout->wgl.swapOnWgl)
        {
            wglSwapBuffersWIN(dout->hDC);
        }
        else
#endif
        {
            SwapBuffers(dout->hDC);
        }

        if (dout->m.presentAlpha && (dout->m.presentAlpha != avxVideoAlpha_OPAQUE))
        {
            if (!dout->m.fse)
                DwmFlush();
        }
    }

    if (mustRestoreCtx)
    {
        wglMakeCurrentWIN(bkpHdc, bkpGlrc);
    }

    //AfxYield();
    AfxSleep(1);

    return err;
}

#if !0
_QOW afxError _DpuPresentDout_BlitSwapFbo(zglDpu* dpu, avxPresentation* ctrl)
{
    afxError err = AFX_ERR_NONE;

    afxSurface dout = ctrl->dout;
    afxUnit bufIdx = ctrl->bufIdx;

    if (afxError_TIMEOUT == AvxWaitForFences(AfxGetHost(ctrl->dout), AFX_TIMEOUT_NONE, FALSE, 1, &ctrl->waitOnDpu, NIL))
    {
        return afxError_TIMEOUT;
    }

    //AFX_ASSERT(dout->m.presentingBufIdx == (afxAtom32)AFX_INVALID_INDEX);
    //dout->m.presentingBufIdx = bufIdx;

    HDC bkpHdc = wglGetCurrentDCWIN();
    HGLRC bkpGlrc = wglGetCurrentContextWIN();
    afxBool mustRestoreCtx = FALSE;

    if (bkpHdc != dout->hDC)
    {
        if (!wglMakeCurrentWIN(dout->hDC, dout->wgl.hSwapRC))
        {
            AfxThrowError();
            return err;
        }
        mustRestoreCtx = TRUE;
    }

    glVmt const* gl = dpu->gl;

    afxRect rc;
    avxRaster buf;
    avxCanvas canv;
    AvxGetSurfaceBuffer(dout, bufIdx, &buf);
    AvxGetSurfaceCanvas(dout, bufIdx, &canv, &rc);
    AFX_ASSERT_OBJECTS(afxFcc_CANV, 1, &canv);
    AFX_ASSERT_OBJECTS(afxFcc_RAS, 1, &buf);

    if (buf->glHandle)
    {
        if (!dout->wgl.swaps[bufIdx].swapFboReady)
        {
            // deferred regen because we need a context.
            if (!dout->wgl.swaps[bufIdx].swapFbo)
            {
                gl->GenFramebuffers(1, &dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
                gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
                AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
            }
            else
            {
                AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
                gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
            }
            _ZglBindFboAttachment(gl, GL_READ_FRAMEBUFFER, NIL, GL_COLOR_ATTACHMENT0, buf->glTarget, buf->glHandle, 0, 0, FALSE);
            dout->wgl.swaps[bufIdx].swapFboReady = TRUE;
        }
        else
        {
            AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
            gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
        }

        static GLenum const invBufs[] =
        {
            GL_COLOR,
            GL_DEPTH,
            GL_STENCIL
        };
        static GLbitfield const clearBitmask = GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT*/;

        //gl->InvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, ARRAY_SIZE(invBufs), invBufs); _ZglThrowErrorOccuried();
        gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); _ZglThrowErrorOccuried();
        //gl->InvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, ARRAY_SIZE(invBufs), invBufs); _ZglThrowErrorOccuried();
        gl->Clear(clearBitmask);  _ZglThrowErrorOccuried();
        //gl->ClearBufferfv(GL_COLOR, /*GL_DRAW_BUFFER0 +*/ 0, AFX_V4D_ZERO); _ZglThrowErrorOccuried();

        afxInt x = (dout->m.presentTransform & avxVideoTransform_MIRROR) ? rc.w : 0;
        afxInt y = (dout->m.presentTransform & avxVideoTransform_FLIP) ? rc.h : 0;
        afxInt w = (dout->m.presentTransform & avxVideoTransform_MIRROR) ? 0 : rc.w;
        afxInt h = (dout->m.presentTransform & avxVideoTransform_FLIP) ? 0 : rc.h;

        gl->BlitFramebuffer(0, 0, rc.w, rc.h, x, y, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST); _ZglThrowErrorOccuried();
        //gl->BindFramebuffer(GL_READ_FRAMEBUFFER, 0); _ZglThrowErrorOccuried();
        //gl->Flush(); _ZglThrowErrorOccuried(); // flush is presenting/swapping

#if !0
        if (dout->wgl.swapOnWgl)
        {
            wglSwapBuffersWIN(dout->hDC);
        }
        else
#endif
        {
            SwapBuffers(dout->hDC);
        }

        if (dout->m.presentAlpha && (dout->m.presentAlpha != avxVideoAlpha_OPAQUE))
        {
            if (!dout->m.fse)
                DwmFlush();
        }
    }

    if (mustRestoreCtx)
    {
        wglMakeCurrentWIN(bkpHdc, bkpGlrc);
    }

    dout->m.lastPresentedBufIdx = bufIdx;

    afxClock currClock;
    AfxGetClock(&currClock);
    ++dout->m.outNo;

    if ((1.0 <= AfxGetSecondsElapsed(&dout->m.outCntResetClock, &currClock)))
    {
        dout->m.outCntResetClock = currClock;
        dout->m.outRate = dout->m.outNo; // 681 no showing (presenting from overlay thread (acquirer)), 818 frozen (present from draw thread (worker))
        dout->m.outNo = 0;

        afxReal64 ct = AfxGetSecondsElapsed(&dout->m.startClock, &currClock);
        afxReal64 dt = AfxGetSecondsElapsed(&dout->m.lastClock, &currClock);
        dout->m.lastClock = currClock;

        if (AfxTestObjectFcc(dout->m.endpointNotifyObj, afxFcc_WND))
        {
            //AfxFormatWindowTitle(dout->endpointNotifyObj, "%0f, %u --- Qwadro Video Graphics Infrastructure --- Qwadro Execution Ecosystem (c) 2017 SIGMA --- Public Test Build", dt, dout->m.outRate);
        }

        if (dout->m.endpointNotifyFn)
        {
            dout->m.endpointNotifyFn(dout->m.endpointNotifyObj, ctrl->bufIdx);
        }
    }

    //dout->m.presentingBufIdx = (afxAtom32)AFX_INVALID_INDEX;
    --dout->m.buffers[ctrl->bufIdx].locked;
    AfxPushInterlockedQueue(&dout->m.freeBuffers, (afxUnit[]) { ctrl->bufIdx });

    //AfxYield();
    AfxSleep(1);

    return err;
}

_QOW afxError _DpuPresentDout_BlitRas(zglDpu* dpu, afxSurface dout, afxUnit bufIdx)
{
    afxError err = AFX_ERR_NONE;

    //if (_ZglActivateDout(dpu, dout))
        //AfxThrowError();

    //AFX_ASSERT(dout->m.presentingBufIdx == (afxAtom32)AFX_INVALID_INDEX);
    //dout->m.presentingBufIdx = bufIdx;

    HDC bkpHdc = wglGetCurrentDCWIN();
    HGLRC bkpGlrc = wglGetCurrentContextWIN();
    afxBool mustRestoreCtx = FALSE;

    if (bkpHdc != dout->hDC)
    {
        if (!wglMakeCurrentWIN(dout->hDC, dout->wgl.hSwapRC))
        {
            AfxThrowError();
            return err;
        }
        mustRestoreCtx = TRUE;
    }

    glVmt const* gl = dpu->gl;

    afxRect rc;
    avxRaster buf;
    avxCanvas canv;
    AvxGetSurfaceBuffer(dout, bufIdx, &buf);
    AvxGetSurfaceCanvas(dout, bufIdx, &canv, &rc);
    AFX_ASSERT_OBJECTS(afxFcc_CANV, 1, &canv);
    AFX_ASSERT_OBJECTS(afxFcc_RAS, 1, &buf);

    if (buf->glHandle)
    {
#if 0
        afxDrawSystem dsys = dpu->activeDsys;
        AFX_ASSERT_OBJECTS(afxFcc_DSYS, 1, &dsys);

        afxWarp extent;
        AvxGetRasterExtent(surf, 0, extent);

        _zglCmdBeginSynthesis cmdBeginOp = { 0 };
        cmdBeginOp.defFbo = TRUE;
        cmdBeginOp.canv = NIL;
        afxRect area = { 0 };
        area.w = extent[0];
        area.h = extent[1];
        cmdBeginOp.rasterCnt = 1;
        cmdBeginOp.rasters[0] = (avxDrawTarget const) { .loadOp = avxLoadOp_CLEAR, .storeOp = avxStoreOp_STORE, .clearValue = { .color = { 0.3, 0.1, 0.3, 1 } } };
        DpuCommenceDrawScope(dpu, NIL, &area, 1, 1, cmdBeginOp.rasters, NIL, NIL, TRUE);
        _DpuBindRasterizer(dpu, dsys->presentRazr, NIL, NIL);
        avxViewport vp = { 0 };
        vp.extent[0] = area.w;
        vp.extent[1] = area.h;
        vp.depth[1] = 1.f;
        DpuSetViewports(dpu, 0, 1, &vp);
        DpuBindRasters(dpu, 0, 0, 1, &surf, (afxUnit[]) { 0 });
        DpuBindSamplers(dpu, 0, 0, 1, &dsys->presentSmp);
        avxDrawIndirect cmdDraw = { 0 };
        cmdDraw.vtxCnt = 4;
        cmdDraw.instCnt = 1;
        DpuDraw(dpu, &cmdDraw);
        DpuConcludeDrawScope(dpu);
#else
        if (!dout->wgl.swaps[bufIdx].swapFboReady)
        {
            // deferred regen because we need a context.
            if (!dout->wgl.swaps[bufIdx].swapFbo)
            {
                gl->GenFramebuffers(1, &dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
                gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
                AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
            }
            else
            {
                AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
                gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
            }
            _ZglBindFboAttachment(gl, GL_READ_FRAMEBUFFER, NIL, GL_COLOR_ATTACHMENT0, buf->glTarget, buf->glHandle, 0, 0, FALSE);
            dout->wgl.swaps[bufIdx].swapFboReady = TRUE;
        }
        else
        {
            AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[bufIdx].swapFbo));
            gl->BindFramebuffer(GL_READ_FRAMEBUFFER, dout->wgl.swaps[bufIdx].swapFbo); _ZglThrowErrorOccuried();
        }

        static GLenum const invBufs[] =
        {
            GL_COLOR,
            GL_DEPTH,
            GL_STENCIL
        };
        static GLbitfield const clearBitmask = GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT*/;

        //gl->InvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, ARRAY_SIZE(invBufs), invBufs); _ZglThrowErrorOccuried();
        gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); _ZglThrowErrorOccuried();
        //gl->InvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, ARRAY_SIZE(invBufs), invBufs); _ZglThrowErrorOccuried();
        gl->Clear(clearBitmask);  _ZglThrowErrorOccuried();
        //gl->ClearBufferfv(GL_COLOR, /*GL_DRAW_BUFFER0 +*/ 0, AFX_V4D_ZERO); _ZglThrowErrorOccuried();

        afxInt x = (dout->m.presentTransform & avxVideoTransform_MIRROR) ? rc.w : 0;
        afxInt y = (dout->m.presentTransform & avxVideoTransform_FLIP) ? rc.h : 0;
        afxInt w = (dout->m.presentTransform & avxVideoTransform_MIRROR) ? 0 : rc.w;
        afxInt h = (dout->m.presentTransform & avxVideoTransform_FLIP) ? 0 : rc.h;

        gl->BlitFramebuffer(0, 0, rc.w, rc.h, x, y, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST); _ZglThrowErrorOccuried();
        //gl->BindFramebuffer(GL_READ_FRAMEBUFFER, 0); _ZglThrowErrorOccuried();
        //gl->Flush(); _ZglThrowErrorOccuried(); // flush is presenting/swapping
#endif

#if !0
        if (dout->wgl.swapOnWgl)
        {
            wglSwapBuffersWIN(dout->hDC);
        }
        else
#endif
        {
            SwapBuffers(dout->hDC);
        }

        //AfxYield();
        AfxSleep(1);

        if (dout->m.presentAlpha && (dout->m.presentAlpha != avxVideoAlpha_OPAQUE))
        {
            if (!dout->m.fse)
                DwmFlush();
        }
    }

    if (mustRestoreCtx)
    {
        wglMakeCurrentWIN(bkpHdc, bkpGlrc);
    }
    return err;
}

#endif

#if 0
_QOW afxError _DpuPresentDout(zglDpu* dpu, afxSurface dout, afxUnit outBufIdx)
{
    afxError err = AFX_ERR_NONE;

    afxDrawSystem dsys = dpu->activeDsys;
    AFX_ASSERT_OBJECTS(afxFcc_DSYS, 1, &dsys);

    AFX_ASSERT(dout->m.presentingBufIdx == (afxAtom32)AFX_INVALID_INDEX);

    //dout->presentingBufIdx = outBufIdx;

    _DpuPresentDout_BlitCanvToGdi(dpu, dout, outBufIdx);
    //_DpuPresentDout_BlitSurfToGdi(dpu, dout, outBufIdx);

    afxClock currClock;
    AfxGetClock(&currClock);
    ++dout->m.outNo;

    if ((1.0 <= AfxGetSecondsElapsed(&dout->m.outCntResetClock, &currClock)))
    {
        dout->m.outCntResetClock = currClock;
        dout->m.outRate = dout->m.outNo; // 681 no showing (presenting from overlay thread (acquirer)), 818 frozen (present from draw thread (worker))
        dout->m.outNo = 0;

        afxReal64 ct = AfxGetSecondsElapsed(&dout->m.startClock, &currClock);
        afxReal64 dt = AfxGetSecondsElapsed(&dout->m.lastClock, &currClock);
        dout->m.lastClock = currClock;

        if (AfxTestObjectFcc(dout->m.endpointNotifyObj, afxFcc_WND))
        {
            AfxFormatWindowTitle(dout->m.endpointNotifyObj, "%0f, %u --- Qwadro Video Graphics Infrastructure --- Qwadro Execution Ecosystem (c) 2017 SIGMA --- Public Test Build", dt, dout->m.outRate);
        }

        if (dout->m.endpointNotifyFn)
        {
            dout->m.endpointNotifyFn(dout->m.endpointNotifyObj, outBufIdx);
        }
    }

    dout->m.presentingBufIdx = (afxAtom32)AFX_INVALID_INDEX;
    AfxPushInterlockedQueue(&dout->m.freeBuffers, (afxUnit[]){ outBufIdx });
    AfxDecAtom32(&dout->m.submCnt);
    
    return err;
}
#endif

_QOW afxError _ZglRelinkDoutCb_WGL(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);
    
    afxDrawSystem dsys = AvxGetSurfaceHost(dout);
    AFX_ASSERT_OBJECTS(afxFcc_DSYS, 1, &dsys);
    
    afxDrawBridge dexu;
    AvxGetDrawBridges(dsys, 0, 1, &dexu);
    afxDrawDevice ddev = AvxGetBridgedDrawDevice(dexu, NIL);
    AFX_ASSERT_OBJECTS(afxFcc_DDEV, 1, &ddev);

    glVmt const* gl = dout->wgl.gl;

    if (dout->wgl.hSwapRC)
    {
        afxDrawSystem currDsys = AvxGetSurfaceHost(dout);

        HDC bkpHdc = wglGetCurrentDCWIN();
        HGLRC bkpGlrc = wglGetCurrentContextWIN();

        if (!wglMakeCurrentWIN(dout->hDC, dout->wgl.hSwapRC))
            AfxThrowError();

        if (dout->wgl.swaps)
        {
            for (afxUnit i = 0; i < dout->m.bufCnt; i++)
            {
                if (dout->wgl.swaps[i].swapFbo)
                {
                    AFX_ASSERT(gl->IsFramebuffer(dout->wgl.swaps[i].swapFbo));
                    gl->DeleteFramebuffers(1, &dout->wgl.swaps[i].swapFbo); _ZglThrowErrorOccuried();
                    dout->wgl.swaps[i].swapFbo = NIL;
                    dout->wgl.swaps[i].swapFboReady = FALSE;
                }
            }
        }

        wglMakeCurrentWIN(bkpHdc, bkpGlrc);

        wglDeleteContextWIN(dout->wgl.hSwapRC);
        dout->wgl.hSwapRC = NIL;
    }

    if (dout->hDC)
    {
        ReleaseDC(dout->hWnd, dout->hDC);
        dout->hDC = NIL;
    }

    dout->hDC = GetDC(dout->hWnd);
            
    if (!dout->dcPixFmt)
    {
        avxFormatDescription pfd;
        AvxDescribeFormats(1, &dout->m.ccfg.bins[0].fmt, &pfd);

        int pxlAttrPairCnt = 0;
        int pxlAttrPairs[][2] =
        {
            { WGL_SUPPORT_OPENGL_ARB, GL_TRUE }, // suportar o que se não OpenGL na fucking API do OpenGL???
            { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE },
#if 0
            { WGL_SUPPORT_GDI_ARB, GL_FALSE },
            { WGL_DRAW_TO_PBUFFER_ARB, GL_FALSE },
            { WGL_DRAW_TO_BITMAP_ARB, GL_FALSE },
            { WGL_BIND_TO_TEXTURE_RGB_ARB, GL_FALSE },
            { WGL_BIND_TO_TEXTURE_RGBA_ARB, GL_FALSE },
#endif
            { WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB },
            { WGL_DOUBLE_BUFFER_ARB, GL_TRUE }, // single buffered is not supported by drivers today. Qwadro will just virtualizes it.
            { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB },
#if 0
            /*
                WGL_SWAP_METHOD_ARB
                If the pixel format supports a back buffer, then this indicates
                how they are swapped. If this attribute is set to
                WGL_SWAP_EXCHANGE_ARB then swapping exchanges the front and back
                buffer contents; if it is set to WGL_SWAP_COPY_ARB then swapping
                copies the back buffer contents to the front buffer; if it is
                set to WGL_SWAP_UNDEFINED_ARB then the back buffer contents are
                copied to the front buffer but the back buffer contents are
                undefined after the operation. If the pixel format does not
                support a back buffer then this parameter is set to
                WGL_SWAP_UNDEFINED_ARB. The <iLayerPlane> parameter is ignored
                if this attribute is specified.
            */
            { WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB },
#endif
            /*
                WGL_COLOR_BITS_ARB
                The number of color bitplanes in each color buffer. For RGBA pixel types,
                it is the size of the color buffer, "excluding" the alpha bitplanes.
                For color-index pixels, it is the size of the color index buffer.
            */

            // ARGB8
            { WGL_COLOR_BITS_ARB, pfd.bpp - pfd.bpc[3] }, // WGL_COLOR_BITS_ARB must not include alpha bits.
            { WGL_ALPHA_BITS_ARB, pfd.bpc[3] },
#if !0
            { WGL_RED_BITS_ARB, pfd.bpc[0] },
            { WGL_GREEN_BITS_ARB, pfd.bpc[1] },
            { WGL_BLUE_BITS_ARB, pfd.bpc[2] },
#endif
#if !0
            { WGL_ALPHA_SHIFT_ARB, pfd.swizzle[3] },
            { WGL_RED_SHIFT_ARB, pfd.swizzle[0] },
            { WGL_GREEN_SHIFT_ARB, pfd.swizzle[1] },
            { WGL_BLUE_SHIFT_ARB, pfd.swizzle[2] },
#endif
#if 0
            { WGL_DEPTH_BITS_ARB, 0 }, // No Qwadro, não é possível desenhar arbitrariamente no default framebuffer. Logo, não há necessidade de stencil.
            { WGL_STENCIL_BITS_ARB, 0 },  // No Qwadro, não é possível desenhar arbitrariamente no default framebuffer. Logo, não há necessidade de stencil.
#endif
#if 0
            { WGL_AUX_BUFFERS_ARB, dout->m.bufCnt },
            { WGL_ACCUM_BITS_ARB, 0 },
            { WGL_ACCUM_ALPHA_BITS_ARB, 0 },
            { WGL_ACCUM_RED_BITS_ARB, 0 },
            { WGL_ACCUM_GREEN_BITS_ARB, 0 },
            { WGL_ACCUM_BLUE_BITS_ARB, 0 },
#endif
            { WGL_TRANSPARENT_ARB, (dout->m.presentAlpha && (dout->m.presentAlpha != avxVideoAlpha_OPAQUE)) },
            //{ WGL_SAMPLE_BUFFERS_ARB,  GL_FALSE },  // works on Intel, didn't work on Mesa
            //{ WGL_SAMPLES_ARB, 0 }, // works on Intel, didn't work on Mesa
            //{ WGL_COLORSPACE_EXT, dout->colorSpc == avxColorSpace_STANDARD ? WGL_COLORSPACE_SRGB_EXT : (dout->colorSpc == avxColorSpace_LINEAR ? WGL_COLORSPACE_LINEAR_EXT : NIL) }, // works on Mesa, didn't work on Intel
            { WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, (pfd.flags & avxFormatFlag_sRGB) }, // works on Mesa, didn't work on Intel
#if 0
            { WGL_NUMBER_OVERLAYS_ARB, 0 },
            { WGL_NUMBER_UNDERLAYS_ARB, 0 },

            { WGL_SHARE_DEPTH_ARB, FALSE },
            { WGL_SHARE_STENCIL_ARB, FALSE },
            { WGL_SHARE_ACCUM_ARB, FALSE },

            { WGL_STEREO_ARB, FALSE },
#endif
            { NIL, NIL },
        };

        UINT fmtCnt;
                
        //if (!wglChooseBestPixelFormatSIG(dout->hDC, &pxlAttrPairs[0][0], 0, 1, &dout->dcPixFmt, &fmtCnt)) AfxThrowError();
        if (!wglChoosePixelFormatARB(dout->hDC, &pxlAttrPairs[0][0], 0, 1, &dout->dcPixFmt, &fmtCnt)) AfxThrowError();
        else
        {
            AFX_ASSERT(dout->dcPixFmt);
            AFX_ASSERT(fmtCnt);
#if 0
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DIRECT3D_ACCELERATED | PFD_SUPPORT_COMPOSITION;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 24;
            pfd.cAlphaBits = 8;
            pfd.cDepthBits = 24;
            pfd.cStencilBits = 8;
#endif
            // The flag PFD_DIRECT3D_ACCELERATED is not an error, and it doesn't mean that you're using Direct3D instead of OpenGL. 
            // It's part of how WDDM (Windows Display Driver Model) drivers report capabilities.
            // So the flag PFD_DIRECT3D_ACCELERATED just means that the format is accelerated by the GPU, 
            // and possibly usable via Direct3D as well — it's a driver hint, not a directive.
            //dout->wglDcPxlFmt = _ZglChoosePixelFormat(dout->wglDc, &(pfd));
            DWORD pfdFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL
#if 0
                | PFD_SWAP_EXCHANGE
#endif
#if 0
                | PFD_DIRECT3D_ACCELERATED
#endif
#if 0
                | PFD_SUPPORT_COMPOSITION
#endif
                ;

            if (!wglDescribePixelFormatWIN(dout->hDC, dout->dcPixFmt, sizeof(dout->dcPfd), &dout->dcPfd))
            {
                if (!DescribePixelFormat(dout->hDC, dout->dcPixFmt, sizeof(dout->dcPfd), &dout->dcPfd))
                    AfxThrowError();
                else if (!SetPixelFormat(dout->hDC, dout->dcPixFmt, &dout->dcPfd))
                    AfxThrowError();
            }
            else
            {
                if (!wglSetPixelFormatWIN(dout->hDC, dout->dcPixFmt, &dout->dcPfd))
                    if (!SetPixelFormat(dout->hDC, dout->dcPixFmt, &dout->dcPfd))
                        AfxThrowError();
            }

            AFX_ASSERT(dout->dcPfd.dwFlags & pfdFlags);
        }
    }

    avxRange const screenRes =
    {
        GetDeviceCaps(dout->hDC, HORZRES),
        GetDeviceCaps(dout->hDC, VERTRES),
        GetDeviceCaps(dout->hDC, PLANES)
    };
    afxReal64 physAspRatio = (afxReal64)GetDeviceCaps(dout->hDC, HORZSIZE) / (afxReal64)GetDeviceCaps(dout->hDC, VERTSIZE);
    afxReal refreshRate = GetDeviceCaps(dout->hDC, VREFRESH);
    avxModeSetting mode = { 0 };
    AvxQuerySurfaceSettings(dout, &mode);
    mode.refreshRate = refreshRate;
    mode.wpOverHp = physAspRatio;
    mode.resolution = screenRes;
    AvxChangeSurfaceSettings(dout, &mode);

    HDC bkpHdc = wglGetCurrentDCWIN();
    HGLRC bkpGlrc = wglGetCurrentContextWIN();

    if (wglCreateContextSIGMA(dout->hDC, dsys->hPrimeRC, dsys->glVerMaj, dsys->glVerMin, FALSE, dsys->robustCtx, FALSE, &dout->wgl.hSwapRC, NIL, FALSE)) AfxThrowError();
    else
    {
        dout->wgl.gl = &dsys->gl;
        gl = dout->wgl.gl;

        if (!wglMakeCurrentWIN(dout->hDC, dout->wgl.hSwapRC)) AfxThrowError();
        else
        {
#if _AFX_DEBUG
            gl->Enable(GL_DEBUG_OUTPUT);
            gl->Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            gl->DebugMessageCallback(_glDbgMsgCb, NIL);
            //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#endif
            
            AFX_ASSERT(_WGL_EXT_swap_control);
            if (_WGL_EXT_swap_control)
            {
                if ((dout->m.presentMode & avxPresentFlag_RATE_LIMITED))
                {
                    // When greater than 0 can causes lag.
                    // It enables vsync (waits for vertical retrace).
                    // When >1, waits for multiple retraces.
                    wglSwapIntervalEXT(1);
                }
                else
                {
#if !0
                    // When tested on Intel iGPU, wglSwapIntervalEXT(-1) seem to be disallowing tearing.
                    // But I need to profile it.
                    wglSwapIntervalEXT(0);
#else
                    if ((dout->m.presentMode & avxPresentFlag_NO_TEARING) || !_WGL_EXT_swap_control_tear)
                    {
                        // It disables vsync (immediate swapping).
                        wglSwapIntervalEXT(0);
                    }
                    else
                    {
                        // It disables vsync but allows tearing.
                        // Rendering isn't delayed to sync with the display, but tearing artifacts may appear.
                        // Useful for benchmarking or reducing input latency.

                        // Useful when:
                        // we're writing a real-time or low-latency app (like a game or VR);
                        // we want uncapped framerates for benchmarking;
                        // we're okay with screen tearing (e.g., for input latency testing);
                        wglSwapIntervalEXT(-1);
                    }
#endif
                }
            }
        }

        if (err)
            wglDeleteContextWIN(dout->wgl.hSwapRC), dout->wgl.hSwapRC = NIL;
    }
    wglMakeCurrentWIN(bkpHdc, bkpGlrc);
    return err;
}

void _ZglPlaceWindowedSurfaceW32(HWND hwnd, afxRect const* rect)
{
    if (!rect || !IsWindow(hwnd)) return;

    // Copy desired client rectangle
    RECT desiredClientRect = { 0, 0, (LONG)rect->w, (LONG)rect->h };

    // Get window style
    DWORD style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);

    // Adjust window rect so the client area is exactly w x h
    AdjustWindowRectEx(&desiredClientRect, style, FALSE, exStyle);

    int winWidth = desiredClientRect.right - desiredClientRect.left;
    int winHeight = desiredClientRect.bottom - desiredClientRect.top;

    // Place the window so that the top-left of the client area is at (x, y)
    // But since the window includes borders and title bars, we need to offset the window
    int offsetX = rect->x + desiredClientRect.left;
    int offsetY = rect->y + desiredClientRect.top;

    SetWindowPos(hwnd,
        HWND_TOP,
        offsetX,
        offsetY,
        winWidth,
        winHeight,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}

void _ZglPlaceSurfaceW32(HWND hwnd, afxRect const* area)
{
    // Get the monitor where the window is (or closest to)
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    // Get monitor info
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!GetMonitorInfo(hMonitor, &monitorInfo))
        return;

    // Use rcMonitor for full monitor (including taskbar), or rcWork for work area
    RECT monitorRect = monitorInfo.rcMonitor;

    int screenWidth = monitorRect.right - monitorRect.left;
    int screenHeight = monitorRect.bottom - monitorRect.top;

    // Define desired client area (full screen)
    RECT desiredClientRect = { area->x, area->y, area->w, area->h };

    // Adjust the window rect to account for window styles (title bar, borders, etc.)
    DWORD style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);

    AdjustWindowRectEx(&desiredClientRect, style, FALSE, exStyle);

    // Calculate width and height including decorations
    int winWidth = desiredClientRect.right - desiredClientRect.left;
    int winHeight = desiredClientRect.bottom - desiredClientRect.top;

    // Move and resize the window so that the client area fills the screen
    SetWindowPos(hwnd,
        HWND_TOP,
        area->x, area->y,                    // Top-left corner of the screen
        winWidth,
        winHeight,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}

void _ZglPlaceFseSurfaceW32(HWND hwnd)
{
    // Get the monitor where the window is (or closest to)
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    // Get monitor info
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!GetMonitorInfo(hMonitor, &monitorInfo))
        return;

    // Use rcMonitor for full monitor (including taskbar), or rcWork for work area
    RECT monitorRect = monitorInfo.rcMonitor;

    int screenWidth = monitorRect.right - monitorRect.left;
    int screenHeight = monitorRect.bottom - monitorRect.top;

    // Desired client area is full monitor
    RECT desiredClientRect = { 0, 0, screenWidth, screenHeight };

    // Get window style
    DWORD style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    DWORD exStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);

    // Adjust window rect to match client area
    AdjustWindowRectEx(&desiredClientRect, style, FALSE, exStyle);

    int winWidth = desiredClientRect.right - desiredClientRect.left;
    int winHeight = desiredClientRect.bottom - desiredClientRect.top;

    // Move the window to monitor origin
    SetWindowPos(hwnd,
        HWND_TOP,
        monitorRect.left,
        monitorRect.top,
        winWidth,
        winHeight,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}

_QOW afxError _ZglDoutAdjust_WGL(afxSurface dout, afxRect const* area, afxBool fse)
{
    afxError err = AFX_ERR_NONE;

    _AvxDoutImplAdjustCb(dout, area, fse);

    if (dout->wgl.swaps)
        for (afxUnit i = 0; i < dout->m.bufCnt; i++)
            dout->wgl.swaps[i].swapFboReady = FALSE;

    return err;
}

_QOW afxError _ZglDoutIoctl_WGL(afxSurface dout, afxUnit code, va_list ap)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    switch (code)
    {
    case 0:
    {
        *va_arg(ap, HDC*) = dout->hDC;
        break;
    }
    case 1:
    {
        *va_arg(ap, int*) = dout->dcPixFmt;
        break;
    }
    default: break;
    }
    return err;
}

_QOW _avxDoutDdi const _ZGL_DOUT_DDI =
{
    .ioctlCb = _ZglDoutIoctl_WGL,
    .adjustCb = _ZglDoutAdjust_WGL,
    .regenCb = _AvxDoutImplRegenBuffers,
    .lockCb = NIL,
    .unlockCb = NIL,
    .presentCb = _ZglDoutPresent_WGL,
    .presOnDpuCb = (void*)_DpuPresentDout_BlitSwapFbo
};

_QOW afxError _ZglDoutDtorCb(afxSurface dout)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    _ZglDoutSuspendFunction(dout);

    if (dout->m.idd)
    {
        AfxDeallocate((void**)&dout->m.idd, AfxHere());
        dout->m.idd = NIL;
    }

    if (dout->wgl.swaps)
        AfxDeallocate((void**)&dout->wgl.swaps, AfxHere());

    AFX_ASSERT(!dout->wgl.hSwapRC); // disconnect should have released it.

    if (dout->wgl.hSwapRC)
        wglDeleteContextWIN(dout->wgl.hSwapRC);

    AFX_ASSERT(!dout->hDC); // disconnect should have released it.

    if (dout->hDC)
        ReleaseDC(dout->hWnd, dout->hDC);

    if (_AVX_DOUT_CLASS_CONFIG.dtor(dout))
        AfxThrowError();

    return err;
}

_QOW afxError _ZglDoutCtorCb(afxSurface dout, void** args, afxUnit invokeNo)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_DOUT, 1, &dout);

    afxDrawSystem dsys = args[0];
    AFX_ASSERT_OBJECTS(afxFcc_DSYS, 1, &dsys);
    afxSurfaceConfig const* cfg = ((afxSurfaceConfig const *)args[1]) + invokeNo;
    AFX_ASSERT(cfg);

    if (_AVX_DOUT_CLASS_CONFIG.ctor(dout, (void*[]) { dsys, (void*)cfg }, 0))
    {
        AfxThrowError();
        return err;
    }

    dout->m.pimpl = &_ZGL_DOUT_DDI;

    dout->hInst = cfg->iop.w32.hInst;
    dout->hWnd = cfg->iop.w32.hWnd;
    dout->hDC = NIL;
    dout->dcPixFmt = 0;
    dout->isWpp = FALSE;
    dout->wgl.hSwapRC = NIL;
    dout->wgl.swapOnWgl = TRUE;
    dout->wgl.swaps = NIL;
    
    if (AfxAllocate(dout->m.bufCnt * sizeof(dout->wgl.swaps[0]), 0, AfxHere(), (void**)&dout->wgl.swaps))
        AfxThrowError();

    for (afxUnit i = 0; i < dout->m.bufCnt; i++)
    {
        dout->wgl.swaps[i].swapFbo = NIL;
        dout->wgl.swaps[i].swapFboReady = FALSE;
    }

    if (_ZglRelinkDoutCb_WGL(dout))
        AfxThrowError();

    HWND hWnd = cfg->iop.w32.hWnd;

    if (hWnd)
    {

        // fetch capabilities of the visual display unit.
#if 0
        HDC dc = GetWindowDC(hWnd);

        if (!dc) AfxThrowError();
        else
        {
            afxWarp const screenRes =
            {
                GetDeviceCaps(dc, HORZRES),
                GetDeviceCaps(dc, VERTRES),
                GetDeviceCaps(dc, PLANES)
            };
            afxReal64 physAspRatio = (afxReal64)GetDeviceCaps(dc, HORZSIZE) / (afxReal64)GetDeviceCaps(dc, VERTSIZE);
            afxReal refreshRate = GetDeviceCaps(dc, VREFRESH);
            AvxChangeSurfaceSettings(dout, physAspRatio, refreshRate, screenRes, FALSE);
        }
        ReleaseDC(hWnd, dc);
#endif

#if 0
        afxDrawDevice ddev = AfxGetDrawOutputDevice(dout);

        if (ddev->useDxgiSwapchain)
        {
            if (ddev->hasDxInterop1)
            {
                HRESULT hr;
                AFX_ASSERT(hr);
                hr = ddev->Direct3DCreate9Ex(D3D_SDK_VERSION, &dout->d3dsw9.d3d9Ex);

                if (FAILED(hr))
                {
                    AfxThrowError();
                    //return -1;
                }

                D3DPRESENT_PARAMETERS d3dpp =
                {
                    .Windowed = TRUE,
                    .BackBufferWidth = dout->m.extent.w,
                    .BackBufferHeight = dout->m.extent.h,
                    // Add one frame for the backbuffer and one frame of "slack" to reduce contention with the window manager when acquiring the backbuffer
                    .BackBufferCount = dout->m.bufCnt,
                    .SwapEffect = 1 ? D3DSWAPEFFECT_FLIPEX : D3DSWAPEFFECT_FLIP,
                    // Automatically get the backbuffer format from the display format
                    .BackBufferFormat = D3DFMT_X8R8G8B8,
                    .PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE,
                    .hDeviceWindow = dout->hWnd,
                };

                hr = IDirect3D9Ex_CreateDeviceEx(dout->d3dsw9.d3d9Ex, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dout->hWnd,
                    D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED | D3DCREATE_NOWINDOWCHANGES,
                    &d3dpp, NULL, &dout->d3dsw9.d3d9DevEx);

                if (FAILED(hr))
                {
                    AfxThrowError();
                    //return -1;
                }

                hr = IDirect3DDevice9Ex_CreateAdditionalSwapChain(dout->d3dsw9.d3d9DevEx, &d3dpp, &dout->d3dsw9.d3d9Sw);

                if (FAILED(hr))
                {
                    AfxThrowError();
                }

                IDirect3DSurface9* backBuf;
                IDirect3DDevice9Ex_GetBackBuffer(dout->d3dsw9.d3d9DevEx, 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuf);

                IDirect3DTexture9* sharedTexture;
                hr = IDirect3DDevice9Ex_CreateTexture(dout->d3dsw9.d3d9DevEx, 800, 600, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &sharedTexture, NULL);

                if (FAILED(hr))
                {
                    AfxThrowError();
                }

                IDirect3DSurface9* pSurface = NULL;
                hr = IDirect3DTexture9_GetSurfaceLevel(sharedTexture, 0, &pSurface);

                if (FAILED(hr))
                {
                    AfxThrowError();
                }
            }
        }
#endif
    }

    if (err && _AVX_DOUT_CLASS_CONFIG.dtor(dout))
        AfxThrowError();

    return err;
}
