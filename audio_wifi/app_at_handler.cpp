#include "app_at_handler.h"
#include "c_utility.h"
#include "THIoT_SerialTrace.h"
#include "audio_decoder.h"
#include "buzz_alarm.h"
#include "led_alert.h"

#define APP_AT_FUNC_TAG_CONSOLE(...) SERIAL_FUNCTION_TAG_LOGI("[APP_AT] ", __VA_ARGS__)

#define POINTER_EOS(x)              (x + strlen(x)) /* Pointer end of string */
#define EQUATION_CODE(x)            (((x + 6) / 7) * 5 + 1)
#define AT_CMD_DYNAMIC_BUFF_SIZE    64
#define AT_CMD_CRC_LENGTH           5
#define AT_CMD_BUFF_SIZE            64 /* 64 byte data */
#define AT_CMD_POLLING_HANDLER_MS   10

extern AUDIOEncoder PlayAudio;
extern ESPBlinkGPIO BuzzAlarm;
extern ESPBlinkGPIO LedAlert;

/* The counter number blink buzz and led */
enum : uint8_t {
    ALARM_NEW_DRIVER_CNT = 2,
    ALARM_NO_DRIVER_CNT = 1,
    ALARM_DROWSINESS_CNT = 20
};

enum : uint8_t {
    NEW_DRIVER_REPEAT = 0,
    DROWSINESS_REPEAT = 1,
    NO_DRIVER_REPEAT = 0
};

enum : uint8_t {
    NEW_DRIVER_ID = 0,
    DROWSINESS_ID,
    NO_DRIVER_ID,
    AUDIO_LIST_ID_NUM
};

const char* const audioList[] PROGMEM = {
 "/NEW_DRIVER.MP3",
 "/DROWSINESS.MP3",
 "/NO_DRIVER.MP3"
};

static char* truncate_last_character(char *buff, int character)
{
    char* s = strrchr(buff, character);
    if (s != nullptr)
    {
        *s = '\0'; /* Add null terminal */
        s++; /* Offset to start of pointer license plates string */
    }

    return s;
}

static boolean verify_and_trim_crc(at_funcation_t* at, char* buff)
{
    char *s;
    uint8_t crc, crc_cal;

    s = strrchr(buff, ',');
    if (s != nullptr)
    {
        *s = '\0';
        crc = atol(s + 1);
        crc_cal = crc_xor((const uint8_t *)at_head, AT_HEAD_LENGTH);
        crc_cal ^= crc_xor((const uint8_t *)at->cmd_name, at->cmd_len);
        crc_cal ^= '=';
        crc_cal ^= crc_xor((const uint8_t *)buff, strlen(buff));
        if (crc == crc_cal)
        {
            return true;
        }
    }

    APP_AT_FUNC_TAG_CONSOLE("Crc %u Error, Crc expected %u", crc, crc_cal);

    return false;
}

#if (0)
static SerialHandleCallbacks SerialHandleDefault;
#endif

APPATHandler::APPATHandler()
{
#if (0)
    _pSerialCallbacks = &SerialHandleDefault;
#endif
}

APPATHandler::~APPATHandler()
{
    if (_atBuffer.buf)
    {
        delete[] _atBuffer.buf;
    }
}

void APPATHandler::run(at_funcation_t* tableHandler, uint8_t cmdNumber)
{
    /* Init at command process */
    _atCmdHandler.cmd_table = tableHandler;
    _atCmdHandler.cmd_num = cmdNumber;
    _atBuffer.buf = new char[AT_CMD_BUFF_SIZE + 1];
    _atBuffer.size = AT_CMD_BUFF_SIZE + 1;
    at_device_init(&_atCmdHandler, &_atBuffer);

    /* Register question setup command */
    at_setup_attach_cb(AT_QUESTION, [](at_funcation* at, char *data){
        APPATHandler* atHandler = (APPATHandler*)(at->arg);
        atHandler->cmdSetupQuestionHandle(at, data);
    }, this);

    /* Register new driver exe command */
    at_exe_attach_cb(AT_NEW_DRIVER, [](at_funcation* at){
        APPATHandler* atHandler = (APPATHandler*)(at->arg);
        atHandler->cmdExeNewDriverHandle(at);
    }, this);

    /* Register new driver setup command */
    at_setup_attach_cb(AT_NEW_DRIVER, [](at_funcation* at, char *data){
        APPATHandler* atHandler = (APPATHandler*)(at->arg);
        atHandler->cmdSetupNewDriverHandle(at, data);
    }, this);

    /* Register drowsiness exe command */
    at_exe_attach_cb(AT_DROWSINESS, [](at_funcation* at){
        APPATHandler* atHandler = (APPATHandler*)(at->arg);
        atHandler->cmdExeDrowsinessHandle(at);
    }, this);

    /* Register no driver exe command */
    at_exe_attach_cb(AT_NO_DRIVER, [](at_funcation* at){
        APPATHandler* atHandler = (APPATHandler*)(at->arg);
        atHandler->cmdExeNoDriverHandle(at);
    }, this);

    cleanPort();

    /* Ticker in main-loop will handle the refresh() function */
    ticker_attach_ms(&_tickerAtCmdCapture, AT_CMD_POLLING_HANDLER_MS, [](void* arg){
        APPATHandler* atHandler = (APPATHandler*)(arg);
        at_device_handle(&atHandler->_atCmdHandler);
    }, this);
}

/** Handle setup question command
 * Receive: <tagCode>,<CRC>
 * Resp: \r\n+QUESTION:<SPACE><TagCode_encrypt>,<status>,<CRC>\r\n
 * status=0: OK
 * status=1: FAIL
 *
 * Example: set question 07AT+QUESTION=12345,31$0D
 * Resp OK:    \r\n+QUESTION: 8821,0,27\r\n\r\nOK\r\n
 * Resp ERROR: \r\n+QUESTION: 8821,1,26\r\n\r\nOK\r\n
*/
void APPATHandler::cmdSetupQuestionHandle(at_funcation* at, char *data)
{
    if (!verify_and_trim_crc(at, data))
    {
      return;
    }

    constexpr uint8_t ELEMENT_NUM = 1;
    long element[ELEMENT_NUM];
    uint8_t fields;

    fields = long_strtok(data, ",", element, ELEMENT_NUM);
    APP_AT_FUNC_TAG_CONSOLE("Fields = %u", fields);
    if (fields != ELEMENT_NUM)
    {
        APP_AT_FUNC_TAG_CONSOLE("Parser Failed, expected is %u", ELEMENT_NUM);
        return;
    }

    /* Assign variable element */
    uint32_t tagCode = element[0];
    sendRespCodeOK(at, tagCode);
}

void APPATHandler::cmdExeNewDriverHandle(at_funcation* at)
{
    APP_AT_FUNC_TAG_CONSOLE("execution");
    sendRespCodeOK(at, 12345);

    if (PlayAudio.playFile(audioList[NEW_DRIVER_ID], NEW_DRIVER_REPEAT))
    {
        BuzzAlarm.statusUpdate(BuzzAlarmCycleBlinkCallbacks::BUZZ_NEW_DRIVER, ALARM_NEW_DRIVER_CNT);
        LedAlert.statusUpdate(LedAlertCycleBlinkCallbacks::LED_NEW_DRIVER, ALARM_NEW_DRIVER_CNT);
    }
}

/** Handle setup question command
 * Receive: <driver name id>
 * Resp: \r\n+NEW_DRIVER:<SPACE>,<status>,<CRC>\r\n
 * status=0: OK
 * status=1: FAIL
 *
 * Example: set question AT+NEW_DRIVER=<driver name id>$0D
 * Resp OK:    \r\n+NEW_DRIVER: 0,27\r\n\r\nOK\r\n
 * Resp ERROR: \r\n+NEW_DRIVER: 1,26\r\n\r\nOK\r\n
*/
void APPATHandler::cmdSetupNewDriverHandle(at_funcation* at, char* data)
{
    constexpr uint8_t ELEMENT_NUM = 1;
    long element[ELEMENT_NUM];
    uint8_t fields;

    fields = long_strtok(data, ",", element, ELEMENT_NUM);
    APP_AT_FUNC_TAG_CONSOLE("Fields = %u", fields);
    if (fields != ELEMENT_NUM)
    {
        APP_AT_FUNC_TAG_CONSOLE("Parser Failed, expected is %u", ELEMENT_NUM);
        return;
    }

    constexpr uint8_t DRIVER_NAME_LENGTH_MAX = 15;
    char driverNameId[DRIVER_NAME_LENGTH_MAX];
    snprintf(driverNameId, DRIVER_NAME_LENGTH_MAX, "/%u.MP3", element[0]);

    if (PlayAudio.playFile(driverNameId, NEW_DRIVER_REPEAT))
    {
        BuzzAlarm.statusUpdate(BuzzAlarmCycleBlinkCallbacks::BUZZ_NEW_DRIVER, ALARM_NEW_DRIVER_CNT);
        LedAlert.statusUpdate(LedAlertCycleBlinkCallbacks::LED_NEW_DRIVER, ALARM_NEW_DRIVER_CNT);
        sendRespCodeOK(at, 11111);
    }
    else
    {
        sendRespCode(at, 22222, atCmdStatus::STATUS_ERROR);
    }
}

void APPATHandler::cmdExeDrowsinessHandle(at_funcation* at)
{
    APP_AT_FUNC_TAG_CONSOLE("execution");
    sendRespCodeOK(at, 12345);
    
    if (PlayAudio.playFile(audioList[DROWSINESS_ID], DROWSINESS_REPEAT))
    {
        BuzzAlarm.statusUpdate(BuzzAlarmCycleBlinkCallbacks::BUZZ_ALARM, ALARM_DROWSINESS_CNT);
        LedAlert.statusUpdate(LedAlertCycleBlinkCallbacks::LED_ALERT, ALARM_DROWSINESS_CNT);
    }
}

void APPATHandler::cmdExeNoDriverHandle(at_funcation* at)
{
    APP_AT_FUNC_TAG_CONSOLE("execution");
    sendRespCodeOK(at, 12345);

    if (PlayAudio.playFile(audioList[NO_DRIVER_ID], NO_DRIVER_REPEAT))
    {
        BuzzAlarm.statusUpdate(BuzzAlarmCycleBlinkCallbacks::BUZZ_NO_DRIVER, ALARM_NO_DRIVER_CNT);
        LedAlert.statusUpdate(LedAlertCycleBlinkCallbacks::LED_NO_DRIVER, ALARM_NO_DRIVER_CNT);
    }
}

void APPATHandler::sendRespCodeOK(at_funcation* at, uint32_t tagCode)
{
    sendRespCode(at, tagCode, atCmdStatus::STATUS_OK);
}

void APPATHandler::sendRespCode(at_funcation* at, uint32_t tagCode, uint8_t status)
{
    /* the number 2 is the length of string "\r\n" */
    char* buff = (char*)malloc(AT_CMD_DYNAMIC_BUFF_SIZE + AT_CMD_CRC_LENGTH + 2 + AT_OK_LENGTH);
    snprintf(buff, AT_CMD_DYNAMIC_BUFF_SIZE, "\r\n%s: %u,%u",
                    at->cmd_name, EQUATION_CODE(tagCode), status);
    /* Calculator CRC buff */
    uint8_t crc = crc_xor((const uint8_t*)buff, strlen(buff));
    sprintf(POINTER_EOS(buff), ",%u\r\n%s", crc, at_cmd_ok);
    at->write((const uint8_t*)buff, strlen(buff));
    free(buff);
}

void APPATHandler::cleanPort()
{
    if (_cleanPort)
    {
        _cleanPort();
    }
}

void APPATHandler::onOutputPort(at_tx_cb cb)
{
    _atCmdHandler.output_cb = cb;
}

void APPATHandler::onInputPort(at_rx_cb cb)
{
    _atCmdHandler.input_cb = cb;
}

void APPATHandler::onCleanPort(void (*cb)())
{
    _cleanPort = cb;
}
