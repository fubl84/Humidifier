#include <Adafruit_GFX.h>     // Display
#include <Adafruit_ST7735.h>  // Display
#include <DHT.h>              // Sensor DHT11

//defines for I/O pins
#define PIN_2_DHT               2
#define PIN_3_RELAY_HUMDIDIFIER 3
#define PIN_4_RELAY_FAN         4
#define PIN_5_RELAY_VENTILATOR  5 
#define PIN_6_WATER_TRIGGER     6
#define PIN_7_WATER_ECHO        7
#define TFT_RST                 8
#define TFT_DC                  9
#define TFT_CS                  10
// The rest of the pins are pre-selected as the default hardware SPI for Arduino Nano (SCK = 13 and SDA = 11)

// Create display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

//define colors for display
#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_TEMP  0xB437
#define COLOR_HUM   0x359A
#define COLOR_ON    0x2700
#define COLOR_OFF   0x5AEB
#define COLOR_LOW   0xFD20
#define COLOR_MID   0xF800
#define COLOR_OK    0x015C
#define COLOR_PLAN  0xEF4E

//bitmaps
const unsigned char ventilator [] PROGMEM = 
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x00, 
  0x00, 0x7f, 0xc0, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x18, 0x3e, 0x00, 0x00, 
  0x3c, 0x3c, 0x00, 0x00, 0x7e, 0x3c, 0x3c, 0x00, 0x7f, 0xb6, 0xfe, 0x00, 0x7f, 0xe3, 0xff, 0x00, 
  0x3f, 0xc1, 0xff, 0x00, 0x3f, 0xe3, 0xff, 0x00, 0x1f, 0x94, 0xfe, 0x00, 0x06, 0x1c, 0x3e, 0x00, 
  0x00, 0x3e, 0x1c, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 
  0x01, 0xff, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00
};

const unsigned char fan [] PROGMEM = 
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x03, 0xf1, 0xe0, 0x00, 0x07, 0xe3, 0xf0, 0x00, 
  0x0f, 0xe7, 0xf8, 0x00, 0x1d, 0xe7, 0x9c, 0x00, 0x39, 0xe7, 0x8c, 0x00, 0x3c, 0x66, 0x3e, 0x00, 
  0x3e, 0x64, 0x7e, 0x00, 0x7e, 0x3c, 0xfe, 0x00, 0x7f, 0x3f, 0xff, 0x00, 0x4f, 0xff, 0x03, 0x00, 
  0x40, 0x77, 0x03, 0x00, 0x60, 0x7f, 0xfb, 0x00, 0x7f, 0xbe, 0x7f, 0x00, 0x7f, 0x1e, 0x7e, 0x00, 
  0x3e, 0x13, 0x3e, 0x00, 0x38, 0x73, 0x8c, 0x00, 0x38, 0xf3, 0x8c, 0x00, 0x1d, 0xf3, 0x9c, 0x00, 
  0x0f, 0xf3, 0xf8, 0x00, 0x07, 0xc3, 0xf0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x3e, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00
};

const unsigned char humidifier [] PROGMEM = 
{
  0x00, 0x00, 0x00, 0x00, 0x07, 0x0f, 0x00, 0x00, 0x1f, 0xdf, 0x80, 0x00, 0x3f, 0xff, 0xe0, 0x00, 
  0x38, 0xff, 0xf0, 0x00, 0x38, 0x78, 0xf0, 0x00, 0x10, 0x70, 0x70, 0x00, 0x00, 0x60, 0x70, 0x00, 
  0x00, 0x60, 0x20, 0x00, 0x3e, 0x60, 0x00, 0x00, 0x3f, 0x60, 0x00, 0x00, 0x73, 0x60, 0x00, 0x00, 
  0x73, 0x00, 0x38, 0x00, 0x33, 0x61, 0xfe, 0x00, 0x03, 0x63, 0xfe, 0x00, 0x00, 0x67, 0xff, 0x00, 
  0x03, 0x0f, 0x0f, 0x00, 0x03, 0x6c, 0x07, 0x00, 0x00, 0x60, 0x02, 0x00, 0x03, 0x6c, 0x00, 0x00, 
  0x03, 0x0c, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x03, 0x6c, 0x00, 0x00, 0x03, 0x6c, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00
};

long  echoDuration=0;             // duration for the echo
long  waterDistance=0;            // distance from sensor to waterlevel (mm)
int   maxWater=20;                // lowest acceptable waterlevel (mm)
int   minWater=160;               // highest acceptable waterlevel (mm)
int   timeUntilVentilation;
int   maxHumidity=78;
int   minHumidity=70;

//initialize DHT
#define DHTTYPE DHT11
DHT dht(PIN_2_DHT, DHTTYPE);

//humidity controll values
#define HIGHLIMIT 78
#define LOWLIMIT  70

//setup timestamps and intervalls
const long timerSensor=10000;
const long timerIntervallVentilation=1800000;
const long timerVentilation=300000;
const long timerFanDelay=30000;

unsigned long previousSensor=0;
unsigned long previousIntervallVentilation=0;
unsigned long previousVentilation=0;
unsigned long previousFanDelay=0;

//create "states"
bool ventilate;
bool fanDelay;
bool finishedHumidifying=true;

void setup() 
{
    //start commenting on serial port
    Serial.begin(9600);
    
    //setup Display
    displaySetup();

    //define relays and turn off (for safety)
    pinMode(PIN_3_RELAY_HUMDIDIFIER, OUTPUT);
    pinMode(PIN_4_RELAY_FAN, OUTPUT);
    pinMode(PIN_5_RELAY_VENTILATOR, OUTPUT);
    digitalWrite(PIN_3_RELAY_HUMDIDIFIER, HIGH),
    digitalWrite(PIN_4_RELAY_FAN, HIGH);
    digitalWrite(PIN_5_RELAY_VENTILATOR, HIGH);

    //define watersensor
    pinMode(PIN_6_WATER_TRIGGER, OUTPUT);
    pinMode(PIN_7_WATER_ECHO, INPUT);
    digitalWrite(PIN_6_WATER_TRIGGER, HIGH);

    //start DHT sensor
    dht.begin();
}

void loop() 
{
    unsigned long currentTime = millis();

    if(currentTime - previousSensor >= timerSensor)
    {
        Serial.print("Getting Waterlevel!");
        //get waterlevel
        digitalWrite(PIN_6_WATER_TRIGGER, LOW);         
        delay(5);                           
        digitalWrite(PIN_6_WATER_TRIGGER, HIGH);        
        delay(10);                         
        digitalWrite(PIN_6_WATER_TRIGGER, LOW);         
        echoDuration = pulseIn(PIN_7_WATER_ECHO, HIGH);        
        waterDistance = (echoDuration/2) * 0.3432;    
        Serial.print("Waterlevel done!");
        if (waterDistance >= 120)               
        {
          tft.fillRoundRect(95,4, 28,28, 7, COLOR_LOW);
          tft.setCursor(100, 23);
          tft.println("W");
          //return;
        }
        else if (waterDistance >= 60 && waterDistance  < 120)
        {
          tft.fillRoundRect(95,4, 28,28, 7, COLOR_MID);
          tft.setCursor(100, 23);
          tft.println("W");
        }
        else
        {
          tft.fillRoundRect(95,4, 28,28, 7, COLOR_OK);
          tft.setCursor(100, 23);
          tft.println("W");
        }

        //get temperature and humidity and show on OLED
        Serial.print("Updating DHT11 Data!");
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        char humidity[10];
        dtostrf(h, 2, 0, humidity);
        char temperature[10];
        dtostrf(t, 2, 0, temperature);
        tft.setFont(&FreeSansBold9pt7b);
    
        //Humidity
        tft.setTextColor(COLOR_HUM);
        tft.fillRect(43,70, 23, 20, COLOR_BLACK);
        tft.setCursor(44, 85);
        tft.println(humidity);
        tft.setCursor(65, 85);
        tft.println("%");
        
        //Temperature
        tft.setTextColor(COLOR_TEMP);
        tft.fillRect(44,130, 23, 20, COLOR_BLACK);
        tft.setCursor(45, 145);
        tft.println(temperature);
        tft.setCursor(68, 145);
        tft.println("C");
       
        //check status and turn on / turn off / ignore
        if(digitalRead(PIN_3_RELAY_HUMDIDIFIER) == LOW)
        {
            Serial.print("PART 1 LOOP OK");
            if(h >= HIGHLIMIT)
            {
                digitalWrite(PIN_3_RELAY_HUMDIDIFIER, HIGH);
                digitalWrite(PIN_5_RELAY_VENTILATOR, HIGH);
                tft.fillRoundRect(5, 4, 28,28, 7, COLOR_OFF);
                tft.drawBitmap(7, 5, ventilator, 25, 25, COLOR_BLACK);
                tft.fillRoundRect(35,4, 28,28, 7, COLOR_OFF);
                tft.drawBitmap(37, 5, humidifier, 25, 25, COLOR_BLACK);
                previousIntervallVentilation=currentTime;
                previousFanDelay=currentTime;
                finishedHumidifying=true;
                fanDelay=true;
                Serial.print("HUMIDITY OK / TURN OFF");
            }
         }
        else
        {
            Serial.print("PART 2 LOOP OK");
            if(h <= LOWLIMIT)
            {
                digitalWrite(PIN_3_RELAY_HUMDIDIFIER, LOW);
                digitalWrite(PIN_4_RELAY_FAN, LOW);
                digitalWrite(PIN_5_RELAY_VENTILATOR, LOW);
                tft.fillRoundRect(5, 4, 28,28, 7, COLOR_ON);
                tft.drawBitmap(7, 5, ventilator, 25, 25, COLOR_BLACK);
                tft.fillRoundRect(35,4, 28,28, 7, COLOR_ON);
                tft.drawBitmap(37, 5, humidifier, 25, 25, COLOR_BLACK);
                tft.fillRoundRect(65,4, 28,28, 7, COLOR_ON);
                tft.drawBitmap(67, 5, fan, 25, 25, COLOR_BLACK);
                finishedHumidifying=false;
                Serial.print("HUMIDITY LOW / TURN ON");
            }
        }
        previousSensor=currentTime;
        Serial.print("Set new 5S Timer!");
    }
    
    //keep the fogfan on for 20 seconds
    if((fanDelay == true) && (currentTime - previousFanDelay >= timerFanDelay))
    {
        Serial.print("FAN DELAY LOOP OK!");
        digitalWrite(PIN_4_RELAY_FAN, HIGH);
        tft.fillRoundRect(65,4, 28,28, 7, COLOR_OFF);
        tft.drawBitmap(67, 5, fan, 25, 25, COLOR_BLACK);
        fanDelay=false;
    }
    
    //when the 30 minutes after humidifying are finished / scheduled ventilation 30 minutes
    if((finishedHumidifying == true) && (currentTime - previousIntervallVentilation >= timerIntervallVentilation))
    {
        Serial.print("30 MINUTES VENT LOOP OK!");
        digitalWrite(PIN_5_RELAY_VENTILATOR, LOW);
        previousVentilation=currentTime;
        tft.fillRoundRect(5, 4, 28,28, 7, COLOR_ON);
        tft.drawBitmap(7, 5, ventilator, 25, 25, COLOR_BLACK);
        ventilate=true;
        finishedHumidifying=false;
    }
    else
    {
        if(finishedHumidifying == true)
        {
            timeUntilVentilation = ((timerIntervallVentilation - (currentTime - previousIntervallVentilation))/1000/60);
            tft.fillRoundRect(5, 4, 28,28, 7, COLOR_OFF);
            tft.drawBitmap(7, 5, ventilator, 25, 25, COLOR_BLACK);
        }
    }
    
    //when 5 minutes of ventilation have passed
    if((ventilate == true) && (currentTime - previousVentilation >= timerVentilation))
    {
        Serial.print("5 MIN VENTILATION LOOP OK!");
        digitalWrite(PIN_5_RELAY_VENTILATOR, HIGH);
        tft.fillRoundRect(5, 4, 28,28, 7, COLOR_OFF);
        tft.drawBitmap(7, 5, ventilator, 25, 25, COLOR_BLACK);
        previousIntervallVentilation=currentTime;
        ventilate=false;
        finishedHumidifying=true;
    }

}

void displaySetup(void)
{
  tft.initR(INITR_BLACKTAB);        // Initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);     // Fill screen with black
  tft.setRotation(0);
  tft.setTextWrap(false);           //disable text overflow
  
  //Global font settings
  tft.setTextColor(ST7735_WHITE);   // Set color of text. First is the color of text and after is color of background
  tft.setFont(&FreeSans9pt7b);
  tft.setTextSize(0);               // Has to be 0 for custom fonts

  //Set backgrounds for symbols
  tft.fillRoundRect(5, 4, 28,28, 7, COLOR_OFF);
  tft.fillRoundRect(35,4, 28,28, 7, COLOR_OFF);
  tft.fillRoundRect(65,4, 28,28, 7, COLOR_OFF);
  //tft.fillRoundRect(95,4, 28,28, 7, COLOR_OK);

  //place symbols
  tft.drawBitmap(7, 5, ventilator, 25, 25, COLOR_BLACK);
  tft.drawBitmap(37, 5, humidifier, 25, 25, COLOR_BLACK);
  tft.drawBitmap(67, 5, fan, 25, 25, COLOR_BLACK);
  //tft.setCursor(100, 23);
  //tft.println("W");
  
  //Initializing humidity display
  tft.setCursor(28, 55);
  tft.println("Humidity");
  tft.drawRoundRect(40,65, 45,30, 10, COLOR_HUM);

  //Initializing temperature display
  tft.setCursor(11, 115);  
  tft.println("Temperature"); 
  tft.drawRoundRect(40,125, 45,30, 10, COLOR_TEMP);
}
