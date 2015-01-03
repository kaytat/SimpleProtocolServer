# Simple Protocol Server

This is a server that sends all the available sound card audio from a Windows PC to the network.  This is meant to be the Windows counterpart to the simple protocol module in PulseAudio.  The simple protocol streams uncompressed 16 bit 2 channel PCM data.

The server is meant to be used with the Simple Protocol Player client on Android.  The play store link is here: https://play.google.com/store/apps/details?id=com.kaytat.simpleprotocolplayer .

## WSAPI Loopback

The server uses the Windows Audio Session API to capture the audio.  And for each audio frame, it sends that data to the connected client.

The code is a simple modification to an existing MSDN example: http://blogs.msdn.com/b/matthew_van_eerde/archive/2014/11/05/draining-the-wasapi-capture-buffer-fully.aspx .  Instead of writing the data to a MMIO file, the data is send over a socket.

## Limitations

### One client only
Although PulseAudio's module will allow multiple connections, this server does not.

### No XP support
I've only tested this with one Win7 machine.  And it doesn't seem to work with XP.

### Sampling rate / format
The Android app will support two sampling frequencies - 44.1 and 48 kHz, just in case the sound card doesn't support one of them.  The app will assume 16 bit, 2 channel PCM.

The sampling rate may need to be manually adjusted by the user since the default format may not be a format supported by the Android app.  To do this, the user will need to manually configure the properties in the "Advanced" tab of the playback device.  Please also see this: http://kaytat.com/blog/?p=332
