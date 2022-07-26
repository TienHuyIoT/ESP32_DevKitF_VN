#include <Adafruit_NeoPixel.h>
#include "led_alert.h"
#include "THIoT_SerialTrace.h"
#include "THIoT_ESPBoard.h"

#define LED_ALERT_TAG_CONSOLE(...) //SERIAL_FUNCTION_TAG_LOGI("[LED_ALERT]", __VA_ARGS__)

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 64 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, W2812_DI, NEO_GRB + NEO_KHZ800);

LedAlertCycleBlinkCallbacks::LedAlertCycleBlinkCallbacks()
{
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    pixels.clear(); // Set all pixel colors to 'off'
    pixels.setBrightness(100);
}

cycleTimeMs_t LedAlertCycleBlinkCallbacks::getCycle(int type) {
    LED_ALERT_TAG_CONSOLE("callback");
    cycleTimeMs_t cycleTime(100 /* On 100ms */, 1900 /* Off 1900ms */);
    switch (type)
    {
    case LED_NORMAL:
        cycleTime = cycleTimeMs(1000, 1000);
        _ledColor.red = 0;
        _ledColor.green = 150;
        _ledColor.blue = 0;
        break;
    case LED_ALERT:
        cycleTime = cycleTimeMs(100, 100);
        _ledColor.red = 150;
        _ledColor.green = 0;
        _ledColor.blue = 0;
        break;
    default:
        break;
    }
    return cycleTime;
}

void LedAlertCycleBlinkCallbacks::firstHalf() {
    LED_ALERT_TAG_CONSOLE("callback firstHalf");
    pixels.fill(pixels.Color(_ledColor.red, _ledColor.green, _ledColor.blue));
    pixels.show();   // Send the updated pixel colors to the hardware.
}

void LedAlertCycleBlinkCallbacks::endHalf() {
    LED_ALERT_TAG_CONSOLE("callback endHalf");
    pixels.clear();
    pixels.show();   // Send the updated pixel colors to the hardware.
}