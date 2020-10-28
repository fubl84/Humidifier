/******************************************************************************
 * @file		  main.cpp
 *
 * @creator		Christoph Seiler
 * @created		2020
 *
 * @brief  		TODO: Short description of this module
 *
 * $Id$
 *
 * $Revision$
 *
 ******************************************************************************/

/* ****************************************************************************/
/* ******************************** INCLUDES **********************************/
/* ****************************************************************************/

#include <Arduino.h>
#include <time.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>     // Display
#include <DHT.h>              // Sensor DHT22
#include <SPI.h>              // Display
#include <WiFi.h>             // WiFi
#include <FirebaseESP32.h>    // Google Firebase
#include <TFT_eSPI.h>         // Touchpanel
#include "FS.h"
#include <ESP32Servo.h>

/* ****************************************************************************/
/* *************************** DEFINES AND MACROS *****************************/
/* ****************************************************************************/

//defines for I/O pins
#define PIN_DHT               25
#define RELAY_HUMDIDIFIER     21
#define RELAY_FAN             19
#define RELAY_VENTILATOR      18
#define RELAY_HEATMAT         5 
#define PIN_WATER_TRIGGER     26
#define PIN_WATER_ECHO        27
#define PIN_SERVO             22
#define WIFI_NETWORK          "Hoppler"
#define WIFI_PASSWORD         "!12121987!09061984!"
#define WIFI_TIMEOUT_MS       20000
#define FIREBASE_HOST         "humidifieresp32.firebaseio.com"
#define FIREBASE_AUTH         "F8EsoAhdc016A00AIojgia26BsHCMinJL2hkulwB"
#define CALIBRATION_FILE      "/TouchCalibration"
#define REPEAT_CAL            false

//define NTP server
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

//define firebase data objects
FirebaseData firebaseData;    //firebase data for relais
FirebaseData firebaseData2;   //firebase data for sensors
FirebaseJsonArray arr1;       //array for relais
FirebaseJsonArray arr2;       //array for sensors
String pathRelais = "/Relais";
String pathSensors = "/Sensors";

//define servo motor
Servo myservo;

//create display
TFT_eSPI tft = TFT_eSPI();
 
//initialize DHT
#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);

//define colors and fonts for display
#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_RED   0xC000
#define COLOR_TEMP  0xB437
#define COLOR_HUM   0x359A
#define COLOR_ON    0x2DEC
#define COLOR_OFF   0x5AEB
#define COLOR_MID   0xFD20
#define COLOR_LOW   0xF800
#define COLOR_OK    0x015C

//bitmaps
// 'fan', 40x40px
const unsigned char fan [] PROGMEM = {
  0x3f, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x60, 0x00, 0x00, 0x00, 0x06, 0x67, 
	0x00, 0x3c, 0x00, 0x62, 0x6d, 0x83, 0xff, 0xc0, 0xd2, 0x69, 0x8f, 0x00, 0x70, 0x92, 0x6f, 0x38, 
	0x00, 0x1c, 0xf2, 0x60, 0x71, 0xfe, 0x06, 0x06, 0x60, 0xc7, 0x80, 0x43, 0x06, 0x61, 0x8c, 0x3c, 
	0x01, 0x86, 0x63, 0x18, 0xff, 0x00, 0xc6, 0x63, 0x30, 0xc1, 0x80, 0x66, 0x66, 0x60, 0x80, 0x80, 
	0x66, 0x64, 0xc0, 0x80, 0x80, 0x36, 0x6c, 0xc0, 0x81, 0x80, 0x32, 0x6d, 0x8e, 0xc1, 0x81, 0x92, 
	0x69, 0xbf, 0xc3, 0xfc, 0x9a, 0x69, 0x30, 0xef, 0xc6, 0xda, 0x79, 0x20, 0x7e, 0x02, 0xde, 0x79, 
	0x60, 0x26, 0x02, 0xce, 0x79, 0x60, 0x66, 0x02, 0xce, 0x79, 0x60, 0x3e, 0x06, 0xde, 0x69, 0x20, 
	0xff, 0x06, 0xda, 0x69, 0xbf, 0xc3, 0xcc, 0x9a, 0x6d, 0x84, 0xc1, 0xf9, 0x92, 0x6c, 0x81, 0x81, 
	0x01, 0xb2, 0x64, 0xc1, 0x81, 0x83, 0x36, 0x66, 0x61, 0x81, 0x83, 0x26, 0x62, 0x61, 0x81, 0x86, 
	0x66, 0x63, 0x38, 0xf3, 0x0c, 0xc6, 0x61, 0x9c, 0x3e, 0x39, 0x86, 0x60, 0xc7, 0x00, 0xf3, 0x86, 
	0x60, 0x61, 0xff, 0xc7, 0x06, 0x67, 0x38, 0x3e, 0x0c, 0x72, 0x6d, 0x8e, 0x00, 0x78, 0x92, 0x6d, 
	0x87, 0xff, 0xe0, 0x92, 0x67, 0x00, 0x7f, 0x00, 0xf2, 0x60, 0x00, 0x00, 0x00, 0x06, 0x7f, 0xff, 
	0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xfc
};
// 'heatmat', 40x40px
const unsigned char heatmat [] PROGMEM = {
  0x00, 0x80, 0x10, 0x02, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0x01, 0x80, 0x20, 0x0c, 0x00, 0x01, 
	0x80, 0x20, 0x0c, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0x00, 0xc0, 0x30, 0x06, 0x00, 0x00, 0xc0, 
	0x18, 0x02, 0x00, 0x00, 0xc0, 0x30, 0x06, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0x00, 0x00, 0x20, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x7e, 0x7c, 0x7c, 0x7c, 
	0x1e, 0x7e, 0x7c, 0x7c, 0xfe, 0x18, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 
	0x00, 0x00, 0x00, 0x0f, 0x3f, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xcf, 0xcf, 0x8f, 0x80, 0x0f, 0xcf, 0xcf, 0xcf, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x70, 
	0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x09, 0xf9, 
	0xf9, 0xf9, 0xf8, 0x01, 0xf1, 0xf1, 0xf1, 0xf0
};
// 'humidifier', 40x40px
const unsigned char humidifier [] PROGMEM = {
  0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0x00, 0x08, 0x00, 0x00, 0x0f, 0x00, 0x1c, 0x00, 0x00, 0x0f, 0x80, 0x3e, 0x00, 0x00, 0x1f, 
	0x80, 0x7a, 0x00, 0x00, 0x1f, 0xc0, 0x18, 0x00, 0x00, 0x3f, 0xc0, 0x18, 0x00, 0x00, 0x3f, 0xe0, 
	0x18, 0x00, 0x00, 0x7f, 0xe0, 0x18, 0x00, 0x00, 0x7f, 0xf0, 0x18, 0x00, 0x00, 0xff, 0xf0, 0x18, 
	0x00, 0x00, 0xff, 0xf8, 0x10, 0x30, 0x01, 0xff, 0xf8, 0x30, 0x78, 0x01, 0xff, 0xfc, 0x60, 0xfe, 
	0x03, 0xff, 0xfc, 0xe0, 0x30, 0x07, 0xff, 0xff, 0x80, 0x30, 0x07, 0xff, 0xff, 0x00, 0x20, 0x0f, 
	0xff, 0xff, 0x00, 0x60, 0x0f, 0xff, 0xff, 0x80, 0x40, 0x1f, 0xff, 0xff, 0x80, 0xc0, 0x1f, 0xff, 
	0xff, 0xc1, 0x80, 0x1f, 0xff, 0xff, 0xc7, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 
	0xe0, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xe0, 
	0x00, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xc0, 0x00, 
	0x1f, 0xff, 0xff, 0x80, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x07, 
	0xff, 0xfe, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x7f, 
	0xf0, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00
};
// 'power', 40x40px
const unsigned char power [] PROGMEM = {
  0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 
	0x3f, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 
	0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xe7, 0xff, 0xf0, 0x0f, 0xfc, 0x67, 
	0x1f, 0xf0, 0x1f, 0xf0, 0xe7, 0x0f, 0xf8, 0x1f, 0xe1, 0xe7, 0xc7, 0xf8, 0x3f, 0xc7, 0xe7, 0xe3, 
	0xfc, 0x3f, 0xcf, 0xe7, 0xf1, 0xfc, 0x3f, 0x8f, 0xe7, 0xf9, 0xfe, 0x7f, 0x9f, 0xe7, 0xf8, 0xfe, 
	0x7f, 0x1f, 0xe7, 0xfc, 0xfe, 0x7f, 0x3f, 0xe7, 0xfc, 0xff, 0xff, 0x3f, 0xe7, 0xfc, 0x7f, 0xff, 
	0x3f, 0xe7, 0xfc, 0x7f, 0xff, 0x3f, 0xe7, 0xfc, 0x7f, 0xff, 0x3f, 0xff, 0xfc, 0x7f, 0x7f, 0x3f, 
	0xff, 0xfc, 0xff, 0x7f, 0x1f, 0xff, 0xfc, 0xfe, 0x7f, 0x9f, 0xff, 0xf8, 0xfe, 0x3f, 0x8f, 0xff, 
	0xf9, 0xfe, 0x3f, 0x8f, 0xff, 0xf1, 0xfc, 0x3f, 0xc7, 0xff, 0xe3, 0xfc, 0x1f, 0xe3, 0xff, 0xc7, 
	0xfc, 0x1f, 0xf0, 0xff, 0x0f, 0xf8, 0x0f, 0xf8, 0x00, 0x1f, 0xf0, 0x07, 0xfe, 0x00, 0x7f, 0xf0, 
	0x07, 0xff, 0xf7, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0x80, 0x00, 
	0x7f, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x01, 
	0xff, 0xc0, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00
};
// 'ventilator', 40x40px
const unsigned char ventilator [] PROGMEM = {
  0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 
	0x07, 0xc0, 0x1c, 0x00, 0x00, 0x0f, 0x00, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0x18, 
	0x00, 0x18, 0x00, 0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 0x38, 0x00, 
	0x70, 0x00, 0x1c, 0x38, 0x00, 0xe7, 0x80, 0x7f, 0x18, 0x00, 0xff, 0xe0, 0x7f, 0xdc, 0x01, 0xff, 
	0xf0, 0x61, 0xfe, 0x7f, 0xf0, 0x38, 0xe0, 0x7f, 0xff, 0xe0, 0x1c, 0xe0, 0x3f, 0xc3, 0xc0, 0x1c, 
	0xe0, 0x1f, 0x81, 0xe0, 0x0e, 0xe0, 0x07, 0x00, 0xe0, 0x0e, 0xe0, 0x06, 0x00, 0x60, 0x06, 0xe0, 
	0x06, 0x00, 0x70, 0x07, 0x60, 0x06, 0x00, 0x70, 0x07, 0x60, 0x06, 0x00, 0x60, 0x07, 0x70, 0x07, 
	0x00, 0xe0, 0x07, 0x70, 0x03, 0x80, 0xf8, 0x03, 0x38, 0x03, 0xc3, 0xfc, 0x07, 0x38, 0x03, 0xff, 
	0xfe, 0x07, 0x1c, 0x0f, 0xff, 0x7f, 0x87, 0x0f, 0x3f, 0x88, 0x3b, 0xee, 0x07, 0xff, 0x80, 0x18, 
	0xfe, 0x03, 0xe7, 0x00, 0x1c, 0x3c, 0x00, 0x0e, 0x00, 0x0c, 0x00, 0x00, 0x0e, 0x00, 0x0c, 0x00, 
	0x00, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0x18, 0x00, 0x1c, 0x00, 0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 
	0x38, 0x00, 0xf0, 0x00, 0x00, 0x38, 0x03, 0xe0, 0x00, 0x00, 0x1f, 0x7f, 0xc0, 0x00, 0x00, 0x1f, 
	0xff, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00
};
// 'water', 40x40px
const unsigned char water [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 
  0x00, 0x7f, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xfc, 0x00, 0x00, 0x0f, 
  0xe1, 0xfe, 0xe0, 0x00, 0x3f, 0x00, 0x3f, 0xf0, 0x00, 0x7e, 0x00, 0x0f, 0xfc, 0x00, 0xf8, 0x00, 
  0x07, 0x7f, 0x87, 0xf0, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x1e, 0x00, 0x0f, 0xff, 0x80, 0xff, 0xe0, 
  0x01, 0xfe, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x0f, 0xff, 0xfc, 0x40, 0x00, 0x1f, 0x80, 0x7e, 0xf0, 
  0x00, 0x3e, 0x00, 0x1f, 0xf8, 0x00, 0x7c, 0x00, 0x0f, 0x7e, 0x01, 0xf8, 0x00, 0x02, 0x3f, 0xff, 
  0xf0, 0x00, 0x00, 0x1f, 0xff, 0xc0, 0x7f, 0x80, 0x07, 0xff, 0x01, 0xff, 0xf0, 0x00, 0x78, 0x07, 
  0xff, 0xf8, 0x00, 0x00, 0x0f, 0xe1, 0xfe, 0xe0, 0x00, 0x1f, 0x00, 0x3f, 0xf0, 0x00, 0x7e, 0x00, 
  0x0f, 0xfc, 0x00, 0xfc, 0x00, 0x07, 0x7f, 0x87, 0xf0, 0x00, 0x00, 0x3f, 0xff, 0xe0, 0x00, 0x00, 
  0x0f, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'window', 40x40px
const unsigned char window [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x06, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xce, 
	0x00, 0x00, 0x00, 0x73, 0xc3, 0x80, 0x00, 0x01, 0xc3, 0xd8, 0xff, 0xff, 0xff, 0x1b, 0xde, 0x60, 
	0x00, 0x06, 0x7b, 0xd3, 0xc0, 0x00, 0x03, 0xcb, 0xd0, 0xc0, 0x00, 0x03, 0x0b, 0xd0, 0x40, 0x00, 
	0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd8, 0x40, 0x00, 0x02, 0x1b, 0xde, 0x40, 0x00, 0x02, 
	0x7b, 0xd3, 0xc0, 0x00, 0x03, 0xcb, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 
	0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xdf, 
	0xc0, 0x00, 0x03, 0xfb, 0xdf, 0xc0, 0x00, 0x03, 0xfb, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 
	0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 
	0x02, 0x0b, 0xd1, 0xc0, 0x00, 0x03, 0x8b, 0xdf, 0xc0, 0x00, 0x03, 0xfb, 0xd8, 0x40, 0x00, 0x02, 
	0x1b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 0xd0, 0x40, 0x00, 0x02, 0x0b, 
	0xd1, 0xc0, 0x00, 0x03, 0x8b, 0xdf, 0x40, 0x00, 0x02, 0x7b, 0xdc, 0xff, 0xff, 0xff, 0x3b, 0xc1, 
	0x80, 0x00, 0x01, 0x83, 0xc7, 0x00, 0x00, 0x00, 0xe3, 0xdf, 0xff, 0xff, 0xff, 0xfb, 0xf0, 0x00, 
	0x00, 0x00, 0x0f, 0x40, 0x00, 0x00, 0x00, 0x02
};

/* ****************************************************************************/
/* ******************************** VARIABLES *********************************/
/* ****************************************************************************/

//define states
bool statusVentilator=false;
bool statusHumidifier=false;
bool statusFan=false;
bool statusHeatmat=false;
bool statusWindow=false;
bool statusStandby=false;
bool jsonVentilator;
bool jsonHumidifier;
bool jsonFan;
bool jsonHeatmat;
bool jsonWindow;
bool jsonStandby;
bool ventilate;
bool fanDelay;
bool finishedHumidifying=true;
bool ON = LOW;
bool OFF = HIGH;

//define variables for waterlevel
long echoDuration = 0;
long waterDistance = 0;
int maxWater = 20;
int minWater = 160;
int waterPercentage;

//define limits for humditiy check -> can be changed in app or via touch
int hHighLimit = 78;
int hLowLimit = 70;
float h=0;
float t=0;

//setup timestamps and intervalls
const long timerCheckSensors = 30000;
const long timerIdleVentilation = 1800000;
const long timerActiveVentilation = 300000;
const long timerFanDelay = 40000;
const long timerSecond = 2000;
unsigned long previousCheckSensors = 0;
unsigned long previousIdleVentilation = 0;
unsigned long previousActiveVentilation = 0;
unsigned long previousFanDelay = 0;
unsigned long previousSecond = 0;
unsigned long currentTime;
int localTime = 0;

void changeRelais(String relaisItem, int PIN, bool onoff);
void updateFirebase(void);
void actionDHT(void);
void subscribeFirebase(FirebaseData &data);
void initiatePins(void);
void initiateDisplay(void);
void updateDisplay(void);
void standby(bool activation);
void updateDHT(void);
void connectWiFi(void);
void initiateFirebase(void);
void getWaterlevel(void);
void updateFirebaseSensors(void);
void touchCalibrate(void);
void touchscreen(void);
void windowAction(bool openclose);
void getLocalTime(void);

void setup()
{
  Serial.begin(115200);

  connectWiFi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  initiateFirebase();

  initiatePins();

  dht.begin();

  tft.init();
  tft.setRotation(3);
  touchCalibrate();
  initiateDisplay();

  updateFirebase();

  myservo.attach(PIN_SERVO);
}

void loop()
{
  currentTime = millis();

  //touchscreen();

  if((statusStandby == false) || (waterPercentage >= 15))
  {
    if((currentTime - previousCheckSensors) > timerCheckSensors)
    {
      previousCheckSensors = currentTime;
      getWaterlevel();

      updateDHT();

      actionDHT();

      updateFirebaseSensors();
    }
    else{}
    //fan delay
    if((fanDelay == true) && (currentTime - previousFanDelay) > timerFanDelay)
    {
      changeRelais("Fan", RELAY_FAN, OFF);
      updateFirebase();
      fanDelay = false;
    }
    else{}
    //ventilation cycle
    if((finishedHumidifying == true) && (currentTime - previousIdleVentilation) > timerIdleVentilation)
    {
      changeRelais("Ventilator", RELAY_VENTILATOR, ON);
      updateFirebase();
      finishedHumidifying = false;
      ventilate = true;
      previousActiveVentilation = currentTime;
    }
    else{}
    //ventilation timer
    if((ventilate == true) && (currentTime - previousActiveVentilation) > timerActiveVentilation)
    {
      changeRelais("Ventilator", RELAY_VENTILATOR, OFF);
      updateFirebase();
      finishedHumidifying = true;
      ventilate = false;
      previousIdleVentilation = currentTime;
    }
    else{}
  }
  else
  {
    if(digitalRead(RELAY_HUMDIDIFIER) == ON)
    {
      changeRelais("Ventilator", RELAY_VENTILATOR, OFF);
      changeRelais("Humidifier", RELAY_HUMDIDIFIER, OFF);
      changeRelais("Fan", RELAY_FAN, OFF);
      Serial.println("Standby aktiviert!");
      updateFirebase();
    }
    else{}
    if((currentTime - previousCheckSensors) > timerCheckSensors)
    {
      previousCheckSensors = currentTime;
      getWaterlevel();

      updateDHT();

      updateFirebaseSensors();
    }
    else{}
  }

  updateDisplay();

  subscribeFirebase(firebaseData);

}

//////////////////////////////////////////////////
/////////////////LOOP FUNCTIONS///////////////////
//////////////////////////////////////////////////

void actionDHT(void)
{
  if(digitalRead(RELAY_HUMDIDIFIER) == ON)
  {
    if(h > hHighLimit)
    {
      changeRelais("Ventilator", RELAY_VENTILATOR, OFF);
      changeRelais("Humidifier", RELAY_HUMDIDIFIER, OFF);
      previousFanDelay = currentTime;
      previousIdleVentilation = currentTime;
      fanDelay = true;
      finishedHumidifying = true;
      Serial.println("Finished humidifying - turning off!");
      Serial.println("Timer set for fan delay & scheduled ventilation!");
      updateFirebase();
    }
    else
    {
      Serial.println("Humidity not reached - keeping everything on!");
    }
  }
  else
  {
    if(h < hLowLimit)
    {
      finishedHumidifying = false;
      changeRelais("Ventilator", RELAY_VENTILATOR, ON);
      changeRelais("Humidifier", RELAY_HUMDIDIFIER, ON);
      changeRelais("Fan", RELAY_FAN, ON);
      Serial.println("Humidity low - turning everything on!");
      updateFirebase();
    }
    else
    {
      Serial.println("Humidity ok - keeping everything off!");
    }
  }
  
}

void changeRelais(String relaisItem, int PIN, bool onoff)
{
  digitalWrite(PIN, onoff);
  if(relaisItem == "Ventilator")
  {
    if(onoff == HIGH)
    {
      statusVentilator = false;
      tft.fillRoundRect(16, 92, 60, 60, 10, COLOR_OFF);
      tft.drawBitmap(26, 102, ventilator, 40, 40, COLOR_WHITE);
    }
    else
    {
      statusVentilator = true;
      tft.fillRoundRect(16, 92, 60, 60, 10, COLOR_ON);
      tft.drawBitmap(26, 102, ventilator, 40, 40, COLOR_WHITE);
    }
  }
  else if(relaisItem == "Humidifier")
  {
    if(onoff == HIGH)
    {
      statusHumidifier = false;
      tft.fillRoundRect(92, 92, 60, 60, 10, COLOR_OFF);
      tft.drawBitmap(102, 102, humidifier, 40, 40, COLOR_WHITE);
    }
    else
    {
      statusHumidifier = true;
      tft.fillRoundRect(92, 92, 60, 60, 10, COLOR_ON);
      tft.drawBitmap(102, 102, humidifier, 40, 40, COLOR_WHITE);
    }
  }
  else if(relaisItem == "Fan")
  {
    if(onoff == HIGH)
    {
      statusFan = false;
      tft.fillRoundRect(168, 92, 60, 60, 10, COLOR_OFF);
      tft.drawBitmap(178, 102, fan, 40, 40, COLOR_WHITE);
    }
    else
    {
      statusFan = true;
      tft.fillRoundRect(168, 92, 60, 60, 10, COLOR_ON);
      tft.drawBitmap(178, 102, fan, 40, 40, COLOR_WHITE);
    }
  }
  else if(relaisItem == "Heatmat")
  {
    if(onoff == HIGH)
    {
      statusHeatmat = false;
      tft.fillRoundRect(16, 168, 85, 60, 10, COLOR_OFF);
      tft.drawBitmap(38, 178, heatmat, 40, 40, COLOR_WHITE);
    }
    else
    {
      statusHeatmat = true;
      tft.fillRoundRect(16, 168, 85, 60, 10, COLOR_MID);
      tft.drawBitmap(38, 178, heatmat, 40, 40, COLOR_WHITE);
      getLocalTime();
      updateFirebase();
    }
  }
  else if(relaisItem == "Window")
  {
    if(onoff == HIGH)
    {
      statusWindow = false;
      tft.fillRoundRect(117, 168, 85, 60, 10, COLOR_OFF);
      tft.drawBitmap(139, 178, window, 40, 40, COLOR_WHITE);
    }
    else
    {
      statusWindow = true;
      tft.fillRoundRect(117, 168, 85, 60, 10, COLOR_HUM);
      tft.drawBitmap(139, 178, window, 40, 40, COLOR_WHITE);
    }
  }
  else
  {
    Serial.println("Error - could not identify declared item.");
  }
  
  Serial.print("Changed PIN ");
  Serial.print(PIN);
  Serial.print(" to ");
  Serial.println(onoff);
}

void standby(bool activation)
{
  if(activation == true)
  {
    statusStandby = true;
    tft.fillRoundRect(218, 168, 85, 60, 10, COLOR_RED);
    tft.drawBitmap(240, 178, power, 40, 40, COLOR_WHITE);
  }
  else
  {
    statusStandby = false;
    tft.fillRoundRect(218, 168, 85, 60, 10, COLOR_OFF);
    tft.drawBitmap(240, 178, power, 40, 40, COLOR_WHITE);
  }
  
}

void updateFirebase(void)
{
  arr1.clear();
  arr1.set("/[0]", statusVentilator);
  arr1.set("/[1]", statusHumidifier);
  arr1.set("/[2]", statusFan);
  arr1.set("/[3]", statusHeatmat);
  arr1.set("/[4]", statusWindow);
  arr1.set("/[5]", statusStandby);
  arr1.set("/[6]", localTime);
  if(Firebase.set(firebaseData, pathRelais, arr1))
  {
    Serial.println("Firebase relais updated!");
  }
  else
  {
    Serial.println("Firebase status update failed!");
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

void updateFirebaseSensors(void)
{
  arr2.clear();
  arr2.set("/[0]", h);
  arr2.set("/[1]", t);
  arr2.set("/[2]", waterPercentage);
  arr2.set("/[3]", hHighLimit);
  arr2.set("/[4]", hLowLimit);
  if(Firebase.set(firebaseData2, pathSensors, arr2))
  {
    Serial.println("Firebase Sensors updated!");
  }
  else
  {
    Serial.println("Firebase sensor update failed!");
    Serial.println("Reason: " + firebaseData2.errorReason());
  }
}

void subscribeFirebase(FirebaseData &data)
{
  if (Firebase.get(firebaseData, pathRelais))
  {
    FirebaseJsonArray &arr1 = data.jsonArray();
    String arrStr;
    arr1.toString(arrStr, true);
    FirebaseJsonData &jsonData = data.jsonData();
    arr1.get(jsonData, 0);
    if(jsonData.stringValue == "true")
    {
      jsonVentilator = true;
    }
    else
    {
      jsonVentilator = false;
    }
    if(jsonVentilator != statusVentilator)
    {
      if(jsonVentilator == true)
      {
        changeRelais("Ventilator", RELAY_VENTILATOR, ON);
      }
      else
      {
        changeRelais("Ventilator", RELAY_VENTILATOR, OFF);
      }
    }
    else{}
    arr1.get(jsonData, 1);
    if(jsonData.stringValue == "true")
    {
      jsonHumidifier = true;
    }
    else
    {
      jsonHumidifier = false;
    }
    if(jsonHumidifier != statusHumidifier)
    {
      if(jsonHumidifier == true)
      {
        changeRelais("Humidifier", RELAY_HUMDIDIFIER, ON);
      }
      else
      {
        changeRelais("Humidifier", RELAY_HUMDIDIFIER, OFF);
      }
    }
    else{}
    arr1.get(jsonData, 2);
    if(jsonData.stringValue == "true")
    {
      jsonFan = true;
    }
    else
    {
      jsonFan = false;
    }
    if(jsonFan != statusFan)
    {
      if(jsonFan == true)
      {
        changeRelais("Fan", RELAY_FAN, ON);
      }
      else
      {
        changeRelais("Fan", RELAY_FAN, OFF);
      }
    }
    else{}
    arr1.get(jsonData, 3);
    if(jsonData.stringValue == "true")
    {
      jsonHeatmat = true;
    }
    else
    {
      jsonHeatmat = false;
    }
    if(jsonHeatmat != statusHeatmat)
    {
      if(jsonHeatmat == true)
      {
        changeRelais("Heatmat", RELAY_HEATMAT, ON);
      }
      else
      {
        changeRelais("Heatmat", RELAY_HEATMAT, OFF);
      }
    }
    else{}
    arr1.get(jsonData, 4);
    if(jsonData.stringValue == "true")
    {
      jsonWindow = true;
    }
    else
    {
      jsonWindow = false;
    }
    if(jsonWindow != statusWindow)
    {
      if(jsonWindow == true)
      {
        windowAction(true);        
      }
      else
      {
        windowAction(false);
      }
    }
    else{}
    arr1.get(jsonData, 5);
    if(jsonData.stringValue == "true")
    {
      jsonStandby = true;
    }
    else
    {
      jsonStandby = false;
    }
    if(jsonStandby != statusStandby)
    {
      if(jsonStandby == true)
      {
        standby(true);
      }
      else
      {
        standby(false);
      }
    }
    else{}
  }
  
}

void updateDisplay(void)
{
  //Humidity and temperature
  tft.fillRoundRect(16, 16, 136, 60, 10, COLOR_HUM);
  tft.drawFloat(h, 1, 36, 26, 6);
  tft.fillRoundRect(168, 16, 136, 60, 10, COLOR_TEMP);
  tft.drawFloat(t, 1, 188, 26, 6);
  
  //Waterlevel
  if (waterPercentage >= 75)               
    {
      tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_OK);
      tft.drawBitmap(254, 102, water, 40, 40, COLOR_WHITE);
    }
    else if ((waterPercentage >= 35) && (waterPercentage < 75))
    {
      tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_MID);
      tft.drawBitmap(254, 102, water, 40, 40, COLOR_WHITE);
    }
    else
    {
      tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_LOW);
      tft.drawBitmap(254, 102, water, 40, 40, COLOR_WHITE);
      // TODO turn everything off and show warning
    }
}

void getWaterlevel(void)
{
  Serial.println("Getting Waterlevel!");
  //get waterlevel
  digitalWrite(PIN_WATER_TRIGGER, LOW);         
  delay(5);                           
  digitalWrite(PIN_WATER_TRIGGER, HIGH);        
  delay(10);                         
  digitalWrite(PIN_WATER_TRIGGER, LOW);         
  echoDuration = pulseIn(PIN_WATER_ECHO, HIGH);        
  waterDistance = (echoDuration/2) * 0.3432;
  waterPercentage = (100-(waterDistance-maxWater))*(100/(minWater-maxWater));
  Serial.print("Waterlevel: ");
  Serial.print(waterDistance);
  Serial.print(" mm or ");
  Serial.print(waterPercentage);
  Serial.println(" % ");
}

void updateDHT(void)
{
  //get temperature and humidity
  Serial.println("Updating DHT22 data!");
  h = dht.readHumidity();
  t = dht.readTemperature();
  if(isnan(t) || isnan(h))
  {
    Serial.println("DHT22 sensor error! Please check the connections and restart the device!");
  }
  else
  {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" °C");
  }
}

void touchscreen(void)
{
  uint16_t x, y;
  // See if there's any touch data for us
  if (tft.getTouch(&x, &y))
  {
    Serial.println(x);
    Serial.println(y);
    if ((y > 92) && (y <= 152)) 
    {  
      if ((x > 16) && (x <= 66)) 
      {
        Serial.println("Ventilator pressed.");
        changeRelais("Ventilator", RELAY_VENTILATOR, !digitalRead(RELAY_VENTILATOR));
      }    
      if ((x > 92) && (x <= 152)) 
      {
        Serial.println("Humidifier pressed.");
        changeRelais("Humidifier", RELAY_HUMDIDIFIER, !digitalRead(RELAY_HUMDIDIFIER));
      }
      if ((x > 168) && (x <= 228)) 
      {
        Serial.println("Fan pressed.");
        changeRelais("Fan", RELAY_FAN, !digitalRead(RELAY_FAN));
      }
      if ((x > 244) && (x <= 304)) 
      {
        Serial.println("Water pressed.");
        // TODO: waterlevel detail popup
      }
    }
    if ((y > 168) && (y <= 228))
    {
      if ((x > 16) && (x <= 101)) 
      {
        Serial.println("Heatmat pressed.");
        changeRelais("Heatmat", RELAY_HEATMAT, !digitalRead(RELAY_HEATMAT));
      }
      if ((x > 117) && (x <= 202)) 
      {
        Serial.println("Window pressed.");
        windowAction(!statusWindow);
      }
      if ((x > 218) && (x <= 303)) 
      {
        Serial.println("Standby pressed.");
        standby(!statusStandby);
      }  
    }
    delay(300);
  }
  else
  {
    Serial.println("No touch detected!");
  }
  
}

void windowAction(bool openclose)
{
  if(openclose == true)
  { 
    statusWindow = true;
    myservo.write(180);
    tft.fillRoundRect(117, 168, 85, 60, 10, COLOR_HUM);
    tft.drawBitmap(139, 178, window, 40, 40, COLOR_WHITE);
    Serial.println("Window opened!");
  }
  else
  {
    statusWindow = false;
    myservo.write(0);
    Serial.println("Window closed!");
    tft.fillRoundRect(117, 168, 85, 60, 10, COLOR_OFF);
    tft.drawBitmap(139, 178, window, 40, 40, COLOR_WHITE);
  }
  updateFirebase();
  
}

void getLocalTime(void)
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  else{}
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  localTime = ((timeinfo.tm_hour * 3600) + (timeinfo.tm_min * 60) + (timeinfo.tm_sec));
}

//////////////////////////////////////////////////
////////////////SETUP FUNCTIONS///////////////////
//////////////////////////////////////////////////

void connectWiFi(void)
{
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void initiateFirebase(void)
{
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if(!Firebase.beginStream(firebaseData, pathRelais) || !Firebase.beginStream(firebaseData, pathSensors))
  {
    Serial.println("Can't begin Firebase connection!");
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

void initiatePins(void)
{
  //define relays and turn off (for safety)
  pinMode(RELAY_HUMDIDIFIER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_VENTILATOR, OUTPUT);
  pinMode(RELAY_HEATMAT, OUTPUT);
  digitalWrite(RELAY_HUMDIDIFIER, HIGH),
  digitalWrite(RELAY_FAN, HIGH);
  digitalWrite(RELAY_VENTILATOR, HIGH);
  digitalWrite(RELAY_HEATMAT, HIGH);

  //define watersensor
  pinMode(PIN_WATER_TRIGGER, OUTPUT);
  pinMode(PIN_WATER_ECHO, INPUT);
  digitalWrite(PIN_WATER_TRIGGER, HIGH);
}

void touchCalibrate(void)
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) 
  {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) 
  {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) 
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) 
  {
    // calibration data valid
    tft.setTouch(calData);
  } 
  else 
  {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) 
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) 
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void initiateDisplay(void)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextWrap(false);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);

  //Set backgrounds for symbols
  tft.fillRoundRect(16, 92, 60, 60, 10, COLOR_OFF);  //Ventilator
  tft.fillRoundRect(92, 92, 60, 60, 10, COLOR_OFF);  //Humidifier
  tft.fillRoundRect(168, 92, 60, 60, 10, COLOR_OFF); //Fan
  tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_OFF); //Waterlevel
  tft.fillRoundRect(16, 168, 85, 60, 10, COLOR_OFF); //Heatmat
  tft.fillRoundRect(117, 168, 85, 60, 10, COLOR_OFF); //Window
  tft.fillRoundRect(218, 168, 85, 60, 10, COLOR_OFF);  //Powerbutton

  tft.drawBitmap(26, 102, ventilator, 40, 40, COLOR_WHITE);
  tft.drawBitmap(102, 102, humidifier, 40, 40, COLOR_WHITE);
  tft.drawBitmap(178, 102, fan, 40, 40, COLOR_WHITE);
  tft.drawBitmap(254, 102, water, 40, 40, COLOR_WHITE);
  tft.drawBitmap(38, 178, heatmat, 40, 40, COLOR_WHITE);
  tft.drawBitmap(139, 178, window, 40, 40, COLOR_WHITE);
  tft.drawBitmap(240, 178, power, 40, 40, COLOR_WHITE);
}