#ifndef __LED_ALERT_H
#define __LED_ALERT_H

#include <Arduino.h>
#include "THIoT_ESPBlinkGPIO.h"

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} ledColor_t;

class LedAlertCycleBlinkCallbacks : public ESPBlinkCycleCallbacks
{
private:
    ledColor_t _ledColor;
public:
    ~LedAlertCycleBlinkCallbacks(){}
    LedAlertCycleBlinkCallbacks();
    enum : uint8_t {
        LED_NORMAL = 0,
        LED_ALERT,
        LED_NEW_DRIVER,
        LED_NO_DRIVER
    };

    cycleTimeMs_t getCycle(int type) override;

    void firstHalf() override;

    void endHalf() override;
};

#endif // __LED_ALERT_H