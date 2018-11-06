#define Df_LimitNumByte     30
#define Df_EndOfChar1       '\r'
#define Df_EndOfChar2       '#'

char CmdErrStr[] = "\r\nERROR\r\n";
char CmdOkStr[] = "\r\nOK\r\n";

enum{
    Df_CmdAT=0,
    Df_CmdReset,
    Df_CmdNext,
    Df_CmdBack,
    Df_CmdPlay,
    Df_CmdPause,
    Df_CmdStop,
    Df_CmdVol,
  Df_CmdPfile
};
char * CmdAtStr[] = {
  "AT",
  "AT+RESET",
  "AT+NEXT",
  "AT+BACK",
  "AT+PLAY",
  "AT+PAUSE",
  "AT+STOP",
  "AT+VOL=",
  "AT+PFILE="
};

#define RX_BUFFER_SIZE0   100

char BuffAtCmd[RX_BUFFER_SIZE0 + 1];
unsigned char EnableBuffAtCmd;

char check_strncmp(char *str, char dat, unsigned char *index, unsigned char len) {
  if (str[*index] == dat) {
    (*index)++;
    if ((*index) == len) {
      *index = 0;
      return 1;
    }
  } else {
    *index = 0;
    if (str[*index] == dat) (*index)++;
  }
  return 0;
}

/*
  - str: con tro chuoi can xac dinh AT
  - leng: chieu dai chuoi khong tinh ky tu ket thuc('\r')
*/
void AtCmdPerform(char *str, unsigned char leng) {
  DBG_OUTPUT_PORT.printf("\r\nCmd: %s", str);
  unsigned int Rt;
  #define Df_PathLeng 20
  char Path[Df_PathLeng + 1];
  char *s;
  //Lenh AT
  //So sanh voi chieu dai leng cho truong hop lenh chi la AT,
  //vi luc nay leng se bang 2, neu leng khac 2 thi se sai
  if (strncmp(str, CmdAtStr[Df_CmdAT], leng) == 0) {
    Serial.printf(CmdOkStr);
    return;
  }
  //Lenh AT+RESET
  if (strncmp(str, CmdAtStr[Df_CmdReset], leng) == 0) {
    Serial.printf(CmdOkStr);
    return;
  }
  //Lenh AT+NEXT
  if (strncmp(str, CmdAtStr[Df_CmdNext], leng) == 0) {
    Serial.printf(CmdOkStr);
    VsPlayingMusic = Df_VsNext;
    return;
  }
  //Lenh AT+BACK
  if (strncmp(str, CmdAtStr[Df_CmdBack], leng) == 0) {
    Serial.printf(CmdOkStr);
    VsPlayingMusic = Df_VsBack;
    return;
  }
  //Lenh AT+PLAY
  if (strncmp(str, CmdAtStr[Df_CmdPlay], leng) == 0) {
    Serial.printf(CmdOkStr);
    if (VsPlayingMusic == Df_VsStop) {
      TaskVsAllPlay = TaskVsAllPlayInit;
    }
    VsPlayingMusic = Df_VsPlay;
    return;
  }
  //Lenh AT+PAUSE
  if (strncmp(str, CmdAtStr[Df_CmdPause], leng) == 0) {
    Serial.printf(CmdOkStr);
    VsPlayingMusic = Df_VsPause;
    return;
  }
  //Lenh AT+STOP
  if (strncmp(str, CmdAtStr[Df_CmdStop], leng) == 0) {
    Serial.printf(CmdOkStr);
    VsPlayingMusic = Df_VsStop;
    return;
  }
  //Lenh AT+VOL=100
  if (strncmp(str, CmdAtStr[Df_CmdVol], 7) == 0) {
    Serial.printf(CmdOkStr);
    Rt = atoi(str + 7);
    VsSetVolume(Rt, Rt);
    return;
  }
  //Lenh AT+PFILE="Path"\r
  if (strncmp(str, CmdAtStr[Df_CmdPfile], 9) == 0) {
    if ((s = strstr(str, "=\"")) == 0) return;
    memset(Path, 0, Df_PathLeng + 1);
    StrncpyUntilChar(Path, s + 2, '"', Df_PathLeng);
    Serial.printf(CmdOkStr);
    VsPlayUpdate(Path);
    return;
  }
  Serial.printf(CmdErrStr);
}

TIMEOUT ToAtCmd;
void AtCmdRecognize(void) {
  static unsigned char IdBuff = 0;
  static unsigned char IndexAt = 0;
  unsigned char LimitRecieve;
  char data;
  if (TimerExpired(&ToAtCmd)) {
    EnableBuffAtCmd = 0;
  }
  if (Serial.available() == 0) return;
  if (EnableBuffAtCmd == 0) {
    LimitRecieve = Df_LimitNumByte;
    while (--LimitRecieve != 0 && Serial.available()) {
      data = (char)Serial.read();
      if (check_strncmp("02AT", data, &IndexAt, 4) == 1) {
        EnableBuffAtCmd = 1;
        memset(BuffAtCmd, 0, RX_BUFFER_SIZE0 + 1);
        //Timeout nhan toi da la 200ms
        TimerUpdate(&ToAtCmd, 200);
        BuffAtCmd[0] = 'A';
        BuffAtCmd[1] = 'T';
        IdBuff = 2;
        return;
      }
    }
    return;
  }

  LimitRecieve = Df_LimitNumByte;
  while (--LimitRecieve != 0 && Serial.available()) {
    data = (char)Serial.read();
    BuffAtCmd[IdBuff++] = data;
    if (IdBuff == RX_BUFFER_SIZE0) {
      EnableBuffAtCmd = 0;
    }

    if (data == Df_EndOfChar1 || data == Df_EndOfChar2) {
      IdBuff--;
      BuffAtCmd[IdBuff] = '\0';
      EnableBuffAtCmd = 0;
      AtCmdPerform(BuffAtCmd, IdBuff);
    }
  }
}

void printnf_hex(char *str, unsigned char len) {
  unsigned char i;
  for (i = 0; i < len; i++) {
    printf("%02X", (unsigned char)str[i]);
  }
}

void putcharn(char *array, unsigned int len) {
  unsigned int i;
  for (i = 0; i < len; i++) putchar(array[i]);
}

/*Copy 1 chuoi ky tu den ky tu cuoi cung 'Chr'*/
boolean StrncpyUntilChar(char *Dest,char *Scr, char Chr, uint16_t Leng){
  uint16_t i;
  for(i=0;i<=Leng;i++){
    if(Scr[i]==Chr){
      strncpy(Dest,Scr,i);  
      Dest[i] = 0;
      return 1;
    }  
  } 
  return 0;   
}
