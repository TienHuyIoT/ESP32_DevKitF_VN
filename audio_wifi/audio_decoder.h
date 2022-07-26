#ifndef __AUDIO_DECODER_H
#define __AUDIO_DECODER_H

#include <Arduino.h>
#include <FS.h>
#include "THIoT_ESPBoard.h"
#include "THIoT_ESPConfig.h"

/* Change SPI speed from 4Mhz to 8Mhz */
#include <VS1053.h>

#define VS1053_VOLUME_DEFAULT  100 // volume level 0-100

class AUDIOEncoder
{
private:
    VS1053 player;
    fs::FS* _fs;
    String _fileName;
    int _repeatNumber;
    int _playingStatus;
    xSemaphoreHandle _xb_semaphore_play;
    xSemaphoreHandle _xb_semaphore_stop;
    xSemaphoreHandle xMutex_PlayingStatus;
    int playingStatusGet();
    void playingStatusSet(int status);

    enum : uint8_t {
        AUDIO_STOP,
        AUDIO_PLAYING
    };

public:
    AUDIOEncoder(fs::FS &fs = NAND_FS_SYSTEM);
    ~AUDIOEncoder();
    void begin();
    /* Play a MP3 file */
    void playFile(const char* fileName, int repeatNumber = 0);
    /* Force stop while playing */
    void stop();
    /* Handle in loop */
    void handle();
};

#endif // __AUDIO_DECODER_H