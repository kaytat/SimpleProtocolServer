/*
    Copyright (C) 2014 kaytat

    Originally derived from the example posted here:
        http://msdn.microsoft.com/en-us/library/windows/desktop/ms737593%28v=vs.85%29.aspx

    Since there is no explict license, the Microsoft LPL applies:
        http://msdn.microsoft.com/en-us/cc300389.aspx
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "simpletcpserver.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

int SimpleTcpServer::setup()
{
    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    dumpLocalIp();

    return 0;
}

int SimpleTcpServer::waitForClient()
{
    printf("%s ...\n", __FUNCTION__);
    // Accept a client socket
    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;

    // Dump socket info
    struct sockaddr peerInfo;
    int peerInfoSize = sizeof(peerInfo);
    int ret = getpeername(clientSocket, &peerInfo, &peerInfoSize);
    if (ret == 0) {
        WCHAR addrStr[256];
        DWORD addrStrSize = sizeof(addrStr);
        ret = WSAAddressToString(
                &peerInfo,
                peerInfoSize,
                NULL,
                addrStr,
                &addrStrSize);
        if (ret == 0) {
            printf("Peer addr: %ls\n", addrStr);
        }
        else {
            printf("Error: WSAAddressToString %d\n", WSAGetLastError());
        }
    }
    else {
        printf("Error: getpeername %d\n", WSAGetLastError());
    }
    return 0;
}

#define PI 3.14159265

// A utility to generate a sine wave.  For testing.
short getSineSample()
{
    static int sineCounter = 0;
    double rad, result;

    rad = sineCounter++;

    // Assuming an output rate of 48khz, this should
    // generate a 1khz sine tone
    rad = fmod(rad, 48.0);
    rad = rad * 2.0 * PI;
    result = sin(rad);

    // sin's range is [-1,+1] scale that to something close to
    // the range of a short.
    return (short)(result * 32000.0);
}

int SimpleTcpServer::sendData(const char* buf, int length)
{
    int iSendResult;
    short* bufferToSend;
    short* tmpBuffer;
    int samplesToSend = 0;
    int numCaptureSamples = length / 2;
    const short* captureSamples = (const short*)buf;

    // Super-simple stereo to mono conversion by averaging the left and right sample
    if (mono || sampleRateDivisor != 1) {
        if (length > bufferSize) {
            printf("Error: capture buffer too large: %d %d\n", length, bufferSize);
            return 0;
        }

        bufferToSend = (short*)scratchBuffer;
        tmpBuffer = (short*)scratchBuffer;

        int s1, s2;
        while (numCaptureSamples > 0) {
            numCaptureSamples -= 2;

            // TODO: eliminate all these 'if's in loop
            if (divisorCounter == 0) {
                if (mono) {
                    s1 = (*captureSamples++);
                    s2 = (*captureSamples++);
                    // Average left and right.  Assume 16 bits per sample, stereo
                    *tmpBuffer++ = (short)((s1 + s2) / 2);
                    samplesToSend++;
                }
                else {
                    *tmpBuffer++ = *captureSamples++;
                    *tmpBuffer++ = *captureSamples++;
                    samplesToSend += 2;
                }
                if (sampleRateDivisor != 1) {
                    divisorCounter++;
                }
            }
            else {
                // Skip 2 samples assuming stereo
                captureSamples += 2;
                divisorCounter++;
                if (divisorCounter == sampleRateDivisor) {
                    divisorCounter = 0;
                }
            }
        }
    }
    else {
        bufferToSend = (short*)buf;
        samplesToSend = numCaptureSamples;
    }

    // Receive until the peer shuts down the connection
    iSendResult = send(clientSocket, (char*)bufferToSend, samplesToSend * 2, 0);
    if (iSendResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    return 0;
}

int SimpleTcpServer::shutdown()
{
    if (clientSocket != INVALID_SOCKET) {
        // shutdown the connection since we're done
        int iResult = ::shutdown(clientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        closesocket(clientSocket);
    }

    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }

    // cleanup
    WSACleanup();

    return 0;
}

void SimpleTcpServer::dumpLocalIp()
{
    int i;
    struct hostent *remoteHost;
    struct in_addr addr;

    char **pAlias;
    char localHostName[80];

    if (gethostname(localHostName, sizeof(localHostName)) == SOCKET_ERROR) {
        printf("Error: gethostname %d\n", WSAGetLastError());
        return;
    }

    remoteHost = gethostbyname(localHostName);
    if (remoteHost == NULL) {
        printf("Error: gethostbyaddr %d\n", WSAGetLastError());
        return;
    }

    printf("Official name: %s\n", remoteHost->h_name);
    for (i = 0, pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
        printf("Alternate name #%d: %s\n", ++i, *pAlias);
    }
    if (remoteHost->h_addrtype == AF_INET) {
        i = 0;
        while (remoteHost->h_addr_list[i] != 0) {
            addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
            printf("\n\n\tIPv4 Address #%d: %s\n\n\n", i, inet_ntoa(addr));
        }
    }
    else if (remoteHost->h_addrtype == AF_INET6) {
        printf("Remotehost is an IPv6 address\n");
    }
}
