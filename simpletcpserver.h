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
        clientSocket(INVALID_SOCKET) {
    }

    int setup();
    int waitForClient();
    int sendData(const char* buf, int length);
    int shutdown();

private:
    SOCKET listenSocket;
    SOCKET clientSocket;

    void dumpLocalIp();
};
