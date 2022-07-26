#include "app_at_handler.h"
#include "c_utility.h"
#include "THIoT_SerialTrace.h"

#define APP_AT_FUNC_TAG_CONSOLE(...) SERIAL_FUNCTION_TAG_LOGI("[APP_AT] ", __VA_ARGS__)

#define POINTER_EOS(x)              (x + strlen(x)) /* Pointer end of string */
#define EQUATION_CODE(x)            (((x + 6) / 7) * 5 + 1)
#define AT_CMD_DYNAMIC_BUFF_SIZE    64
#define AT_CMD_CRC_LENGTH           5
#define AT_CMD_BUFF_SIZE            128 /* 128 byte data */
#define AT_CMD_POLLING_HANDLER_MS   10


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

#if (0)
SerialHandleCallbacks::SerialHandleCallbacks(/* args */) {}
SerialHandleCallbacks::~SerialHandleCallbacks() {}

size_t SerialHandleCallbacks::onOutputPort(const uint8_t* buff, size_t length)
{
    APP_AT_FUNC_TAG_CONSOLE("[SerialHandleCallbacks]>> default <<");
    return 0;
}
size_t SerialHandleCallbacks::onInputPort(uint8_t* buff, size_t length)
{
    APP_AT_FUNC_TAG_CONSOLE("[SerialHandleCallbacks]>> default <<");
    return 0;
}
void SerialHandleCallbacks::onCleanPort()
{
    APP_AT_FUNC_TAG_CONSOLE("[SerialHandleCallbacks]>> default <<");
}
#endif
