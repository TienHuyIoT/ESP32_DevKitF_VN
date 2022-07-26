#include "THIoT_SerialTrace.h"
#include "audio_decoder.h"

#define AUDIO_ENCODER_TAG_CONSOLE(...) SERIAL_TAG_LOGI("[AUDIO_ENCODER]", __VA_ARGS__)
#define AUDIO_ENCODER_FUNCTION_CONSOLE(...) SERIAL_FUNCTION_TAG_LOGI("[AUDIO_ENCODER]", __VA_ARGS__)

AUDIOEncoder::AUDIOEncoder(fs::FS &fs)
    : _fs(&fs), player(VS1053_CS, VS1053_DCS, VS1053_DREQ)
{
    vSemaphoreCreateBinary(_xb_semaphore_play);
    vSemaphoreCreateBinary(_xb_semaphore_stop);
    xMutex_PlayingStatus = xSemaphoreCreateMutex();
}

AUDIOEncoder::~AUDIOEncoder()
{
}

void AUDIOEncoder::begin()
{
    AUDIO_ENCODER_TAG_CONSOLE("Hello VS1053!");
    // initialize a player
    player.begin();
    // Only perform an update if we really are using a VS1053, not. eg. VS1003
    if (player.getChipVersion() == 4)
    {
        player.loadDefaultVs1053Patches();
    }
    player.switchToMp3Mode(); // optional, some boards require this
    player.setVolume(VS1053_VOLUME_DEFAULT);

    xSemaphoreTake(_xb_semaphore_play, 0);
    xSemaphoreTake(_xb_semaphore_stop, 0);
}

void AUDIOEncoder::playFile(const char *fileName, int repeatNumber)
{
    AUDIO_ENCODER_TAG_CONSOLE("Play sound %s", fileName);
    stop();
    _fileName = fileName;
    _repeatNumber = repeatNumber;
    playingStatusSet(AUDIOEncoder::AUDIO_PLAYING);
    xSemaphoreGive(_xb_semaphore_play);
}

void AUDIOEncoder::stop()
{
    if (playingStatusGet() == AUDIOEncoder::AUDIO_PLAYING)
    {
        playingStatusSet(AUDIOEncoder::AUDIO_STOP);
        AUDIO_ENCODER_TAG_CONSOLE("Stop sound %s", _fileName.c_str());
    }
    /* Wait handle task reached end */
    xSemaphoreTake(_xb_semaphore_stop, 200 / portTICK_PERIOD_MS);
}

void AUDIOEncoder::handle()
{
    xSemaphoreTake(_xb_semaphore_play, portMAX_DELAY);
    boolean runFlag = 1;
    File audioFile = _fs->open(_fileName);
    if (!audioFile)
    {
        AUDIO_ENCODER_TAG_CONSOLE("File %s is not exist", _fileName.c_str());
        return;
    }
    AUDIO_ENCODER_TAG_CONSOLE("Playing %s", _fileName.c_str());

    int playTimeMs = millis();
    constexpr int AUDIO_BUFF_SIZE = 128;
    uint8_t *buff = (uint8_t *)malloc(AUDIO_BUFF_SIZE);
    while (playingStatusGet() == AUDIOEncoder::AUDIO_PLAYING)
    {
        if (digitalRead(VS1053_DREQ))
        {
            int length = audioFile.read(buff, AUDIO_BUFF_SIZE);
            if (0 == length)
            {
                AUDIO_ENCODER_TAG_CONSOLE("_repeatNumber = %u", _repeatNumber);
                if (_repeatNumber > 0)
                {
                    --_repeatNumber;
                    audioFile.seek(0);
                }
                else
                {
                    break;
                }
            }
            else
            {
                player.playChunk(buff, length);
            }
        }
    }
    playTimeMs = millis() - playTimeMs;
    AUDIO_ENCODER_TAG_CONSOLE("End %s, Play time = %u(s)", _fileName.c_str(), playTimeMs/1000);
    free(buff);
    audioFile.close();
    xSemaphoreGive(_xb_semaphore_stop);
}

int AUDIOEncoder::playingStatusGet()
{
    int status;
    xSemaphoreTake(xMutex_PlayingStatus, portMAX_DELAY);
    status = _playingStatus;
    xSemaphoreGive(xMutex_PlayingStatus);
    return status;
}

void AUDIOEncoder::playingStatusSet(int status)
{
    xSemaphoreTake(xMutex_PlayingStatus, portMAX_DELAY);
    _playingStatus = status;
    xSemaphoreGive(xMutex_PlayingStatus);
}