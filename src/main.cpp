/*
Z_LDG18__NOWR01
V1 Started adding SdCard Save
*/
#define DEBUG false
#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif
 
// Include Libraries
#include <Arduino.h>
#include "SD_MMC.h"
#include "FS.h"
#include "EEPROM.h"
#include "pin_config.h"
#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include "OneButton.h" // https://github.com/mathertel/OneButton
#include "FileDefinitions.h"
#define EEPROM_SIZE 1
unsigned int FileCount= 0 ;
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
 
char Vnum[] = "01"; // Start adding SDcard Save
char filechar[31];
char datachar[100];
String fileString ;
String dataString ;

void sd_init(void) {
  int32_t x, y;
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed") ;
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card attached");
    return;
  }
  String str;
  str = "SD_MMC Card Type: ";
  if (cardType == CARD_MMC) {
    str += "MMC";
  } else if (cardType == CARD_SD) {
    str += "SD_MMCSC";
  } else if (cardType == CARD_SDHC) {
    str += "SD_MMCHC";
  } else {
    str += "UNKNOWN";
  }

  Serial.println(str);
  uint32_t cardSize = SD_MMC.cardSize() / (1024 * 1024);

  str = "SD_MMC Card Size: ";
  str += cardSize;
  Serial.println(str);

  str = "Total space: ";
  str += uint32_t(SD_MMC.totalBytes() / (1024 * 1024));
  str += "MB";
  Serial.println(str);

  str = "Used space: ";
  str += uint32_t(SD_MMC.usedBytes() / (1024 * 1024));
  str += "MB";
  Serial.println(str);
}

// Define a data structure Send
//typedef struct MVstatus_results
typedef struct
{
  int8_t MVstatus = 0;
  // 0= Ready to test, 1 = testing results, 2 = stopped Max Torque,
  // 3 = Test stopped by user, 4 = Test completed
  int MVrotation = 67;
  int MVtorque = 68;
  bool MVsendOK = true ;
  char MVserial_No[15] = "            " ;
  float MVcalFact  = 69. ;
  uint8_t MV_Mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} ;
} MVstatus_results;

// Create a structured object
MVstatus_results MV_Data;
 
 
// Callback function executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  memcpy(&MV_Data, incomingData, sizeof(MV_Data));
 //   dataString = String(MV_Data.MVtorque) + '\n';
//  strcpy(dataString,MV_Data);
    dataString =
      String(MV_Data.MVstatus) + ","
    + String(MV_Data.MVrotation) + ","
    + String(MV_Data.MVtorque) + ","
    + String(MV_Data.MVsendOK) + ","
    + String(MV_Data.MVserial_No) + ","
    + String(MV_Data.MVcalFact) + ","
    + String(MV_Data.MV_Mac[0]) + ":" + String(MV_Data.MV_Mac[1])+ ":" +String(MV_Data.MV_Mac[2])+ ":"
    + String(MV_Data.MV_Mac[3])+ ":" +String(MV_Data.MV_Mac[4])+ ":" +String(MV_Data.MV_Mac[5])
    + '\n';
    dataString.toCharArray(datachar, 100);
    appendFile(SD_MMC, filechar, datachar);
  
  
  
  debug("Data received: ");
  debug(len);
  debug(" Rotation: ");
  debug(MV_Data.MVrotation); 
//tft.drawString("LeftButton:", tft.width() / 2, tft.height() / 2 - 16);
  tft.drawNumber(MV_Data.MVrotation,tft.width() / 2, 55,4);
//  delay(100);
  debug(" Torque: ");
  debugln(MV_Data.MVtorque); 
  tft.drawNumber(MV_Data.MVtorque,tft.width() / 2, 79,4);
  //delay(100);
  /*
  debug("Float Value: ");
  debugln(MV_Data.c);
  debug("Boolean Value: ");
  debugln(MV_Data.d);
  debugln();
  */
}
 
void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  Serial.println("Hello T-dongle");
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
//    tft.drawString("[WiFi Scan]", tft.width() / 2, tft.height() / 2 );
//    tft.drawString("RightButton:", tft.width() / 2, tft.height() / 2 + 16);
    sd_init();
    delay(4000);
  EEPROM.begin(EEPROM_SIZE) ;
  FileCount = EEPROM.read(0) +1 ;
  fileString = "/MVDat" + String(FileCount) + ".csv" ;
  fileString.toCharArray(filechar, 14);

  Serial.printf("file name: %s\n", fileString);
  tft.drawString(fileString, tft.width() / 2, 2);
  delay(2000);
  EEPROM.write(0, FileCount);
  EEPROM.commit();

    dataString.toCharArray(datachar, 30); // put filename as first line of data
    writeFile(SD_MMC, filechar, datachar);

  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
 
  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);



}
 
void loop() {
 
}