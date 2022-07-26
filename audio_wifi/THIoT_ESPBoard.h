#ifndef	__ESP_BOARD_H
#define __ESP_BOARD_H

/* -1: is not used */
#define LED_STATUS_GPIO         -1

/*
SPI BUS
*/
#ifdef ESP32
#define SPI_SCK_PIN     		18
#define SPI_MISO_PIN     		5
#define SPI_MOSI_PIN     		17
#define SPI_NSS_PIN				21
#define VS1053_CS               SPI_NSS_PIN
#define VS1053_DCS              0
#define VS1053_DREQ             39

#define GSM_UART_TX             33
#define GSM_UART_RX             25

#define AT_CMD_PORT             Serial1
#define AT_UART_TX              GSM_UART_TX
#define AT_UART_RX              GSM_UART_RX

#define BUZZ_CONTROL            32

#define W2812_DI                27

#define SX1278_NSS_PIN          16
#define SX1278_NSS_PINMODE_INIT()  pinMode(SX1278_NSS_PIN, OUTPUT)
#define SX1278_NSS_RELEASE()       digitalWrite(SX1278_NSS_PIN, HIGH)

#elif defined(ESP8266)
#define SPI_SCK_PIN     		14
#define SPI_MISO_PIN     		12
#define SPI_MOSI_PIN     		13
#define SPI_NSS_PIN				15

// GPIO Ethernet SPI Chip select
#define ETH_NSS_PIN             15
#endif

// GPIO SDcard SPI Chip select, don't care if the SD_CARD_ENABLE = 0
#define SD_NSS_PIN				16 
// GPIO control power of SD card, don't care if the SD_CARD_ENABLE = 0
#define SD_POWER_PIN     		22

/* GPIO to reset factory, -1 is not used */
#define FACTORY_INPUT_PIN       -1

/* GPIO to enable ethernet, don't care if the ETH_ENABLE = 0 */
#define ETH_GPIO_ENABLE         -1 /* HW 39(web485 Button2), 4(esp8266)*/
#if (ETH_GPIO_ENABLE != -1)
#define ETH_GPIO_ENABLE_INIT()  pinMode(ETH_GPIO_ENABLE, INPUT_PULLUP)
#define ETH_STATUS_IS_ON()      (digitalRead(ETH_GPIO_ENABLE) == 0)
#else
#define ETH_GPIO_ENABLE_INIT()
#define ETH_STATUS_IS_ON()    
#endif

#endif // __ESP_BOARD_H
