/*
    Copyright (C) 2014 kaytat

    Originally derived from the example posted here:
        http://blogs.msdn.com/b/matthew_van_eerde/archive/2014/11/05/draining-the-wasapi-capture-buffer-fully.aspx

    Since there is no explict license, the Microsoft LPL applies:
        http://msdn.microsoft.com/en-us/cc300389.aspx
*/

// loopback-capture.h

// call CreateThread on this function
// feed it the address of a LoopbackCaptureThreadFunctionArguments
// it will capture via loopback from the IMMDevice
// and dump output to the HMMIO
// until the stop event is set
// any failures will be propagated back via hr

struct LoopbackCaptureThreadFunctionArguments {
    IMMDevice *pMMDevice;
    bool bInt16;
    HANDLE hStartedEvent;
    HANDLE hStopEvent;
    UINT32 nFrames;
    HRESULT hr;
    bool bMono;
    INT32 iSampleRateDivisor;
};

DWORD WINAPI LoopbackCaptureThreadFunction(LPVOID pContext);
