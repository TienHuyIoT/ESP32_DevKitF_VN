#include <Arduino.h>
#include <functional> // std::function
#include "THIoT_ESPConfig.h"
#include "THIoT_ESPBoard.h"
#include "THIoT_ESPBlinkGPIO.h"
#include "THIoT_SDFSClass.h"
#include "THIoT_SerialTrace.h"
#include "THIoT_ESPTimeSystem.h"
#include "THIoT_ESPWifiHandle.h"
#include "THIoT_ESPSysParams.h"
#include "THIoT_ESPWebserver.h"
#include "THIoT_WebserverURLHandle.h"
#include "THIoT_ESPEEPromParams.h"
#include "THIoT_ESPEthernet.h"
#include "THIoT_ESPWatchDogTimer.h"
#include "THIoT_ESPResetReason.h"
#include "THIoT_ESPAsyncEasyNTP.h"
#include "THIoT_ESPLogTrace.h"
#include "THIoT_FactoryButton.h"
#include "esp_led_status.h"
#include "buzz_alarm.h"
#include "led_alert.h"
#include "audio_decoder.h"
#include "platform_ticker.h"
#include "platform_ticker.h"
#include "at_cmd_handler.h"
#include "app_at_handler.h"

#define ARDUINO_RUNNING_CORE 1

/* Private macro -------------------------------------------------------------*/
#define MAIN_CONSOLE(...) SERIAL_LOGI(__VA_ARGS__)
#define MAIN_TAG_CONSOLE(...) SERIAL_TAG_LOGI("[MAIN]", __VA_ARGS__)
#define MAIN_FUNCTION_LOG(...) FS_FUNCTION_TAG_LOGI("[MAIN]", __VA_ARGS__)

/* Private variables ---------------------------------------------------------*/
ESPWebserver webServer;
#if (defined FACTORY_INPUT_PIN) && (FACTORY_INPUT_PIN != -1)
FactoryButton factorySysParams(FACTORY_INPUT_PIN);
#endif
#if (defined LED_STATUS_GPIO) && (LED_STATUS_GPIO != -1)
ESPBlinkGPIO LEDStatus(LED_STATUS_GPIO, HIGH);
#endif

ESPBlinkGPIO BuzzAlarm(BUZZ_CONTROL, LOW);
ESPBlinkGPIO LedAlert(BLINK_NOT_AVAILABLE);

AUDIOEncoder PlayAudio(SD_FS_SYSTEM);
APPATHandler atCommandHandler;

void TaskAudio(void *pvParameters);

uint32_t atOutputPortCallback(const uint8_t *s, uint32_t len)
{
  return AT_CMD_PORT.write(s, len);
}

uint32_t atInputPortCallback(uint8_t *s, uint32_t len)
{
  return AT_CMD_PORT.read(s, len);
}

void atCleanPortCallback(void)
{
  while(AT_CMD_PORT.available())
  {
    AT_CMD_PORT.read();
  }
}

void board_setup()
{
    AT_CMD_PORT.begin(38400, SERIAL_8N1, AT_UART_RX, AT_UART_TX);
    AT_CMD_PORT.printf("Hello serial1");

    SX1278_NSS_PINMODE_INIT();
    SX1278_NSS_RELEASE();    

    atCommandHandler.onCleanPort(atCleanPortCallback);
    atCommandHandler.onInputPort(atInputPortCallback);
    atCommandHandler.onOutputPort(atOutputPortCallback);
    atCommandHandler.run(at_fun_handle, AT_CMD_HANDLE_NUM);

    // initialize SPI
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    PlayAudio.begin();
    ESPWifi.onOTAStatus([](int type){
        if (ESPWifiHandle::OTA_START == type)
        {
            /* Stop Playing decoder audio to improve performance speed download firmware */
            PlayAudio.stop();
        }
    });

    BuzzAlarm.setCycleCallbacks(new BuzzAlarmCycleBlinkCallbacks());
    BuzzAlarm.statusUpdate(BuzzAlarmCycleBlinkCallbacks::BUZZ_NORMAL, 3);

    LedAlert.setCycleCallbacks(new LedAlertCycleBlinkCallbacks());
    LedAlert.statusUpdate(LedAlertCycleBlinkCallbacks::LED_NORMAL, 3);

    xTaskCreatePinnedToCore(
        TaskAudio, "TaskAudio" // A name just for humans
        ,
        10 * 1024 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        NULL, ARDUINO_RUNNING_CORE);
}

void setup()
{
#ifdef ESP32
    SERIAL_PORT.begin(SERIAL_BAUDRATE, SERIAL_8N1, -1, 1);
#elif defined(ESP8266)
    SERIAL_PORT.begin(SERIAL_BAUDRATE, SERIAL_8N1);
#endif
    MAIN_TAG_CONSOLE("\r\n\r\nbuild_time: %s %s GMT", __DATE__, __TIME__);
    MAIN_TAG_CONSOLE("==== Firmware version %u.%u.%u ====\r\n",
                     FW_VERSION_MAJOR,
                     FW_VERSION_MINOR,
                     FW_VERSION_BUILD);

    MAIN_TAG_CONSOLE("Reset reason %s", esp_reset_reason_str().c_str());

    // Enable watch dog timer
    // WDT_TIMEOUT_VALUE only effected for ESP32
    // ESP8266 refer https://techtutorialsx.com/2017/01/21/esp8266-watchdog-functions/
    wdt_enable(WDT_TIMEOUT_VALUE);

    // Make updating led status by wifi status
#if (defined LED_STATUS_GPIO) && (LED_STATUS_GPIO != -1)
    LEDStatus.setCycleCallbacks(new ESPLedCycleBlinkCallbacks());
#if (defined ETH_ENABLE) && (ETH_ENABLE == 1)
    Ethernet.onLedStatus(std::bind(&ESPBlinkGPIO::statusUpdate, &LEDStatus, std::placeholders::_1, 0));
#endif
    ESPWifi.onLedStatus(std::bind(&ESPBlinkGPIO::statusUpdate, &LEDStatus, std::placeholders::_1, 0));
    LEDStatus.statusUpdate(ESPLedCycleBlinkCallbacks::BLINK_NORMAL);
#endif

    // Load params form eeprom memory
    EEPParams.load();

    // Initialize and auto format nand memory file system
#ifdef ESP32
    NAND_FS_SYSTEM.begin(true);
#elif defined(ESP8266)
    NAND_FS_SYSTEM.begin();
#endif
    FSHandle.listDir(NAND_FS_SYSTEM, "/", 0);

#if (defined SD_CARD_ENABLE) && (SD_CARD_ENABLE == 1)
#if (defined SD_SPI_INTERFACE) && (SD_SPI_INTERFACE == 1)
    /* Init SPI first */
#ifdef ESP32
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN);
    /* Init Sd Card second*/
    HTH_sdCard.begin(SPI);
#elif defined(ESP8266)
    SPI.begin();
    /* Init Sd Card second*/
    HTH_sdCard.begin();
#endif // ESP32
#else
    // SDMMC interface only with ESP32
    HTH_sdCard.begin();
#endif // (defined SD_SPI_INTERFACE) && (SD_SPI_INTERFACE == 1)
    FSHandle.listDir(SD_FS_SYSTEM, "/", 0);
#endif // (defined SD_CARD_ENABLE) && (SD_CARD_ENABLE == 1)

    // Always initialize after NAND_FS_SYSTEM.begin();
    // Because some function of system will need some params
    // load from a file system to initial.
    ESPConfig.load(&NAND_FS_SYSTEM);

    // Must be load after ESPConfig.load()
    // Because it needs some parameters to configure time zone.
    ESPTime.load();

    // Save reset reason into log file.
    MAIN_FUNCTION_LOG("reset reason %s", esp_reset_reason_str().c_str());

    // Check starting condition for ethernet.
#if (defined ETH_ENABLE) && (ETH_ENABLE == 1)
#if (defined ETH_GPIO_ENABLE) && (ETH_GPIO_ENABLE != -1)
    ETH_GPIO_ENABLE_INIT();
    if (ETH_STATUS_IS_ON())
    {
        Ethernet.enable();
    }
    else
    {
        Ethernet.disable();
    }
#else
    Ethernet.enable();
#endif
    // If the ethernet is enabled, the Wifi will not be allowed to initialize .
    Ethernet.begin();
    ESPWifi.begin(!Ethernet.isEnable());
#else
    // Init wifi and accompanied services
    ESPWifi.begin();
#endif

    // register callback handle http request
    webServer.onUrlHandle(new WebserverURLHandle("param_wifi"));
    webServer.begin();

    // handle factory system params by hold button over 2s
#if (defined FACTORY_INPUT_PIN) && (FACTORY_INPUT_PIN != -1)
    factorySysParams.onFactory([]()
                               {
                                   ESPConfig.setDefault();
#if (defined LED_STATUS_GPIO) && (LED_STATUS_GPIO != -1)
                                   LEDStatus.statusUpdate(ESPLedCycleBlinkCallbacks::BLINK_FACTORY_SYSTEM_PARAMS);
#endif
                               });
    factorySysParams.begin();
#endif

    board_setup();
}

void TaskAudio(void *pvParameters)
{
    (void)pvParameters;

    // PlayAudio.playFile("/DAUMUA.MP3");

    for (;;) // A Task shall never return or exit.
    {
        PlayAudio.handle();
    }
}

void loop()
{
    wdt_reset();
    ESPWifi.loop();
#if (defined ETH_ENABLE) && (ETH_ENABLE == 1)
    Ethernet.loop();
#endif
    ticker_schedule_handler();
}
