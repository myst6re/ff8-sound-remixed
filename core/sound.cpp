#include "sound.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#endif

Sound::Sound() :
    _dataOffset(0), _dataLength(0), _bufferReadCursor(0), _bufferWriteCursor(0),
    _adpcmData(QByteArray()), _format(WaveFormatEx()), _isLooping(false)
{
}

void Sound::setDataInfos(uint32_t offset, uint32_t length)
{
    _dataOffset = offset;
    _dataLength = length;
}

void Sound::setBufferCursor(uint32_t read, uint32_t write)
{
    _bufferReadCursor = read;
    _bufferWriteCursor = write;
}

void Sound::setIsLooping(bool isLooping)
{
    _isLooping = isLooping;
}

void Sound::setWaveFormat(const WaveFormatEx &format, const QByteArray &adpcmData)
{
    _format = format;
    _adpcmData = adpcmData;
}

bool Sound::play(QIODevice *audioDat) const
{
    if (! audioDat->seek(_dataOffset)) {
        return false;
    }
    QByteArray data = audioDat->read(_dataLength);
    
    if (data.isEmpty()) {
        return false;
    }
    
    if (_format.wFormatTag == WAVE_FORMAT_ADPCM) {
#ifdef Q_OS_WIN
        HACMSTREAM has[2];
        WaveFormatEx destFormat = _format;
        destFormat.wFormatTag = WAVE_FORMAT_PCM;
        destFormat.wBitsPerSample = 16;
        destFormat.nBlockAlign = 16 * destFormat.nChannels / 8;
        destFormat.nAvgBytesPerSec = destFormat.nBlockAlign * destFormat.nSamplesPerSec;
        destFormat.cbSize = 0;
    
        acmStreamOpen(has, nullptr, (LPWAVEFORMATEX)&_format, (LPWAVEFORMATEX)&destFormat, nullptr, 0, 0, 0);
        
        ulong dstLength;
        if (acmStreamSize(has[0], data.size(), &dstLength, 0) == ERROR_SUCCESS) {
            if (dstLength != 0) {
                QByteArray targetData;
                targetData.reserve(dstLength);
                struct tACMSTREAMHEADER pash = tACMSTREAMHEADER();
                pash.pbSrc = (LPBYTE)data.constData();
                pash.cbSrcLength = data.size();
                pash.cbStruct = 84;
                pash.pbDst = (LPBYTE)targetData.data();
                pash.cbDstLength = dstLength;
                acmStreamPrepareHeader(has[0], &pash, 0);
                acmStreamConvert(has[0], &pash, 0);
                acmStreamUnprepareHeader(has[0], &pash, 0);
                acmStreamClose(has[0], 0);
                data = targetData;
            }
        }
#else
        return false;
#endif
    }
    
    // Play
    
#ifdef Q_OS_WIN
    
#else
    return false;
#endif
    
    return true;
}

QByteArray Sound::toWav(QIODevice *audioDat) const
{
    QByteArray ret;
    
    if (! audioDat->seek(_dataOffset)) {
        return ret;
    }
    QByteArray data = audioDat->read(_dataLength);
    
    if (data.isEmpty()) {
        return ret;
    }

    uint32_t length;
    
    // Credits to Qhimm (from FF8SND)
    ret.append("RIFF", 4);
    length = _dataLength + 36 + _adpcmData.size();
    ret.append((const char *)&length, 4);
    ret.append("WAVEfmt ", 8);
    length = 18 + _adpcmData.size();
    ret.append((const char *)&length, 4);
    ret.append((const char *)&_format, 18);
    ret.append(_adpcmData);
    ret.append("data", 4);
    ret.append((const char *)&_dataLength, 4);
    ret.append(data);
    
    return ret;
}
