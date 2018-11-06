unsigned char TaskVsPlay = TaskVsPlayInit;
unsigned char TaskVsPlayContinue;
unsigned char VsPlayPre;
TIMEOUT VsPlayTo;

unsigned int VsPlayBuffCnt;
char *VsPointBuff;

void VsInitIo(void) {
  //_XRESET_INIT();
  _XCS_INIT();
  _XDCS_INIT();
  _DREQ_INIT();
  _MISO_INIT();
  _MOSI_INIT();
  _SCK_INIT();
}

void VsInitSpiControl(void) {
  // SPI initialization
  // SPI Type: Master
  // SPI Clock Rate: 125 kHz
  // SPI Clock Phase: Cycle Start
  // SPI Clock Polarity: Low
  // SPI Data Order: MSB First
  //SPI.begin(18,5,17,3);  //Phai luon khoi dong lai SPI
  SPI.begin(_int_sck_io,_int_miso_io,_int_mosi_io,_xcs_io);  //Phai luon khoi dong lai SPI
  //SPI.setDataMode(SPI_MODE0);
  //SPI.setBitOrder(MSBFIRST);
  //SPI.setFrequency(125000);
  SPISettings( 125000, MSBFIRST, SPI_MODE0 );
  //SPI.begin(18,5,17,21);  //Phai luon khoi dong lai SPI
}

void VsInitSpiData(void) {
  // SPI initialization
  // SPI Type: Master
  // SPI Clock Rate: 8000 kHz
  // SPI Clock Phase: Cycle Start
  // SPI Clock Polarity: Low
  // SPI Data Order: MSB First
  //SPI.begin();
  //SPI.setDataMode(SPI_MODE0);
  //SPI.setBitOrder(MSBFIRST);
  //SPI.setFrequency(8000000);
  SPISettings( 8000000, MSBFIRST, SPI_MODE0 );
  //SPI.begin(18,5,17,21);  //Phai luon khoi dong lai SPI
}

unsigned char VsSpiread(void)
{
  unsigned char x;
  x = 0;
  // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low
  x = SPI.transfer(0xFF);
  return x;
}

void VsSpiwrite(unsigned char c)
{
  // MSB first, clock low when inactive (CPOL 0), data valid on leading edge (CPHA 0)
  // Make sure clock starts low
  SPI.write(c);
}

unsigned int VsSciRead(unsigned char addr) {
  unsigned int data;
  VsInitSpiControl();
  XCS_LOW();
  XDCS_HIGH();
  VsSpiwrite(VS1053_SCI_READ);
  VsSpiwrite(addr);
  delayMicroseconds(100);
  while (!_DREQ_DETECT());
  data = VsSpiread();
  data <<= 8;
  data |= VsSpiread();
  XCS_HIGH();
  return data;
}

void VsSciWrite(unsigned char  addr, unsigned int data) {
  VsInitSpiControl();
  XCS_LOW();
  XDCS_HIGH();
  VsSpiwrite(VS1053_SCI_WRITE);
  VsSpiwrite(addr);
  VsSpiwrite(data >> 8);
  VsSpiwrite(data & 0xFF);
  while (!_DREQ_DETECT());
  XCS_HIGH();
}

void VsPlayData(char *buffer, unsigned char buffsiz) {
  //unsigned char i;
  VsInitSpiData();
  XDCS_LOW();
  XCS_HIGH();
  //for (i = 0; i < buffsiz; i++) SPI.write(buffer[i]);
  SPI.writeBytes ( (uint8_t*)buffer, buffsiz ) ;
  XDCS_HIGH();
}

void VsSetVolume(unsigned char  left, unsigned char right) {
  unsigned int v;
  v = left;
  v <<= 8;
  v |= right;
  VsSciWrite(VS1053_REG_VOLUME, v);
  v = VsSciRead(VS1053_REG_VOLUME);
#if  Df_Db_Vs1053 == 1
  DBG_OUTPUT_PORT.printf("\r\nRead Volume: %u", v);
#endif
}

void VsPlayUpdate(const char *trackname) {

  //Cap nhat ten bai hat
  //if (strncmp(trackname, "0:/", 3)) sprintf(VsFile, "0:/%s", trackname);
  //else strcpy(VsFile, trackname);
  strcpy(VsPath, trackname);
  //Neu dang mo file thi dong lai
  if (VsPlayingMusic == Df_VsPlay || VsPlayingMusic == Df_VsPause) {
    VsFile.close();
  }
  TaskVsPlay = TaskVsPlayStart;
}

bool VsStartPlayingFile(const char *trackname) {
  // VsReset playback
  VsSciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
  // resync
  VsSciWrite(VS1053_REG_WRAMADDR, 0x1e29);
  VsSciWrite(VS1053_REG_WRAM, 0);

  // re-open the file for reading:
  VsFile = SD_MMC.open(trackname);
  if (!VsFile) {
    // if the file didn't open, print an error:
    DBG_OUTPUT_PORT.println("error opening");
    return false;
  }
#if  Df_Db_Vs1053 == 1
  DBG_OUTPUT_PORT.printf("\r\nFile %s opened OK", trackname);
#endif

  // As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0
  VsSciWrite(VS1053_REG_DECODETIME, 0x00);
  //VsSciWrite(VS1053_REG_DECODETIME, 0x00);
  VsPlayingMusic = Df_VsPlay;
  //reset vi tri bo dem
  VsPlayBuffCnt = 0;
  //Khoi dong ngat ngoai 0
  //InitINT();
  return true;
}

bool VsFeedBuffer(void) {
  unsigned int fat_nbytes = 0;
  // Feed the hungry buffer! :)
  while (_DREQ_DETECT()) {
    if (VsPlayBuffCnt == 0) {
      fat_nbytes = 0;
      // Read some audio data from the SD card file
      fat_nbytes = VsFile.readBytes(FatfsBuff, df_FatfsBuffLenght);

      VsPlayBuffCnt = fat_nbytes;
      VsPointBuff = FatfsBuff;
      //kiem tra ket thuc file
      if (fat_nbytes == 0 || VsPlayingMusic != Df_VsPlay) {
        //Neu ket thuc file thi chuyen sang bai
        if (fat_nbytes == 0) VsPlayingMusic = Df_VsNext;
        /* close the file */
        VsFile.close();
        #if  DF_DEBUG_FATFS == 1
          DBG_OUTPUT_PORT.printf("\r\nFile closed OK");
          if (fat_nbytes == 0) DBG_OUTPUT_PORT.printf("\r\nNext");
        #endif
        return false;
      }
    }
    //Send data to Vsxx
    if (VsPlayBuffCnt >= VS1053_DATABUFFERLEN) {
      VsPlayData(VsPointBuff, VS1053_DATABUFFERLEN);
      VsPointBuff += VS1053_DATABUFFERLEN;
      VsPlayBuffCnt -= VS1053_DATABUFFERLEN;
    } else {
      VsPlayData(VsPointBuff, VsPlayBuffCnt);
      VsPlayBuffCnt = 0;
    }
  }
  return true;
}

unsigned long BitRateV1L3[16] = {0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 0};
unsigned long SampleRateV1[4] = {44100, 48000, 32000, 0};
unsigned int VsTimePlay(const char *trackname) {
  unsigned char i;
  unsigned int fat_nbytes;
  unsigned long j;
  unsigned long Size;
  unsigned int Time;

  VsFile = SD_MMC.open(trackname);
  if (!VsFile) {
    // if the file didn't open, print an error:
    DBG_OUTPUT_PORT.println("error opening");
    return false;
  }
  j = 100;
  VsFile.seek(df_FatfsBuffLenght * j);
  while (j < 200) {
    fat_nbytes = 0;

    fat_nbytes = VsFile.readBytes(FatfsBuff, df_FatfsBuffLenght);
    for (i = 0; i < (fat_nbytes - 2); i++) {
      // http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
      // https://en.wikipedia.org/wiki/MP3
      if (FatfsBuff[i] == 0xFF) {
        if ((FatfsBuff[i + 1] & 0xF0) == 0xF0) break;
      }
    }
    j++;
    if (i < (fat_nbytes - 2)) break;
  }
  Size = VsFile.size();
  VsFile.close();
  Time = Size * 8 / BitRateV1L3[FatfsBuff[i + 2] >> 4];
#if  Df_Db_Vs1053 == 1
  DBG_OUTPUT_PORT.printf("\r\nFile: %s", trackname);
  DBG_OUTPUT_PORT.printf("\r\nFrame Header: %02X%02X%02X%02X", FatfsBuff[i], FatfsBuff[i + 1], FatfsBuff[i + 2], FatfsBuff[i + 3]);
  DBG_OUTPUT_PORT.printf("\r\nVersion: %u", rbi(FatfsBuff[i + 1], 3));
  DBG_OUTPUT_PORT.printf("\r\nLayer: %u", 4 - ((FatfsBuff[i + 1] & 0x06) >> 1));
  DBG_OUTPUT_PORT.printf("\r\nBitrate Index: %u", FatfsBuff[i + 2] >> 4);
  DBG_OUTPUT_PORT.printf("\r\nBitrate: %lu", BitRateV1L3[FatfsBuff[i + 2] >> 4]);
  DBG_OUTPUT_PORT.printf("\r\nSampleRate index: %u", (FatfsBuff[i + 2] & 0x0F) >> 2);
  DBG_OUTPUT_PORT.printf("\r\nSampleRate: %lu", SampleRateV1[(FatfsBuff[i + 2] & 0x0F) >> 2]);
  DBG_OUTPUT_PORT.printf("\r\nTime Total: %u:%02u", Time / 60, Time % 60);
#endif
  return Time;
}

/*Ham doc thoi gian decode va frameHeader*/
unsigned int DecodeTime(void) {
  unsigned int t = VsSciRead(VS1053_REG_DECODETIME);
  unsigned long FrameHeader;
#if  Df_Db_Vs1053 == 1
  DBG_OUTPUT_PORT.printf("\r\nDecodeTime: %u", t);
#endif
  /*Test kiem tra FrameHeader MP3*/
  if (t == 1) {
    FrameHeader = VsSciRead(VS1053_REG_HDAT1);
    FrameHeader <<= 16;
    FrameHeader |= VsSciRead(VS1053_REG_HDAT0);
#if  Df_Db_Vs1053 == 1
    DBG_OUTPUT_PORT.printf("\r\nFrameHeader: %08lX", FrameHeader);
#endif
  }
  return t;
}

void TaskVsPlayProc(const char *trackname)
{
  switch (TaskVsPlay) {
    case TaskVsPlayDelay:
      if (!TimerExpired(&VsPlayTo)) break;
      TaskVsPlay = TaskVsPlayContinue;
      break;
    case TaskVsPlayInit:
      if (VsPlayPre == 0) {
        VsPlayPre = 1;
        //_XRESET_LOW();
        XCS_HIGH();
        XDCS_HIGH();
        VsInitSpiControl();
        // TODO: http://www.vlsi.fi/player_vs1011_1002_1003/modularplayer/vs10xx_8c.html#a3
        // hardware VsReset
        //_XRESET_LOW();
        TaskVsPlay = TaskVsPlayDelay;
        TaskVsPlayContinue = TaskVsPlayInit;
        TimerUpdate(&VsPlayTo, 100);
        break;
      }
      if (VsPlayPre == 1) {
        VsPlayPre = 2;
        //_XRESET_HIGH();
        XCS_HIGH();
        XDCS_HIGH();
        TaskVsPlay = TaskVsPlayDelay;
        TaskVsPlayContinue = TaskVsPlayInit;
        TimerUpdate(&VsPlayTo, 100);
        break;
      }
      if (VsPlayPre == 2) {
        VsPlayPre = 3;
        //Soft Reset
        VsSciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);
        TaskVsPlay = TaskVsPlayDelay;
        TaskVsPlayContinue = TaskVsPlayInit;
        TimerUpdate(&VsPlayTo, 100);
        break;
      }
      VsSciWrite(VS1053_REG_CLOCKF, 0x6000);
      VsSetVolume(40, 40);
      #if  Df_Db_Vs1053 == 1
        uint16_t St;
        St = VsSciRead(VS1053_REG_STATUS);
        DBG_OUTPUT_PORT.printf("\r\nVS1053_REG_STATUS: %04X", St);
        if ((St >> 4) & 0x0F) {
          DBG_OUTPUT_PORT.printf("\r\nVS1053 found");
        }
        else DBG_OUTPUT_PORT.printf("\r\nCouldn't find VS1053, do you have the right pins defined?");
      #endif
      VsPlayingMusic = Df_VsFree;
      VsPlayPre = 0;
      TaskVsPlay = TaskVsPlayFree;
      //TaskVsPlay = TaskVsPlayStart;
      break;
    case TaskVsPlayStart:
      VsTimePlay(trackname);  //Tinh thoi gian phat 1 file MP3
      //Bat dau phat file MP3
      if (!VsStartPlayingFile(trackname)) {
        TaskVsPlay = TaskVsPlayFree;
        break;
      }
      TaskVsPlay = TaskVsPlayReady;
      break;
    case TaskVsPlayReady:
      // wait till its ready for data
      if (!_DREQ_DETECT()) break;
      TaskVsPlay = TaskVsPlayFeed;
      TimerUpdate(&VsPlayTo, 100);
      #if  Df_Db_Vs1053 == 1
            DBG_OUTPUT_PORT.printf("\r\nPlay Start");
      #endif
      break;
    case TaskVsPlayFeed:
      if (VsPlayingMusic == Df_VsPause) break;
      if (VsFeedBuffer() == false) TaskVsPlay = TaskVsPlayFree;
      if (!TimerExpired(&VsPlayTo)) break;
      TimerUpdate(&VsPlayTo, 1000);
      //DecodeTime();
      break;
    case TaskVsPlayFree:

      break;
  }
}

/*
  Ham chi chay khi the nho va Vs10xx khoi dong OK
*/
int VsIdPlay;
unsigned int VsFileTotal;
void TaskVsAllPlayProc(const char *Path) {
  unsigned int T;
  //Neu Vs10xx chua khoi dong OK
  if (TaskVsPlay != TaskVsPlayFree) return;
  switch (TaskVsAllPlay) {
    case TaskVsAllPlayInit:
      FatfsDirectoryScan(0xffff, &VsFileTotal, ".MP3",FatfsPath);
      #if  Df_Db_Vs1053 == 1
        DBG_OUTPUT_PORT.printf("\r\nTotal File: %u", VsFileTotal);
      #endif
      if (VsFileTotal == 0) {
        VsIdPlay = 0;
        TaskVsAllPlay = TaskVsAllPlayStop;
        break;
      }
      VsPlayingMusic = Df_VsFree;
      TaskVsAllPlay = TaskVsAllPlayStart;
      break;

    case TaskVsAllPlayStart:
      if (VsPlayingMusic == Df_VsNext) {
        if (++VsIdPlay == VsFileTotal) VsIdPlay = 0;
      }
      if (VsPlayingMusic == Df_VsBack) {
        if (--VsIdPlay < 0) VsIdPlay = VsFileTotal - 1;
      }
      if (VsPlayingMusic == Df_VsStop) {
        TaskVsAllPlay = TaskVsAllPlayStop;
        VsIdPlay = 0;
        break;
      }
      if (FatfsDirectoryScan(VsIdPlay, &T, ".MP3", FatfsPath) == false) {
        TaskVsAllPlay = TaskVsAllPlayStop;
        #if  Df_Db_Vs1053 == 1
          DBG_OUTPUT_PORT.printf("\r\nStop");
        #endif
        break;
      }
      //Bat dau phat
      VsPlayUpdate(FatfsPath);
      break;

    case TaskVsAllPlayStop:

      break;
  }
}

void TimerUpdate(TIMEOUT *Time, unsigned long Tio) {
  Time->TSta = millis();
  Time->Tio = Tio;
  Time->En = 1;
}

unsigned char TimerExpired(TIMEOUT *Time) {
  if ((Time->En) && ((millis() - Time->TSta) >= Time->Tio)) {
    Time->En = 0;
    return 1;
  }
  return 0;
}
