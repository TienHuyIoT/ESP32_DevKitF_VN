//#define BOARD_HAS_1BIT_SDMMC
#include <SPI.h>
#include "SD_MMC.h"
#include "Vs1053.h"
#include "define_port.h"

#define Df_Db_Vs1053    1
#define DF_DEBUG_FATFS  1

#define DBG_OUTPUT_PORT Serial
File VsFile;
File Root;

typedef struct {
  unsigned long TSta;   //Time Start
  unsigned long Tio;   //Timeout
  unsigned char En;
} TIMEOUT ;

#define df_FatPathLen   20
char FatfsPath[df_FatPathLen + 1];
#define df_FatfsBuffLenght      128
char FatfsBuff[df_FatfsBuffLenght + 1];
char *VsPath = FatfsPath;
unsigned char  VsPlayingMusic = Df_VsFree;
unsigned char TaskVsAllPlay = TaskVsAllPlayInit;

void setup()
{
  _GSM_INIT();
  VsInitIo(); 
  _SDPOWER_INIT();
  // Open serial communications and wait for port to open:  
  Serial.begin(115200, SERIAL_8N1, -1, 1);
  //DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("Initializing SD card...");
  //if (!SD_MMC.begin()) 
  if (!SD_MMC.begin((const char*)"/sdcard",true))
  {
    DBG_OUTPUT_PORT.println("initialization failed!");
    return;
  }
  DBG_OUTPUT_PORT.println("initialization done.");
  Root = SD_MMC.open("/");
}

void loop()
{
  AtCmdRecognize();
  TaskVsAllPlayProc("/");
  TaskVsPlayProc(VsPath);
}
