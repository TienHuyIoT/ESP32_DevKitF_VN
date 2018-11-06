#ifndef _DF_PORT
#define _DF_PORT

// The SPI interface for VS1053 and SD Card uses hardware SPI.
//
// Wiring:
// NodeMCU  GPIO    Pin to program  Wired to SD        Wired to VS1053
// -------  ------  --------------  ---------------     -------------------
// D0       GPIO16  16              -                   DCS
// D1       GPIO5    5              -                   CS
// D2       GPIO4    4              -                   DREQ
// D3       GPIO0    0 FLASH        CS                  -
// D4       GPIO2    2              -                   XRST
// D5       GPIO14  14 SCLK         (CLK)               SCK
// D6       GPIO12  12 MISO         DO                  MISO
// D7       GPIO13  13 MOSI         DI                  MOSI
// D8       GPIO15  15              -                   -
// D9       GPI03    3 RXD0         -                   -
// D10      GPIO1    1 TXD0         -                   -
// -------  ------  --------------  ---------------     -------------------
// GND      -        -
// VCC 3.3  -        -
// VCC 5 V  -        -
// RST      -        -

//MISO
#define     _int_miso_io      	    5
#define     _MISO_INIT()            pinMode(_int_miso_io, INPUT_PULLUP)
//MOSI
#define     _int_mosi_io      	    17
#define     _MOSI_INIT()            pinMode(_int_mosi_io, OUTPUT)
//SCK
#define     _int_sck_io      	      18
#define     _SCK_INIT()             pinMode(_int_sck_io, OUTPUT)

//VS1053-XRESET
//#define  	_xreset_io      		      21
//#define     _XRESET_INIT()         	pinMode(_xreset_io, OUTPUT),digitalWrite(_xreset_io, HIGH)
//#define     _XRESET_HIGH()     		  digitalWrite(_xreset_io, HIGH)
//#define     _XRESET_LOW() 		   	  digitalWrite(_xreset_io, LOW)
//VS1053-XCS
#define  	_xcs_io      			        21
#define     _XCS_INIT()         	  pinMode(_xcs_io, OUTPUT),digitalWrite(_xcs_io, HIGH)
#define     XCS_HIGH()     			    digitalWrite(_xcs_io, HIGH)
#define     XCS_LOW() 		   		    digitalWrite(_xcs_io, LOW)
//VS1053-XDCS
#define  	_xdsc_io      			      0
#define     _XDCS_INIT()         	  pinMode(_xdsc_io, OUTPUT),digitalWrite(_xdsc_io, HIGH)
#define     XDCS_HIGH()     		    digitalWrite(_xdsc_io, HIGH)
#define     XDCS_LOW() 		   		    digitalWrite(_xdsc_io, LOW)
//VS1053-DREQ
#define     _dreq_io      	    	  39
//#define     _DREQ_INIT()            pinMode(_dreq_io, INPUT_PULLUP)
#define     _DREQ_INIT()            pinMode(_dreq_io, INPUT)
#define     _DREQ_DETECT()          digitalRead(_dreq_io)

//MMC power
#define    _sdpower_io                22
#define     _SDPOWER_INIT()          pinMode(_sdpower_io, OUTPUT),digitalWrite(_sdpower_io, LOW)
#define     _SDPOWER_HIGH()          digitalWrite(_sdpower_io, HIGH)
#define     _SDPOWER_LOW()           digitalWrite(_sdpower_io, LOW)

//GSM RST
#define    _gsm_io                26
#define     _GSM_INIT()          pinMode(_gsm_io, OUTPUT),digitalWrite(_gsm_io, LOW)
#define     _GSM_HIGH()          digitalWrite(_gsm_io, HIGH)
#define     _GSM_LOW()           digitalWrite(_gsm_io, LOW)

#endif
