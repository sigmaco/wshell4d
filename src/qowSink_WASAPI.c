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

#include "qowAudio_W32.h"
#include "../../icd_amiga/src/zalInteropWasapi.h"
//#include "qowBase.h"

_QOW afxError _QowAsioLockCb(afxSink asi, afxUnit64 timeout, afxUnit minFrameCnt, amxBufferedTrack* room)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_ASIO, 1, &asi);

    afxUnit paddingFrameCnt, bufFrameCnt;
    afxUnit wRoom = wasapiOutputGetRoom(&asi->idd.wasapi, &paddingFrameCnt, &bufFrameCnt);
    
    void* p;
    err = wasapiOutputLock(&asi->idd.wasapi, minFrameCnt, &p);

    *room = (amxBufferedTrack) { .offset = (afxSize)p, .frameCnt = asi->idd.wasapi.lockedOutFrameCnt, .freq = asi->m.freq };

    return err;
}

_QOW afxError _QowAsioUnlockCb(afxSink asi, afxFlags flags)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_ASIO, 1, &asi);
#if 0
    amxAudioIo op = { 0 };
    op.chansPerFrame = 2;
    op.samplesPerChan = 1;
    op.period.chanCnt = 2;
    op.period.sampCnt = asi->idd.wasapi.lockedOutFrameCnt;
    _AmxDumpAudio(asi->m.buffers[0], &op, asi->idd.wasapi.lockedOutPtr);
#endif
    //AfxCopy(asi->idd.wasapi.lockedOutPtr, AmxGetBufferMap(asi->m.buffers[0]->buf, asi->idd.wasapi.lockedOutBaseFrame * 2 * 2, 1), asi->idd.wasapi.lockedOutFrameCnt * 2 * 2);
    err = wasapiOutputUnlock(&asi->idd.wasapi, asi->idd.wasapi.lockedOutFrameCnt, NIL);

    return err;
}

_QOW void _QowAsioFlushCb(afxSink asi)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_ASIO, 1, &asi);

    afxUnit paddingFrameCnt, bufFrameCnt;
    afxUnit wRoom = wasapiOutputGetRoom(&asi->idd.wasapi, &paddingFrameCnt, &bufFrameCnt);
#if 0
    if (wRoom)
        _QowWasapiWriteParture(&asi->idd.wasapi, wRoom, NIL);
#endif

    if (bufFrameCnt)
    {
        AFX_ASSERT(asi->idd.wasapi.bufferFrameCount >= bufFrameCnt - paddingFrameCnt);

        afxUnit frameCnt = 0;
        if ((frameCnt = audio_ringbuffer_available(&asi->m.rb)))
        {
#if 0
            afxUnit minFrameCnt = AFX_MIN(frameCnt, bufFrameCnt);
            void* p;
            err = wasapiOutputLock(&asi->idd.wasapi, minFrameCnt, &p);
            if (!err)
            {
                audio_ringbuffer_read(&asi->m.rb, p, 4, asi->idd.wasapi.lockedOutFrameCnt);
                err = wasapiOutputUnlock(&asi->idd.wasapi, asi->idd.wasapi.lockedOutFrameCnt, NIL);
            }
#else
            if (asi->idd.wasapi.pRenderClient)
            {
                wasapiOutputRb(&asi->idd.wasapi, &asi->m.rb);
            }
            else
            {
                wasapiInputRb(&asi->idd.wasapi, &asi->m.rb);
            }
#endif
        }
    }
}

_QOW afxError _QowAsioDtorCb(afxSink asi)
{
    afxError err = AFX_ERR_NONE;
    AFX_ASSERT_OBJECTS(afxFcc_ASIO, 1, &asi);

    afxMixDevice sdev = AfxGetHost(asi);

    _ZalWasapiStartStop(&asi->idd.wasapi, FALSE);

    if (_ZalWasapiDestroy(&asi->idd.wasapi))
        AfxThrowError();

    return err;
}

_QOW afxError _QowAsioCtorCb(afxSink asi, void** args, afxUnit invokeNo)
{
    afxResult err = NIL;
    AFX_ASSERT_OBJECTS(afxFcc_ASIO, 1, &asi);

    afxMixSystem msys = args[0];
    AFX_ASSERT_OBJECTS(afxFcc_MSYS, 1, &msys);
    afxSinkConfig const* cfg = ((afxSinkConfig const *)args[1]) + invokeNo;
    AFX_ASSERT(cfg);
    afxBool record = *(afxBool*)(args[2]);

    afxUri dev;
    afxUri endpoint;
    //AfxClipUriDevice(&dev, &endpoint);
    
    "//./wasapi/mic";
    "//./wasapi/aux";
    "//./wasapi/mix";


    if (_AMX_ASIO_CLASS_CONFIG.ctor(asi, (void*[]) { msys, (void*)cfg }, 0))
    {
        AfxThrowError();
        return err;
    }
    
    if (_ZalWasapiCreate(&asi->idd.wasapi, asi->m.fmt, asi->m.chanCnt, asi->m.freq))
        AfxThrowError();

    asi->m.flushCb = _QowAsioFlushCb;
    asi->m.lockCb = _QowAsioLockCb;
    asi->m.unlockCb = _QowAsioUnlockCb;

    _ZalWasapiStartStop(&asi->idd.wasapi, TRUE);

    return err;
}
