/*
  Ham nay tim trong thu muc co file can khong
  return
  -
*/
bool FatfsDirectoryScan(
  unsigned int IdF,
  unsigned int *RId,
  const char *extensions,
  char *NamePath)
{
  unsigned int i = 0;
  #if  DF_DEBUG_FATFS == 1
    DBG_OUTPUT_PORT.printf("\r\nIdF: %u", IdF);
    DBG_OUTPUT_PORT.printf("\r\nExtension: %s", extensions);
  #endif
  (*RId) = 0;
  File Entry;
  while (1) {
    Entry =  Root.openNextFile();
    if (! Entry) {
      // no more files
      #if  DF_DEBUG_FATFS == 1
            DBG_OUTPUT_PORT.printf("\r\nNo more files");
      #endif
      // return to the first file in the directory
      Root.rewindDirectory();
      break;
    }
    if (! strstr(Entry.name(), extensions)) {
      continue;
    }
    if (i == IdF) {
      strcpy(NamePath, Entry.name());
      #if  DF_DEBUG_FATFS == 1
            DBG_OUTPUT_PORT.printf("\r\nFile %u is %s", i, NamePath);
      #endif
      Root.rewindDirectory();
      return true;
    }
    #if  DF_DEBUG_FATFS == 1
        DBG_OUTPUT_PORT.printf("\r\nIdF: %u", i);
    #endif
    i++;
    (*RId) = i;
  }
  return false;
}
