/*
    Copyright (C) 2014 kaytat

    Originally derived from the example posted here:
        http://blogs.msdn.com/b/matthew_van_eerde/archive/2014/11/05/draining-the-wasapi-capture-buffer-fully.aspx

    Since there is no explict license, the Microsoft LPL applies:
        http://msdn.microsoft.com/en-us/cc300389.aspx
*/

// prefs.h

class CPrefs {
public:
    IMMDevice *m_pMMDevice;
    bool m_bInt16;
    PWAVEFORMATEX m_pwfx;

    // set hr to S_FALSE to abort but return success
    CPrefs(int argc, LPCWSTR argv[], HRESULT &hr);
    ~CPrefs();

};
