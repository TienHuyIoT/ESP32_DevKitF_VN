#ifndef __APP_AT_HANDLER_H
#define __APP_AT_HANDLER_H

#include <Arduino.h>
#include "platform_ticker.h"
#include "at_cmd_handler.h"

#if (0)
class SerialHandleCallbacks
{
private:
    /* data */
public:
    SerialHandleCallbacks(/* args */);
    virtual ~SerialHandleCallbacks();

    virtual size_t onOutputPort(const uint8_t*, size_t);
    virtual size_t onInputPort(uint8_t*, size_t);
    virtual void onCleanPort();
};
#endif

class APPATHandler
{
public:
    typedef enum : uint8_t {
        STATUS_OK = 0,
        STATUS_ERROR
    } atCmdStatus;

private:
    at_cmd_cxt_t _atCmdHandler;
    at_buffer_t _atBuffer;
    ticker_function_handle_t _tickerAtCmdCapture;
#if (0)
    SerialHandleCallbacks* _pSerialCallbacks;
#endif
    void (*_cleanPort)();
    void cmdSetupQuestionHandle(at_funcation* at, char *data);

    void sendRespCode(at_funcation* at, uint32_t tagCode, uint8_t status);
    void sendRespCodeOK(at_funcation* at, uint32_t tagCode);
public:
    APPATHandler();
    ~APPATHandler();

    void run(at_funcation_t* tableHandler, uint8_t cmdNumber);
    void cleanPort();
    void onOutputPort(at_tx_cb cb);
    void onInputPort(at_rx_cb cb);
    void onCleanPort(void (*cb)());
#if (0)
    void onSerialHandle(SerialHandleCallbacks* pCallbacks)
    {
        if (pCallbacks != nullptr)
        {
            _pSerialCallbacks = pCallbacks;
        }
    }
#endif
};

#endif // __APP_AT_HANDLER_H