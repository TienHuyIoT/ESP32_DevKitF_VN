## ESP wifi template

### Libraries
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - C++
- [AsyncTCP ESP32](https://github.com/me-no-dev/AsyncTCP) - C++
- [AsyncTCP ESP8266](https://github.com/me-no-dev/ESPAsyncTCP) - C++
- [asyncHTTPrequest](https://github.com/boblemaire/asyncHTTPrequest) - C++
- [EasyDDNS](https://github.com/ayushsharma82/EasyDDNS) - C++
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson/tree/5.x) - C++
- [oneButton](https://github.com/mathertel/OneButton) - C++
- [oneButton](https://github.com/mathertel/OneButton) - C++
- [ESP_VS1053_Library](https://github.com/baldram/ESP_VS1053_Library) - C++
- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) - C++

### HttpServer page
| Link | Description |
| ------ | ------ |
| /wifi.htm | setting wifi access point and station |
| /index.htm | Monitor some information system as Heap memory, temperature |
| /edit | web editor internal file system |
| /edit_sdfs | web editor SD card file system |
| Authorize default | pass: admin, user" admin |
| Pin default | 1234 |

### Http request API
| Link | Description |
| ------ | ------ |
| /get?param_wifi=restart | Restart ESP |
| /get?param_wifi=fw_version | Firmware version |

### Usage API file system editor
**Request to download file from sd card (method GET)**
  - /edit_sdfs?download=/file.txt
  - /edit_sdfs?download=/file.txt&filename=newName.txt

**Request to edit file from sd card (method GET)**
  - /edit_sdfs?edit=/file.txt

**Request to delete file from sd card (method DELETE)**
  - /edit_sdfs , path=/file.txt

**Request to download file from spiffs/LittleFS (method GET)**
  - /edit?download=/file.txt
  - /edit?download=/file.txt&filename=newName.txt

**Request to edit file from spiffs/LittleFS (method GET)**
  - /edit?edit=/file.txt

**Request to delete file from spiffs/LittleFS (method DELETE)**
  - /edit , path=/file.txt

### Usage ESP8266 Sketch Data Upload
**1. Download ESP8266LittleFS-2.6.0.zip**
  - Link: https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases
  - Unpack the file into C:\Users\PC-NAME\Documents\Arduino\hardware\esp8266com\esp8266\tools\mklittlefs\mklittlefs.exe

### Usage ESP32 Sketch Data Upload
**1. Download esp32fs.zip**
  - link: https://github.com/lorol/arduino-esp32fs-plugin/releases
  - Unpack the tool into "C:\Users\PC-NAME\Documents\Arduino\tools\ESP32FS\tool\esp32fs.jar"

**2. Download x86_64-w64-mingw32-mklittlefs-295fe9b.zip**
  - Link: https://github.com/earlephilhower/mklittlefs/releases
  - Unpack the file into C:\Users\PC-NAME\Documents\Arduino\hardware\espressif\esp32\tools\mklittlefs\mklittlefs.exe

**3. Download mkfatfs**
  - Link: https://github.com/labplus-cn/mkfatfs/releases/tag/v2.0.1
  - Unpack and rename the file into C:\Users\PC-NAME\Documents\Arduino\hardware\espressif\esp32\tools\mkfatfs\mkfatfs.exe

**4. mkspiffs.exe is already in directory**
  - C:\Users\PC-NAME\Documents\Arduino\hardware\espressif\esp32\tools\mkspiffs\mkspiffs.exe

**5. Install LITTLEFS**
  - Link: https://github.com/lorol/LITTLEFS
  - Or Manage Libraries with search "LITTLEFS"

### Sketch Data Upload
  - Restart Arduino IDE after any steps above
  - Tools --> ESP32 Sketch Data Upload --> [LITTLEFS, SPIFFS, FATFS, Erase Flash]
  - Tools --> ESP8266 LittleFS Data Upload
  - Tools --> ESP8266 Sketch Data Upload

### Tools using convert gzip
- [Online Convert gzip](https://online-converting.com/archives/convert-to-gzip/) - Select "Compress this file, output-gz"
- [App PeaZip convert gzip](https://peazip.github.io/index.html)
- [Online Convert file to array C](http://tomeko.net/online_tools/file_to_hex.php?lang=en)

### Tools usage json
- [Check json online](http://json.parser.online.fr/)
- [Json arduino assistant](https://arduinojson.org/v5/assistant/)
