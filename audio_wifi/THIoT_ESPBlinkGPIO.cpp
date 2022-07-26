#include "THIoT_ESPBlinkGPIO.h"
#include "THIoT_SerialTrace.h"

#define BLINK_GPIO_TAG_CONSOLE(...) //SERIAL_TAG_LOGI("[BLINK_GPIO]", __VA_ARGS__)

ESPBlinkCycleCallbacks::~ESPBlinkCycleCallbacks(){}

cycleTimeMs_t ESPBlinkCycleCallbacks::getCycle(int type) {
    BLINK_GPIO_TAG_CONSOLE("getCycle >> Default <<");
    return cycleTimeMs(100, 900);
}

void ESPBlinkCycleCallbacks::firstHalf()
{
    BLINK_GPIO_TAG_CONSOLE("firstHalf >> Default <<");
}

void ESPBlinkCycleCallbacks::endHalf()
{
    BLINK_GPIO_TAG_CONSOLE("endHalf >> Default <<");
}

ESPBlinkGPIO::ESPBlinkGPIO(const int gpio, const bool stLevel)
: _gpio(gpio), _stLevel(stLevel), _pCallback(nullptr)
{
    if (_gpio != BLINK_NOT_AVAILABLE)
    {
        pinMode(_gpio, OUTPUT);
        digitalWrite(_gpio, _stLevel);
    }
    _counter = 0;
}

ESPBlinkGPIO::~ESPBlinkGPIO()
{
}

void ESPBlinkGPIO::detach()
{    
    _longTick.detach();
    _shortTick.detach();

    if (_pCallback)
    {
        _pCallback->endHalf();
    }

    if (_gpio != BLINK_NOT_AVAILABLE)
    {
        digitalWrite(_gpio, _stLevel);
    }
    
    BLINK_GPIO_TAG_CONSOLE("stop");
}

void ESPBlinkGPIO::statusUpdate(int source, int counter)
{
    if (_pCallback)
    {
        cycleTimeMs_t cycle = _pCallback->getCycle(source);
        attach(cycle.firstHalf, cycle.endHalf, counter);
    }
}

void ESPBlinkGPIO::setCycleCallbacks(ESPBlinkCycleCallbacks* cb) 
{
    _pCallback = cb;
}

void ESPBlinkGPIO::attach(int firstHalf_ms, int endHalf_ms, int counter)
{
    BLINK_GPIO_TAG_CONSOLE("Start");
    if (counter)
    {
        _counter = counter;
    }
    _firstHalf_ms = firstHalf_ms;
    _endHalf_ms = endHalf_ms;
    firstHalfLevel(); // called immediately
    // make an interval timeout with total cycle
    _longTick.attach_ms<void*>(_firstHalf_ms + _endHalf_ms, []( void* arg){ 
        ((ESPBlinkGPIO*)(arg))->firstHalfLevel();
    }, this);
}

void ESPBlinkGPIO::counterSet(int counter)
{
    _counter = counter;
}

void ESPBlinkGPIO::firstHalfLevel()
{
    if (_gpio != BLINK_NOT_AVAILABLE)
    {
        digitalWrite(_gpio, !_stLevel);
    }

    if (_pCallback)
    {
        _pCallback->firstHalf();
    }

    // make an one timeout with the first haft cycle ms
    _shortTick.once_ms<void*>(_firstHalf_ms, []( void* arg){ 
        ((ESPBlinkGPIO*)(arg))->endHalfLevel();
    }, this);
}

void ESPBlinkGPIO::endHalfLevel() 
{
    if (_gpio != BLINK_NOT_AVAILABLE)
    {
        digitalWrite(_gpio, _stLevel);
    }

    if (_pCallback)
    {
        _pCallback->endHalf();
    }

    if (_counter > 0)
    {
        if (--_counter == 0)
        {
            // stop cycle blinking GPIO
            _longTick.detach();
            BLINK_GPIO_TAG_CONSOLE("end");
        }
    }
}