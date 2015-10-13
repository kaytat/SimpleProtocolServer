/*
    Copyright (C) 2014 kaytat

    Originally derived from the example posted here:
        http://msdn.microsoft.com/en-us/library/windows/desktop/ms737593%28v=vs.85%29.aspx

    Since there is no explict license, the Microsoft LPL applies:
        http://msdn.microsoft.com/en-us/cc300389.aspx
*/

#pragma once

#define DEFAULT_PORT "12345"

class SimpleTcpServer {
public:
    SimpleTcpServer() :
        listenSocket(INVALID_SOCKET),
        clientSocket(INVALID_SOCKET),
        mono(true),
        sampleRateDivisor(1),
        divisorCounter(0),
        bufferSize(0),
        scratchBuffer(NULL) {
    }

    ~SimpleTcpServer() {
        if (scratchBuffer != NULL) {
            delete scratchBuffer;
            scratchBuffer = NULL;
        }
    }

    int setup();
    int waitForClient();
    void configure(
        bool mono,
        unsigned int sampleRateDivisor,
        int bufferSize) {
        this->mono = mono;
        if (sampleRateDivisor >= 1) {
            this->sampleRateDivisor = sampleRateDivisor;
        }
        else {
            this->sampleRateDivisor = 1;
        }
        this->divisorCounter = 0;
        this->bufferSize = bufferSize;
        this->scratchBuffer = new short[bufferSize / 2];
    }
    int sendData(const char* buf, int length);
    int shutdown();

private:
    SOCKET listenSocket;
    SOCKET clientSocket;
    bool mono;
    unsigned int sampleRateDivisor;
    unsigned int divisorCounter;
    int bufferSize;
    short* scratchBuffer;
    void dumpLocalIp();
};
