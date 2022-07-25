#ifndef __AUDIO_DECODER_H
#define __AUDIO_DECODER_H

#include <Arduino.h>
#include <FS.h>
#include "THIoT_ESPBoard.h"
#include "THIoT_ESPConfig.h"
#include <VS1053.h>

#define VOLUME  100 // volume level 0-100

class AUDIOEncoder
{
private:
    VS1053 player;
    fs::FS* _fs;
    File _audioFile;
    String _fileName;
    boolean _runFlag;
    xSemaphoreHandle _xb_semaphore;

public:
    AUDIOEncoder(fs::FS &fs = NAND_FS_SYSTEM);
    ~AUDIOEncoder();
    void begin();
    void playFile(const char* fileName);
    void handle();
};

#endif // __AUDIO_DECODER_H