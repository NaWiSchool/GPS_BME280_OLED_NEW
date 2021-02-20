/*
 *  Sketch for controlling an GPS-Module and BME280
 *  Output on OLED and SD-Card (data logger).
 *  Version: 0.1
 *  Author: M. Luetzelberger, info@raspberryblog.de
 *  Date: 2021-02-19
 *  License: CC-SA-BY-NC 4.0
 *  Changes:
 *  * 2021-02-19 initial version
 *  * 2021-02-20 add SD-Card
 *  
 *  Dependencies:
 *  https://github.com/JVKran/Forced-BME280, License: Apache-2.0
 *  https://github.com/mikalhart/TinyGPSPlus, License: unknown
 *  https://github.com/greiman/SSD1306Ascii, License: unknown
 *  
 *  The circuit:
 *  I2C devices, for both BME280 and OLED
 *  SDA - A4
 *  SCL - A5
 *  
 *  SPI - CardReader
 *  * MOSI - pin 11
 *  * MISO - pin 12
 *  * CLK - pin 13
 *  * CS - pin 10
 *  
 *  Serial - GPS
 *  TX - D3, connects to RX of GPS
 *  RX - D4, connects to TX of GPS
 */

#define DEBUG // Enable debugging output on console and OLED
#define ENLOG // Enable logging

// Libraries
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <forcedClimate.h>
#ifdef ENLOG
    #include <SPI.h>
    #include <SD.h>
#endif
#ifdef DEBUG
   #include "SSD1306AsciiWire.h"
#endif

// HW-Setup 
#define TXPin 3
#define RXPin 4
#define GPSBaud 9600
#define BaudRate 115200
#define chipSelect 10

// Objects
TinyGPSPlus gps;
SoftwareSerial softSerial(RXPin, TXPin);
ForcedClimate bme280 = ForcedClimate();
#ifdef DEBUG
    SSD1306AsciiWire oled;
#endif

 // Variable for storing data on SD-Card
String dataString;

//
void setup() {
 #ifdef DEBUG
    Serial.begin(BaudRate);
 #endif   
 softSerial.begin(GPSBaud);
 bme280.begin();
 #ifdef ENLOG
    SD.begin(chipSelect);
 #endif
 #ifdef DEBUG
    oled.begin(&Adafruit128x64, 0x3C);
    oled.setFont(Adafruit5x7);
 #endif
 #ifdef ENLOG
 dataString = F("-------------- DeviceReset -- New Measurement ---------------");
 SDwrite();
 #endif
}

//
void loop() {
 printDateTime(gps.date, gps.time); // print time in ISO format
 printFloat("Lat: ", gps.location.lat(), gps.location.isValid(), 11, 6);
 printFloat("Lon: ", gps.location.lng(), gps.location.isValid(), 11, 6);
 printFloat("Alt: ", gps.altitude.meters(), gps.altitude.isValid(), 6, 2);
 printFloat("Temp: ", bme280.getTemperatureCelcius(true),true,6,2);
 printFloat("Humi: ", bme280.getRelativeHumidity(true),true,6,2);
 printFloat("Pres: ", bme280.getPressure(true),true,6,2);
 Serial.println();
 if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring"));
}
 #ifdef ENLOG
 SDwrite();
 #endif
 smartDelay(3000);
 #ifdef DEBUG
    oled.clear();
 #endif
}

#ifdef ENLOG
void SDwrite() {
File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    #ifdef DEBUG
        Serial.println(dataString);
    #endif
    dataString = "";
  }
  else {
    #ifdef DEBUG
        Serial.println(F("Error: Cannot write datalog.txt!"));
    #endif    
  }
}
#endif

// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (softSerial.available())
      gps.encode(softSerial.read());
  } while (millis() - start < ms);
}

// Functions for console output
static void printFloat(const char *str, float val, bool valid, int len, int prec) {
  if (!valid) {
    #ifdef DEBUG
    Serial.print(F("NULL "));
    #endif
    dataString += String("NULL;");
  }
  else
  {
    #ifdef DEBUG
    int slen = strlen(str);
    for (int i=0; i<len; ++i) {
      Serial.print(i<slen ? str[i] : ' ');
      oled.print(i<slen ? str[i] : ' ');
    }
    Serial.print(val, prec);
    oled.println(val, prec);
    #endif
    dataString += String(val, prec);
    dataString += String(";");
    #ifdef DEBUG
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
    Serial.print(' ');
    #endif
  }
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t) {
  if (!d.isValid()) {
    #ifdef DEBUG
        Serial.print(F("NULL;"));
    #endif
    dataString += String(F("NULL;"));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d-%02d-%02d", d.year(), d.month(), d.day());
    #ifdef DEBUG
        Serial.print(sz);
        oled.print(sz);
    #endif    
    dataString += String(sz);
    dataString += String(F(";"));
  }
  
  if (!t.isValid()) {
    #ifdef DEBUG
        Serial.print(F("NULL;"));
    #endif
    dataString += String(F("NULL;"));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
    #ifdef DEBUG
        Serial.print(sz);
        Serial.print(F(" "));
        oled.println(sz);
    #endif    
    dataString += String(sz);
    dataString += String(F(";"));
  }
  smartDelay(0);
}
