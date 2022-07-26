#ifndef __BUZZ_ALARM_H
#define __BUZZ_ALARM_H

#include <Arduino.h>
#include "THIoT_ESPBlinkGPIO.h"
#include "THIoT_SerialTrace.h"
#define BUZZ_ALARM_TAG_CONSOLE(...) SERIAL_FUNCTION_TAG_LOGI("[BUZZ_ALARM]", __VA_ARGS__)
class BuzzAlarmCycleBlinkCallbacks : public ESPBlinkCycleCallbacks
{
public:
    ~BuzzAlarmCycleBlinkCallbacks(){}
    enum : uint8_t {
        BUZZ_NORMAL = 0,
        BUZZ_ALARM
    };

    cycleTimeMs_t getCycle(int type) override {
        BUZZ_ALARM_TAG_CONSOLE("callback");
        cycleTimeMs_t cycleTime(100 /* On 100ms */, 1900 /* Off 1900ms */);
        switch (type)
        {
        case BUZZ_NORMAL:
            cycleTime = cycleTimeMs(1000, 1000);
            break;
        case BUZZ_ALARM:
            cycleTime = cycleTimeMs(100, 500);
            break;
        default:
            break;
        }
        return cycleTime;
    }
};

#endif // __BUZZ_ALARM_H