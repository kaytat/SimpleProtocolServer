/*
    Copyright (C) 2014 kaytat

    Originally derived from the example posted here:
        http://blogs.msdn.com/b/matthew_van_eerde/archive/2014/11/05/draining-the-wasapi-capture-buffer-fully.aspx

    Since there is no explict license, the Microsoft LPL applies:
        http://msdn.microsoft.com/en-us/cc300389.aspx
*/

// guid.cpp

#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
