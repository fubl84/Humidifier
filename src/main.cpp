//For ESP32
#include <Arduino.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>     // Display
#include <DHT.h>              // Sensor DHT22
#include <SPI.h>              // Display
#include <WiFi.h>             // WiFi
#include <FirebaseESP32.h>    // Google Firebase
#include <TFT_eSPI.h>         // Touchpanel
#include "FS.h"

//defines for I/O pins
#define PIN_DHT               25
#define PIN_RELAY_HUMDIDIFIER 21
#define PIN_RELAY_FAN         19
#define PIN_RELAY_VENTILATOR  18
#define PIN_RELAY_HEATMAT     5 
#define PIN_WATER_TRIGGER     26
#define PIN_WATER_ECHO        27
#define WIFI_NETWORK          "Hoppler"
#define WIFI_PASSWORD         "!12121987!09061984!"
#define WIFI_TIMEOUT_MS       20000
#define FIREBASE_HOST         "humidifieresp32.firebaseio.com"
#define FIREBASE_AUTH         "F8EsoAhdc016A00AIojgia26BsHCMinJL2hkulwB"
#define CALIBRATION_FILE      "/TouchCalData3"
#define REPEAT_CAL            false

// Create display
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

//define colors and fonts for display
#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_RED   0xC000
#define COLOR_TEMP  0xB437
#define COLOR_HUM   0x359A
#define COLOR_ON    0x2DEC
#define COLOR_OFF   0x5AEB
#define COLOR_LOW   0xFD20
#define COLOR_MID   0xF800
#define COLOR_OK    0x015C

//bitmaps
// 'fan', 40x40px
const unsigned char fan [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 
  0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xe7, 0x01, 0xff, 0x80, 0xe7, 0xe7, 0x07, 
  0xff, 0xe0, 0xe7, 0xe7, 0x1f, 0xff, 0xf8, 0xe7, 0xe0, 0x3f, 0x9f, 0xfc, 0x07, 0xe0, 0x7e, 0x0f, 
  0x7e, 0x07, 0xe0, 0xf8, 0x0f, 0x1f, 0x07, 0xe1, 0xf0, 0x0f, 0x0f, 0x87, 0xe1, 0xff, 0x07, 0x87, 
  0x87, 0xe3, 0xff, 0xc7, 0x87, 0xc7, 0xe3, 0xff, 0xe7, 0x87, 0xc7, 0xe7, 0x83, 0xff, 0x0f, 0xe7, 
  0xe7, 0x81, 0xff, 0x1f, 0xe7, 0xe7, 0x00, 0x7f, 0x3f, 0xe7, 0xe7, 0x03, 0xff, 0xfc, 0xe7, 0xe7, 
  0x0f, 0xff, 0xfc, 0xe7, 0xe7, 0x3f, 0xff, 0xf0, 0xe7, 0xe7, 0x7f, 0xff, 0xe0, 0xe7, 0xe7, 0xfc, 
  0xfe, 0x00, 0xe7, 0xe7, 0xf8, 0xff, 0x81, 0xe7, 0xe7, 0xf0, 0xff, 0xc1, 0xe7, 0xe3, 0xe1, 0xe7, 
  0xff, 0xc7, 0xe3, 0xe1, 0xe3, 0xff, 0xc7, 0xe1, 0xe1, 0xe0, 0xff, 0x87, 0xe1, 0xf0, 0xf0, 0x0f, 
  0x87, 0xe0, 0xf8, 0xf0, 0x1f, 0x07, 0xe0, 0x7e, 0xf0, 0x7e, 0x07, 0xe0, 0x3f, 0xf9, 0xfc, 0x07, 
  0xe7, 0x1f, 0xff, 0xf8, 0xe7, 0xe7, 0x07, 0xff, 0xe0, 0xe7, 0xe7, 0x01, 0xff, 0x80, 0xe7, 0xe0, 
  0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
// 'heatmat', 40x40px
const unsigned char heatmat [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x10, 0x00, 0x00, 0x1c, 
  0x00, 0x70, 0x00, 0x00, 0x3c, 0x38, 0x70, 0x00, 0x00, 0x38, 0x38, 0x70, 0x00, 0x00, 0x3c, 0x70, 
  0x78, 0x00, 0x00, 0x1e, 0x3c, 0x3c, 0x00, 0x00, 0x0e, 0x3c, 0x1c, 0x00, 0x00, 0x0e, 0x1e, 0x3c, 
  0x00, 0x00, 0x0e, 0x1e, 0x38, 0x00, 0x00, 0x08, 0x1c, 0x10, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 
  0x7f, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 
  0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x7e, 0x00, 0x03, 0xc0, 0x00, 0xff, 0x00, 
  0x03, 0xc0, 0x00, 0xff, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'humidifier', 40x40px
const unsigned char humidifier [] PROGMEM = {
  0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x18, 0x00, 0x00, 
  0x0f, 0x80, 0x3c, 0x00, 0x00, 0x1f, 0x80, 0x7e, 0x00, 0x00, 0x3d, 0xc0, 0x7f, 0x00, 0x00, 0x39, 
  0xc0, 0x7f, 0x00, 0x00, 0x39, 0xc0, 0x7e, 0x00, 0x00, 0x70, 0xe0, 0x1c, 0x00, 0x00, 0x70, 0xf0, 
  0x1c, 0x00, 0x00, 0xf0, 0x70, 0x1c, 0x00, 0x00, 0xe0, 0x70, 0x1c, 0x00, 0x01, 0xe0, 0x38, 0x1c, 
  0x38, 0x01, 0xc0, 0x3c, 0x38, 0x7c, 0x03, 0xc0, 0x1c, 0x78, 0xfc, 0x07, 0x80, 0x1e, 0xf0, 0xfe, 
  0x07, 0x00, 0x0f, 0xf0, 0xfe, 0x0f, 0x00, 0x7f, 0xe0, 0x78, 0x0e, 0x01, 0xff, 0x80, 0x70, 0x1e, 
  0x03, 0xfb, 0x80, 0x70, 0x3c, 0x07, 0x83, 0xc0, 0xe0, 0x38, 0x0f, 0x01, 0xc1, 0xe0, 0x38, 0x0e, 
  0x01, 0xe7, 0xc0, 0x38, 0x1c, 0x03, 0xff, 0x80, 0x70, 0x1c, 0x0f, 0xff, 0x00, 0x70, 0x38, 0x1f, 
  0xfc, 0x00, 0x70, 0x38, 0x3f, 0xe0, 0x00, 0x70, 0x38, 0x38, 0x60, 0x00, 0x70, 0x00, 0x30, 0xe0, 
  0x00, 0x70, 0x00, 0x70, 0xe0, 0x00, 0x38, 0x00, 0x30, 0xe0, 0x00, 0x38, 0x00, 0x01, 0xe0, 0x00, 
  0x38, 0x00, 0x01, 0xc0, 0x00, 0x1c, 0x00, 0x01, 0xc0, 0x00, 0x1e, 0x00, 0x02, 0x00, 0x00, 0x0f, 
  0x00, 0x07, 0x00, 0x00, 0x0f, 0x80, 0x07, 0x00, 0x00, 0x07, 0xf0, 0xf8, 0x00, 0x00, 0x01, 0xff, 
  0xf8, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00
};
// 'power', 40x40px
const unsigned char power [] PROGMEM = {
  0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0xf7, 0x00, 0x00, 0x00, 0x00, 0xe7, 0x00, 0x00, 0x00, 0x7c, 0xe7, 0x3e, 0x00, 0x00, 0xfc, 
  0xe7, 0x3f, 0x00, 0x01, 0xfc, 0xe7, 0x3f, 0x80, 0x03, 0xfc, 0xe7, 0x3f, 0xc0, 0x07, 0xdc, 0xe7, 
  0x3b, 0xe0, 0x0f, 0x9c, 0xe7, 0x39, 0xf0, 0x0f, 0x3c, 0xe7, 0x3c, 0xf0, 0x1e, 0x7c, 0xe7, 0x3e, 
  0x78, 0x1e, 0xfc, 0xe7, 0x3f, 0x78, 0x3c, 0xf0, 0xe7, 0x0f, 0x3c, 0x39, 0xe0, 0xe7, 0x07, 0x9c, 
  0x79, 0xc0, 0xe7, 0x03, 0xde, 0x7b, 0xc0, 0xe7, 0x03, 0xde, 0x7b, 0xc0, 0xff, 0x03, 0xde, 0x7b, 
  0xc0, 0xff, 0x03, 0xde, 0x7b, 0xc0, 0xff, 0x03, 0xce, 0x73, 0x80, 0x7e, 0x01, 0xce, 0x7b, 0xc0, 
  0x00, 0x01, 0xce, 0x7b, 0xc0, 0x00, 0x03, 0xde, 0x7b, 0xc0, 0x00, 0x03, 0xde, 0x7b, 0xc0, 0x00, 
  0x03, 0xde, 0x7b, 0xc0, 0x00, 0x03, 0xde, 0x79, 0xe0, 0x00, 0x07, 0x9e, 0x3d, 0xf0, 0x00, 0x0f, 
  0xbc, 0x3c, 0xf8, 0x00, 0x1f, 0x3c, 0x1e, 0x7c, 0x00, 0x3e, 0x78, 0x0f, 0x3f, 0x00, 0xfc, 0xf0, 
  0x0f, 0xbf, 0xff, 0xfd, 0xf0, 0x07, 0xcf, 0xff, 0xf3, 0xe0, 0x07, 0xc7, 0xff, 0xe3, 0xe0, 0x01, 
  0xf8, 0x00, 0x1f, 0x80, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00, 0x3f, 
  0xff, 0xfc, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00
};
// 'ventilator', 40x40px
const unsigned char ventilator [] PROGMEM = {
  0x03, 0xfe, 0x00, 0x06, 0x00, 0x07, 0xfe, 0x00, 0x3f, 0xc0, 0x1f, 0xff, 0x81, 0xff, 0xf8, 0x3f, 
  0xff, 0x83, 0xff, 0xfc, 0x3f, 0xff, 0xc3, 0xff, 0xfc, 0x3f, 0xff, 0xcf, 0xff, 0xfe, 0x7f, 0xff, 
  0xcf, 0xff, 0xff, 0x7f, 0xff, 0xdf, 0xff, 0xff, 0x7f, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0x7f, 0xff, 0xbf, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xfc, 
  0x3f, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xf3, 0xf0, 0x07, 0xff, 0xe7, 0xfe, 0x00, 0x07, 
  0xff, 0xc3, 0xff, 0x80, 0x01, 0xff, 0xc3, 0xff, 0xe0, 0x00, 0x7f, 0xe7, 0xff, 0xe0, 0x0f, 0xcf, 
  0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 
  0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfd, 0xff, 
  0xfe, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xfe, 
  0xff, 0xff, 0xfb, 0xff, 0xfe, 0xff, 0xff, 0xf3, 0xff, 0xfe, 0x7f, 0xff, 0xf3, 0xff, 0xfc, 0x3f, 
  0xff, 0xc3, 0xff, 0xfc, 0x3f, 0xff, 0xc1, 0xff, 0xfc, 0x1f, 0xff, 0x81, 0xff, 0xf8, 0x03, 0xfc, 
  0x00, 0x7f, 0xe0, 0x00, 0x60, 0x00, 0x7f, 0xc0
};
// 'water', 40x40px
const unsigned char water [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 
  0x00, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x0f, 0xff, 0xfc, 0x40, 0x00, 0x1f, 
  0xff, 0xff, 0xf0, 0x00, 0x7f, 0x80, 0x7f, 0xf8, 0x00, 0x7f, 0x00, 0x3f, 0xfe, 0x03, 0xfc, 0x00, 
  0x0f, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xf0, 0x7f, 0x80, 0x1f, 0xff, 0xc3, 0xff, 0xf0, 
  0x07, 0xff, 0x87, 0xff, 0xf8, 0x00, 0x30, 0x1f, 0xff, 0xfe, 0xe0, 0x00, 0x3f, 0xe0, 0xff, 0xf8, 
  0x00, 0x7f, 0x80, 0x3f, 0xfc, 0x01, 0xfe, 0x00, 0x1f, 0xff, 0x03, 0xfc, 0x00, 0x07, 0x7f, 0xff, 
  0xf8, 0x0c, 0x00, 0x1f, 0xff, 0xe1, 0xff, 0xe0, 0x0f, 0xff, 0xc3, 0xff, 0xf8, 0x01, 0xfe, 0x0f, 
  0xff, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xfe, 0xf0, 0x00, 0x3f, 0xc0, 0x7f, 0xf8, 0x00, 0xfe, 0x00, 
  0x1f, 0xfe, 0x01, 0xfe, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x02, 0x7f, 0xff, 0xf0, 0x00, 0x00, 
  0x1f, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

long  echoDuration=0;             // duration for the echo
long  waterDistance=0;            // distance from sensor to waterlevel (mm)
int   maxWater=20;                // lowest acceptable waterlevel (mm)
int   minWater=80;               // highest acceptable waterlevel (mm)
int   timeUntilVentilation;
int   wait30 = 30000;             // time to reconnect when connection is lost
int   failCounter = 0;
float h;                          //global for humidity
float t;                          //global for temperature
char humidity[5];
char temperature[5];
String serverAnswer ="";

//initialize DHT
#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[6];

//humidity controll values
#define HIGHLIMIT 78
#define LOWLIMIT  70

//setup timestamps and intervalls
const long timerSensor=30000;
const long timerIntervallVentilation=1800000;
const long timerVentilation=300000;
const long timerFanDelay=40000;
const long timerSecond=2000;

unsigned long previousSensor=0;
unsigned long previousIntervallVentilation=0;
unsigned long previousVentilation=0;
unsigned long previousFanDelay=0;
unsigned long previousSecond=0;

//create "states"
bool ventilate;
bool fanDelay;
bool finishedHumidifying=true;
bool iconVentilator;
bool iconHumidifier;
bool iconFan;
bool iconHeatmat;

FirebaseData firebaseData;

FirebaseJson json;

void displaySetup(void);
void connectToWiFi(void);
void readDHT22(void);
void updateDisplay(void);
void getWaterlevel(void);
void checkFirebase(void);
void touchScreen(void);
void touchCalibrate(void);
void drawButtons(void);
void pinInit(void);


void setup() 
{
  //start commenting on serial port
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1);
  
  //touchCalibrate();

  displaySetup();

  //drawButtons();

  connectToWiFi();

  //setup connection to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //Firebase.setwriteSizeLimit(firebaseData, "small");
/* PROBABLY NOT NEEDED?
  Firebase.setString(firebaseData, "/statusVentilator", "0");
  Firebase.setString(firebaseData, "/statusHumidifier", "0");
  Firebase.setString(firebaseData, "/statusFan", "0");
  Firebase.setString(firebaseData, "/statusHeatmat", "0");
  Firebase.setString(firebaseData,"/iconVentilator", "0");
  Firebase.setString(firebaseData,"/iconHumidifier", "0");
  Firebase.setString(firebaseData,"/iconFan", "0");
  Firebase.setString(firebaseData,"/iconHeatmat", "0");
*/
  pinInit();

  //start DHT sensor
  dht.begin();

}

void loop() 
{
  unsigned long currentTime = millis();

  //touchScreen();
  
  if(currentTime - previousSecond >= timerSecond) //every 2 Seconds
  {
    //check the database
    previousSecond = currentTime;

    checkFirebase();

    updateDisplay();
  }
  else{}

  if(currentTime - previousSensor >= timerSensor) //every 30 Seconds
  {
    getWaterlevel();

    readDHT22();

    //check status and turn on / turn off / ignore
    if(digitalRead(PIN_RELAY_HUMDIDIFIER) == LOW)
    {
        Serial.println("PART 1 LOOP OK");
        if(h >= HIGHLIMIT)
        {
            digitalWrite(PIN_RELAY_HUMDIDIFIER, HIGH);
            Firebase.setString(firebaseData,"/statusHumidifier", "0");
            iconHumidifier=true;
            digitalWrite(PIN_RELAY_VENTILATOR, HIGH);
            Firebase.setString(firebaseData,"/statusVentilator", "0");
            iconVentilator=true;
            previousIntervallVentilation=currentTime;
            previousFanDelay=currentTime;
            finishedHumidifying=true;
            fanDelay=true;
            Serial.println("HUMIDITY OK / TURN OFF");
        }
        else{Serial.println("PART 1 FINISHED, RELAIS ON");}
      }
    else
    {
      Serial.println("PART 2 LOOP OK");
      if(h <= LOWLIMIT)
      {
          digitalWrite(PIN_RELAY_HUMDIDIFIER, LOW);
          Firebase.setString(firebaseData,"/statusHumidifier", "1");
          iconHumidifier=true;
          digitalWrite(PIN_RELAY_FAN, LOW);
          Firebase.setString(firebaseData,"/statusFan", "1");
          iconFan=true;
          digitalWrite(PIN_RELAY_VENTILATOR, LOW);
          Firebase.setString(firebaseData,"/statusVentilator", "1");
          iconVentilator=true;
          finishedHumidifying=false;
          Serial.println("HUMIDITY LOW / TURN ON");
      }
      else{}
    }
      previousSensor=currentTime;
      Serial.println("Set new 30S Timer!");
  }
  else{}
  
  //keep the fogfan on for 40 seconds
  if((fanDelay == true) && (currentTime - previousFanDelay >= timerFanDelay))
  {
      Serial.println("FAN DELAY LOOP OK!");
      digitalWrite(PIN_RELAY_FAN, HIGH);
      Firebase.setString(firebaseData,"/statusFan", "0");
      iconFan=true;
      fanDelay=false;
  }
  else{}
  
  //when the 30 minutes after humidifying are finished / scheduled ventilation 30 minutes
  if((finishedHumidifying == true) && (currentTime - previousIntervallVentilation >= timerIntervallVentilation))
  {
      Serial.println("30 MINUTES VENT LOOP OK!");
      digitalWrite(PIN_RELAY_VENTILATOR, LOW);
      Firebase.setString(firebaseData,"/statusVentilator", "1");
      iconVentilator=true;
      previousVentilation=currentTime;
      ventilate=true;
      finishedHumidifying=false;
  }
  else{}
  
  //when 5 minutes of ventilation have passed
  if((ventilate == true) && (currentTime - previousVentilation >= timerVentilation))
  {
      Serial.println("5 MIN VENTILATION LOOP OK!");
      digitalWrite(PIN_RELAY_VENTILATOR, HIGH);
      Firebase.setString(firebaseData,"/statusVentilator", "0");
      iconVentilator=true;
      previousIntervallVentilation=currentTime;
      ventilate=false;
      finishedHumidifying=true;
  }
  else{}

}

void displaySetup(void)
{
  tft.fillScreen(ILI9341_BLACK);     // Fill screen with black
  tft.setTextWrap(false);           //disable text overflow

  //Set backgrounds for symbols
  tft.fillRoundRect(16, 92, 60, 60, 10, COLOR_OFF);  //Ventilator
  tft.fillRoundRect(92, 92, 60, 60, 10, COLOR_OFF);  //Humidifier
  tft.fillRoundRect(168, 92, 60, 60, 10, COLOR_OFF); //Fan
  tft.fillRoundRect(16, 168, 136, 60, 10, COLOR_OFF); //Heatmat
  tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_OFF); //Waterlevel
  tft.fillRoundRect(168, 168, 136, 60, 10, COLOR_RED);  //Powerbutton

  //place symbols
  tft.drawBitmap(26, 102, ventilator, 40, 40, COLOR_BLACK);
  tft.drawBitmap(102, 102, humidifier, 40, 40, COLOR_BLACK);
  tft.drawBitmap(178, 102, fan, 40, 40, COLOR_BLACK);
  tft.drawBitmap(64, 178, heatmat, 40, 40, COLOR_BLACK);
  tft.drawBitmap(218, 178, power, 40, 40, COLOR_WHITE);

}

void readDHT22(void)
{
  //get temperature and humidity and smooth values
  Serial.println("Updating DHT22 Data!");
  h = dht.readHumidity();
  t = dht.readTemperature();
  if(isnan(t) || isnan(h))
  {
    Serial.println("Failed reading DHT22");
    if(failCounter > 15)
    {
      ESP.restart();
    }
    else
    {
      failCounter++;
    }
    
  }
  else
  { 
    failCounter = 0; 
    Serial.println(h);
    Serial.println(t);
    dtostrf(h, 1, 0, humidity);
    dtostrf(t, 1, 0, temperature);
  }
}

void updateDisplay(void)
{
  //update icons?
  if((digitalRead(PIN_RELAY_VENTILATOR) == HIGH) && (iconVentilator == true))
  {
    tft.fillRoundRect(16, 92, 60, 60, 10, COLOR_OFF);
    tft.drawBitmap(26, 102, ventilator, 40, 40, COLOR_BLACK);
    Firebase.setString(firebaseData,"/iconVentilator", "0");
    iconVentilator = false;
  }
  else if((digitalRead(PIN_RELAY_VENTILATOR) == LOW) && (iconVentilator == true))
  {
    tft.fillRoundRect(16, 92, 60, 60, 10, COLOR_ON);
    tft.drawBitmap(26, 102, ventilator, 40, 40, COLOR_WHITE);
    Firebase.setString(firebaseData,"/iconVentilator", "1");
    iconVentilator = false;
  }
  else{}
  
  if((digitalRead(PIN_RELAY_HUMDIDIFIER) == HIGH) && (iconHumidifier == true))
  {
    tft.fillRoundRect(92, 92, 60, 60, 10, COLOR_OFF);
    tft.drawBitmap(102, 102, humidifier, 40, 40, COLOR_BLACK);
    Firebase.setString(firebaseData,"/iconHumidifier", "0");
    iconHumidifier = false;
  }
  else if((digitalRead(PIN_RELAY_HUMDIDIFIER) == LOW) && (iconHumidifier == true))
  {
    tft.fillRoundRect(92, 92, 60, 60, 10, COLOR_ON);
    tft.drawBitmap(102, 102, humidifier, 40, 40, COLOR_WHITE);
    Firebase.setString(firebaseData,"/iconHumidifier", "1");
    iconHumidifier = false;
  }
  else{}

  if((digitalRead(PIN_RELAY_FAN) == HIGH) && (iconFan == true))
  {
    tft.fillRoundRect(168, 92, 60, 60, 10, COLOR_OFF);
    tft.drawBitmap(178, 102, fan, 40, 40, COLOR_BLACK);
    Firebase.setString(firebaseData,"/iconFan", "0");
    iconFan = false;
  }
  else if((digitalRead(PIN_RELAY_FAN) == LOW) && (iconFan == true))
  {
    tft.fillRoundRect(168, 92, 60, 60, 10, COLOR_ON);
    tft.drawBitmap(178, 102, fan, 40, 40, COLOR_WHITE);
    Firebase.setString(firebaseData,"/iconFan", "1");
    iconFan = false;
  }
  else{}

  if((digitalRead(PIN_RELAY_HEATMAT) == HIGH) && (iconHeatmat == true))
  {
    tft.fillRoundRect(16, 168, 136, 60, 10, COLOR_OFF); //Heatmat
    tft.drawBitmap(64, 178, heatmat, 40, 40, COLOR_BLACK);
    Firebase.setString(firebaseData,"/iconHeatmat", "0");
    iconHeatmat = false;
  }
  else if((digitalRead(PIN_RELAY_HEATMAT) == LOW) && (iconHeatmat == true))
  {
    tft.fillRoundRect(16, 168, 136, 60, 10, COLOR_LOW); //Heatmat
    tft.drawBitmap(64, 178, heatmat, 40, 40, COLOR_WHITE);
    Firebase.setString(firebaseData,"/iconHeatmat", "1");
    iconHeatmat = false;
  }
  else{}
  
  //Global font settings
  tft.setTextColor(ILI9341_WHITE);   // Set color of text. First is the color of text and after is color of background
  tft.setTextSize(1);
  
  //Humidity and temperature
  tft.fillRoundRect(16, 16, 136, 60, 10, COLOR_HUM);
  tft.drawFloat(h, 1, 36, 26, 6);
  tft.fillRoundRect(168, 16, 136, 60, 10, COLOR_TEMP);
  tft.drawFloat(t, 1, 188, 26, 6);
  
  //Waterlevel
  if (waterDistance >= 65)               
    {
      tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_LOW);
      tft.drawBitmap(254, 102, water, 40, 40, COLOR_WHITE);
      //return; // TODO :  WARNING AND RESTART FROM BEGINNING
    }
    else if (waterDistance >= 45 && waterDistance  < 65)
    {
      tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_MID);
      tft.drawBitmap(254, 102, water, 40, 40, COLOR_WHITE);
    }
    else
    {
      tft.fillRoundRect(244, 92, 60, 60, 10, COLOR_OK);
      tft.drawBitmap(254, 102, water, 40, 40, COLOR_WHITE);
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
    Serial.println("Waterlevel done!");
    Serial.println(waterDistance);
 }

 void connectToWiFi(void)
{
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while(WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime < WIFI_TIMEOUT_MS))
  {
    Serial.print(".");
    delay(100);
  }

  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println(" Failed!");
    //take action maybe restart?
  }
  else
  {
    Serial.println("Connected!");
    Serial.println(WiFi.localIP());
  }
}

void checkFirebase(void)
{
  Firebase.setFloat(firebaseData, "/Humidity", h);
  Firebase.setFloat(firebaseData, "/Temperature", t);
  Firebase.setInt(firebaseData, "/Waterlevel", waterDistance);

  if(Firebase.getString(firebaseData,"/userInput"))
  {
    if(firebaseData.stringData() == "true")
    {
      Firebase.setString(firebaseData, "/userInput", "false");

      if(Firebase.getString(firebaseData,"/statusVentilator"))
      {
        if((firebaseData.stringData() == "1") && (digitalRead(PIN_RELAY_VENTILATOR) == HIGH))
        {
          digitalWrite(PIN_RELAY_VENTILATOR, LOW);
          iconVentilator=true;
        }
        else if((firebaseData.stringData() == "0") && (digitalRead(PIN_RELAY_VENTILATOR) == LOW))
        {
          digitalWrite(PIN_RELAY_VENTILATOR, HIGH);
          iconVentilator=true;
        }
        else
        {
          Serial.println("statusVentilator OK!");
        }
      }
      else
      {
        Serial.println("Error reading statusVentilator");
      }
      if(Firebase.getString(firebaseData,"/statusHumidifier"))
      {
        if((firebaseData.stringData() == "1") && (digitalRead(PIN_RELAY_HUMDIDIFIER) == HIGH))
        {
          digitalWrite(PIN_RELAY_HUMDIDIFIER, LOW);
          iconHumidifier=true;
        }
        else if((firebaseData.stringData() == "0") && (digitalRead(PIN_RELAY_HUMDIDIFIER) == LOW))
        {
          digitalWrite(PIN_RELAY_HUMDIDIFIER, HIGH);
          iconHumidifier=true;
        }
        else
        {
          Serial.println("statusHumidifier OK!");
        }
      }
      else
      {
        Serial.println("Error reading statusHumidifier");
      }
      if(Firebase.getString(firebaseData,"/statusFan"))
      {
        if((firebaseData.stringData() == "1") && (digitalRead(PIN_RELAY_FAN) == HIGH))
        {
          digitalWrite(PIN_RELAY_FAN, LOW);
          iconFan=true;
        }
        else if((firebaseData.stringData() == "0") && (digitalRead(PIN_RELAY_FAN) == LOW))
        {
          digitalWrite(PIN_RELAY_FAN, HIGH);
          iconFan=true;
        }
        else
        {
          Serial.println("statusFan OK!");
        }
      }
      else
      {
        Serial.println("Error reading statusFan");
      }
      if(Firebase.getString(firebaseData,"/statusHeatmat"))
      {
        if((firebaseData.stringData() == "1") && (digitalRead(PIN_RELAY_HEATMAT) == HIGH))
        {
          digitalWrite(PIN_RELAY_HEATMAT, LOW);
          iconHeatmat=true;
        }
        else if((firebaseData.stringData() == "0") && (digitalRead(PIN_RELAY_HEATMAT) == LOW))
        {
          digitalWrite(PIN_RELAY_HEATMAT, HIGH);
          iconHeatmat=true;
        }
        else
        {
          Serial.println("statusHeatmat OK!");
        }
      }
      else
      {
        Serial.println("Error reading statusHeatmat");
      }
    }
    else{Serial.println("No user input - carry on");}
  }
}

void drawButtons(void)
{
  key[1].initButton(&tft, 46, 122, 60, 60, TFT_WHITE, COLOR_BLACK, TFT_WHITE, "V", 1);
  key[2].initButton(&tft, 122, 122, 60, 60, TFT_WHITE, COLOR_BLACK, TFT_WHITE, "H", 1);
  key[3].initButton(&tft, 198, 122, 60, 60, TFT_WHITE, COLOR_BLACK, TFT_WHITE, "F", 1);
  //key[4].initButton(&tft, 274, 122, 60, 60, TFT_WHITE, COLOR_BLACK, TFT_WHITE, "W", 1);
  key[5].initButton(&tft, 84, 198, 136, 60, TFT_WHITE, COLOR_BLACK, TFT_WHITE, "HM", 1);
  key[4].initButton(&tft, 236, 198, 136, 60, TFT_WHITE, COLOR_BLACK, TFT_WHITE, "PO", 1);
  key[1].drawButton();
  key[5].drawButton();
}

void touchScreen(void)
{
  uint16_t t_x = 0, t_y = 0;
  boolean pressed = tft.getTouch(&t_x, &t_y);

  for(uint8_t b = 1; b < 7; b++) 
  {
    if(pressed && key[b].contains(t_x, t_y)) {
      key[b].press(true); 
    } 
    else 
    {
      key[b].press(false); 
    }
  }
  
  for(uint8_t b = 1; b < 7; b++) 
  {
    if(key[b].justPressed()) 
    {
      // Icon Ventilator
      if(b == 1)
      {
        Serial.println("Ventilator pressed!");
        digitalWrite(PIN_RELAY_VENTILATOR, !digitalRead(PIN_RELAY_VENTILATOR));
        iconVentilator=true;
        Serial.println("Ventilator " + digitalRead(PIN_RELAY_VENTILATOR));
        
      }
      // Icon Humidifier
      if(b == 2)
      {
        Serial.println("Humidifier pressed!");
        digitalWrite(PIN_RELAY_HUMDIDIFIER, !digitalRead(PIN_RELAY_HUMDIDIFIER));
        iconHumidifier=true;
        
      }
      // Icon Fan
      if(b == 3)
      {
        Serial.println("Fan pressed!");
        digitalWrite(PIN_RELAY_FAN, !digitalRead(PIN_RELAY_FAN));
        iconFan=true;
        
      }
      // Icon Water
      if(b == 4)
      {
        Serial.println("Power pressed!");
        // TODO: SLEEP MODE
      }
      // Icon Heatmat
      if(b == 5)
      {
        Serial.println("Heatmat pressed!");
        digitalWrite(PIN_RELAY_HEATMAT, !digitalRead(PIN_RELAY_HEATMAT));
        iconHeatmat=true;
        
      }
      // Icon Power
      if(b == 6)
      {
        Serial.println("Water pressed!");
        // TODO: POPUP Window with details on waterlevel
      }
    }
  }
}

void touchCalibrate(void)
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void pinInit (void)
{
   //define relays and turn off (for safety)
  pinMode(PIN_RELAY_HUMDIDIFIER, OUTPUT);
  pinMode(PIN_RELAY_FAN, OUTPUT);
  pinMode(PIN_RELAY_VENTILATOR, OUTPUT);
  pinMode(PIN_RELAY_HEATMAT, OUTPUT);
  digitalWrite(PIN_RELAY_HUMDIDIFIER, HIGH),
  digitalWrite(PIN_RELAY_FAN, HIGH);
  digitalWrite(PIN_RELAY_VENTILATOR, HIGH);
  digitalWrite(PIN_RELAY_HEATMAT, HIGH);

  //define watersensor
  pinMode(PIN_WATER_TRIGGER, OUTPUT);
  pinMode(PIN_WATER_ECHO, INPUT);
  digitalWrite(PIN_WATER_TRIGGER, HIGH);
}