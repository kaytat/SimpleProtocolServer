# Simple Protocol Server

This is server that sends all the available sound card audio from a Windows PC to the network.  This is meant to be the Windows counterpart to the simple protocol module in PulseAudio.  The simple protocol streams uncompressed 16 bit PCM data.

The server is meant to be used with the Simple Protocol Player client on Android.  The play store link is here: https://play.google.com/store/apps/details?id=com.kaytat.simpleprotocolplayer .

## WSAPI Loopback

The server uses the Windows Audio Session API to capture the audio.  And for each audio frame, it sends that data to the connected client.

The code is a simple modification to an existing MSDN example: http://blogs.msdn.com/b/matthew_van_eerde/archive/2014/11/05/draining-the-wasapi-capture-buffer-fully.aspx .  Instead of writing the data to a MMIO file, the data is send over a socket.

It looks like the sampling frequency can't be changed in for loopback, and so the data is 2 channel, 16 bit PCM at 48kHz.

And unfortunately, the current Android app doesn't support it.  It's been worked on.
