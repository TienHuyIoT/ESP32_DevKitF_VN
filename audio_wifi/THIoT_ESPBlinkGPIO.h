#ifndef __ESP_BLINK_GPIO_H
#define __ESP_BLINK_GPIO_H

#include <Arduino.h>
#include <Ticker.h>

#define BLINK_NOT_AVAILABLE -1U

typedef struct cycleTimeMs {
public:
    int firstHalf;
    int endHalf;
    cycleTimeMs(int f, int e):firstHalf(f), endHalf(e){}
} cycleTimeMs_t;

class ESPBlinkCycleCallbacks
{
public:
    ~ESPBlinkCycleCallbacks();
    virtual cycleTimeMs_t getCycle(int type);
    virtual void firstHalf();
    virtual void endHalf();
};

class ESPBlinkGPIO
{
private:
    const int _gpio;
    const bool _stLevel; // stable level
    int _firstHalf_ms;
    int _endHalf_ms;
    int _counter;
    Ticker _longTick;
    Ticker _shortTick;
    ESPBlinkCycleCallbacks* _pCallback;
    void firstHalfLevel();
    void endHalfLevel();
public:
    ESPBlinkGPIO(const int gpio = -1, const bool stLevel = LOW);
    ~ESPBlinkGPIO();
    void counterSet(int counter);
    void attach(int firstHalf_ms, int endHalf_ms, int counter = 0 /* Forever */);
    void detach();
    void statusUpdate(int source, int counter = 0);
    void setCycleCallbacks(ESPBlinkCycleCallbacks* cb);
};

#endif // __ESP_BLINK_GPIO_H
