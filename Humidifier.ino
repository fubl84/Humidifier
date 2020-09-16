/***************************************************************************//**
 * @file		Humidifier.ino
 *
 * @creator		
 * @created		
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
#include "inttypes.h"
#include <SeeedOLED.h> // Für OLED
#include <Wire.h> // Für OLED
#include <DHT.h> // Sensor

/* ****************************************************************************/
/* *************************** DEFINES AND MACROS *****************************/
/* ****************************************************************************/
#define MOD_10_SECONDS  10000

//defines for I/O pins
#define PIN_3_RELAY_HUMDIDIFIER     3
#define PIN_4_RELAY_FAN             4
#define PIN_5_RELAY_VENTILATOR      5
#define PIN_12_WATER_LEVEL_TRIGGER  12  // Trigger Pin Wasserstand
#define PIN_13_ECHO_PIN             13  // Echo Pin Wasserstand

#define MIN_WATER_LEVEL             130 // Entfernung Minimaler Wasserstand (mm)
#define MAX_WATER_LEVEL             40  // Entfernung Maximaler Wasserstand (mm)

#define DHTPIN 2                      // Welcher Pin wird für das DHT benutzt?
#define DHTTYPE DHT11                 // Wir haben erstmal "nur" das DHT11

// Humidity Kontroll-Werte
#define SETPOINT 74                   // Mittelwert Luftfeuchtigkeit
#define DEADBAND 4                    // Schwellenwert Luftfeuchtigkeit

/* ****************************************************************************/
/* *********************** STRUCTS, ENUMS AND TYPEDEFS ************************/
/* ****************************************************************************/
typedef enum States
{
  State_Init,
  State_Idle,
  State_Humidification,
  State_Ventilation,
}States_t;

typedef struct UltrasonicMeasurmentData
{
  uint16_t Time;
  uint16_t Distance;
}UltrasonicMeasurmentData_t;

typedef struct HumidifierData
{
  States_t                    State;
  UltrasonicMeasurmentData_t  UltrasonicMeasurmentData;
  uint8_t                     WaterLevelPercent; // Prozent der Befüllung anlegen
  float                       Humidity;
  float                       Temperature;
  bool                        HumdidifierStatus;
}HumidifierData_t;

/* ****************************************************************************/
/* ******************** FORWARD DECLARATIONS / PROTOTYPES *********************/
/* ****************************************************************************/

/***************************************************************************//**
 * TODO: A short description.
 *
 * @param param TODO: Parameter description
 *
 * @return      TODO: return description
 *
 ******************************************************************************/
void setup_oled();

/***************************************************************************//**
 * TODO: A short description.
 *
 * @param param TODO: Parameter description
 *
 * @return      TODO: return description
 *
 ******************************************************************************/
uint16_t measureTime(void);

/***************************************************************************//**
 * TODO: A short description.
 *
 * @param param TODO: Parameter description
 *
 * @return      TODO: return description
 *
 ******************************************************************************/
uint16_t calcDistance(uint16_t time);

/***************************************************************************//**
 * TODO: A short description.
 *
 * @param param TODO: Parameter description
 *
 * @return      TODO: return description
 *
 ******************************************************************************/
uint8_t calcWaterLevelPercent(uint16_t distance);

/***************************************************************************//**
 * TODO: A short description.
 *
 * @param param TODO: Parameter description
 *
 * @return      TODO: return description
 *
 ******************************************************************************/
void ControlHumdifier(bool active);

/***************************************************************************//**
 * TODO: A short description.
 *
 * @param param TODO: Parameter description
 *
 * @return      TODO: return description
 *
 ******************************************************************************/
void ControlVentilator(bool active);

/* ****************************************************************************/
/* ******************************** VARIABLES *********************************/
/* ****************************************************************************/
HumidifierData_t HumidifierData;

// Initialisierung für DHT
DHT dht(DHTPIN, DHTTYPE);

// Zeitmessung wird vorbereitet
uint32_t previousTemp = 0;
uint32_t previousInterVent = 0;
uint32_t previousDelay = 0;
uint32_t ventTimeout = 0;

const uint32_t timerTemp = 10000;        //Intervall für Messung 10 Sekunden
const uint32_t timerInterVent = 1800000; //Intervall für Lüftung 30 Minuten
const uint32_t timerVent = 300000;       //Intervall für Lüftungsdauer 5 Minuten
const uint32_t timerDelay = 20000;       //Intervall für Nachlüftung 20 Sekunden

/* ****************************************************************************/
/* ************************** FUNCTION DEFINITIONS ****************************/
/* ****************************************************************************/

/***************************************************************************//**
 *
 ******************************************************************************/
void setup(void) 
{
  HumidifierData.State = State_Init;

  // OLED "einrichten"
  setup_oled();

  // Alle Relays definieren und ausschalten
  pinMode(PIN_3_RELAY_HUMDIDIFIER, OUTPUT);
  pinMode(PIN_4_RELAY_FAN, OUTPUT);
  pinMode(PIN_5_RELAY_VENTILATOR, OUTPUT);
  digitalWrite(PIN_3_RELAY_HUMDIDIFIER, LOW);
  digitalWrite(PIN_4_RELAY_FAN, LOW);
  digitalWrite(PIN_5_RELAY_VENTILATOR, LOW);

  dht.begin();                          //der DHT soll starten

  // Wassersensor einrichten
  pinMode(PIN_12_WATER_LEVEL_TRIGGER, OUTPUT);
  pinMode(PIN_13_ECHO_PIN, INPUT);     
}

/***************************************************************************//**
 *
 ******************************************************************************/
void loop_new(void) 
{
  switch(HumidifierData.State)
  {
    case State_Init:
    HumidifierData.State = State_Idle;
    break;

    case State_Idle:
    {
      //Measure temperature and humidity every 10 seconds
      if(millis() % MOD_10_SECONDS)
      {

      }
    }
    break;

    case State_Humidification:
    break;

    case State_Ventilation:
    break;

    default:
    break;
  }
}

/***************************************************************************//**
 *
 ******************************************************************************/
void loop() 
{
  unsigned long currentTime = millis();           //Zeitzähler wird "gestartet"

  // Abfrage, ob Lüfter vom Humidifier ausgeschaltet werden muss
  if (digitalRead(PIN_5_RELAY_VENTILATOR) == HIGH && digitalRead(PIN_3_RELAY_HUMDIDIFIER) == LOW)
  {
    if (currentTime - previousDelay >= timerDelay)
    {
      digitalWrite(PIN_4_RELAY_FAN, LOW);
    }
  }

  // Abfrage für Temp & Humidity alle 10 Sekunden
  if (currentTime - previousTemp >= timerTemp)   
  {                                               
    previousTemp = currentTime;                  //Neue Referenz-Zeit

    HumidifierData.UltrasonicMeasurmentData.Time = measureTime();
    HumidifierData.UltrasonicMeasurmentData.Distance = calcDistance(HumidifierData.UltrasonicMeasurmentData.Time);
    HumidifierData.WaterLevelPercent = calcWaterLevelPercent(HumidifierData.UltrasonicMeasurmentData.Distance);

    if (HumidifierData.WaterLevelPercent <= 15)               //Wenn das gemessene Wasser unter 15% liegt
    {
      digitalWrite(4, LOW);                       
      return;
    }
    else{}

    // Temperatur und Humidity auslesen
    HumidifierData.Humidity = dht.readHumidity();
    HumidifierData.Temperature = dht.readTemperature();

    // Abfrage, ob Befeuchter ausgeschaltet werden muss
    if(HumidifierData.Humidity > SETPOINT + DEADBAND) 
    {
      HumidifierData.HumdidifierStatus = false;

      ControlHumdifier(false);

      previousDelay = currentTime;  //Für den Lüfter vom Humidifier
      previousInterVent = currentTime;   //Für das halbstündige Lüften
    }
    else if(HumidifierData.Humidity < SETPOINT - DEADBAND)
    {
      HumidifierData.HumdidifierStatus = true;

      ControlHumdifier(true);
    }
    else{}

    updateDisplay(HumidifierData.WaterLevelPercent, HumidifierData.Humidity, HumidifierData.Temperature, HumidifierData.HumdidifierStatus);      
  }

  // ventilation
  if (currentTime - timerInterVent >= timerInterVent)   //Alle 30 Minuten lüften
  {
      ControlVentilator(true);
      previousInterVent = currentTime;             
      ventTimeout = currentTime;                                                                
  }

  //Check if blower should be turned off
  if (currentTime - ventTimeout >= timerVent)
  {
    ControlVentilator(false);
  }
  else{}
}

/***************************************************************************//**
 *
 ******************************************************************************/
void setup_oled(void)
{
  Wire.begin();
  SeeedOled.init();                    
  SeeedOled.clearDisplay();             
  SeeedOled.setNormalDisplay();         
  SeeedOled.setPageMode();              
  SeeedOled.setTextXY(0, 0);            
  SeeedOled.putString("HUMIDITY: ");    
  SeeedOled.setTextXY(3, 0);            
  SeeedOled.putString("TEMP.: ");       
  SeeedOled.setTextXY(0, 13);           
  SeeedOled.putString("%");             
  SeeedOled.setTextXY(3, 13);           
  SeeedOled.putString("C");              
  SeeedOled.setTextXY(5, 0);            
  SeeedOled.putString("WASSER:");       
  SeeedOled.setTextXY(5, 13);            
  SeeedOled.putString("%"); 
}

/***************************************************************************//**
 *
 ******************************************************************************/
uint16_t measureTime(void)
{
  uint16_t time;

  // Wasserstand abrufen und anzeigen
  digitalWrite(PIN_12_WATER_LEVEL_TRIGGER, LOW);         //Keinen Ton senden
  delay(5);                           //5 Millisekunden warten
  digitalWrite(PIN_12_WATER_LEVEL_TRIGGER, HIGH);        //Ultraschall senden
  delay(10);                          //10 Millisekunden Ton senden
  digitalWrite(PIN_12_WATER_LEVEL_TRIGGER, LOW);         //Ton abschalten
  time = pulseIn(PIN_13_ECHO_PIN, HIGH);        //Dauer, bis der Ton zurückkommt - in Mikrosekunden

  return(time);
}

/***************************************************************************//**
 *
 ******************************************************************************/
uint16_t calcDistance(uint16_t time)
{
  uint16_t distance;

  distance = (time / 2) * 0.3432; //Entfernung in Millimetern
}

/***************************************************************************//**
 *
 ******************************************************************************/
uint8_t calcWaterLevelPercent(uint16_t distance)
{
  return(100 - ((distance - MAX_WATER_LEVEL) * (100 / (MIN_WATER_LEVEL - MAX_WATER_LEVEL))));     //Umrechnen in Prozent
}

/***************************************************************************//**
 *
 ******************************************************************************/
void updateDisplay(uint8_t waterlevel, float humidity, float temperature, bool humstatus)
{
  char pstring[10];
  char hstring[10];
  char tstring[10];
  char hsstring[4];

  // Temperatur und Humidity in einen String umwandeln
  dtostrf(waterlevel,3, 0, pstring);
  dtostrf(humidity,2, 0, hstring);
  dtostrf(temperature,2, 0, tstring);

  if(humstatus == true)
  {
    strcpy(hsstring, "ON");
  }
  else
  {
    strcpy(hsstring, "OFF");
  }  

  if (waterlevel <= 15)               //Wenn das gemessene Wasser unter 15% liegt
  {
    SeeedOled.setTextXY(6, 0);                  
    SeeedOled.putString("    WATER LOW    ");   
    SeeedOled.setTextXY(5, 7);                  
    SeeedOled.putString("    ");                
    SeeedOled.setTextXY(5, 8);                  
    SeeedOled.putString(pstring);               
  }
  else
  {
    SeeedOled.setTextXY(5, 7);                  
    SeeedOled.putString("    ");               
    SeeedOled.setTextXY(5, 9);                 
    SeeedOled.putString(pstring);               
    SeeedOled.setTextXY(6, 0);                  
    SeeedOled.putString("                 ");   
  }

  // Aktuellen Wert auf dem OLED anzeigen lassen
  SeeedOled.setTextXY(0, 11);            
  SeeedOled.putString(hstring);         
  SeeedOled.setTextXY(3, 11);            
  SeeedOled.putString(tstring);

  SeeedOled.setTextXY(6, 0);            
  SeeedOled.putString(hsstring);
}

/***************************************************************************//**
 *
 ******************************************************************************/
void ControlHumdifier(bool active)
{
  if((active == true) &&
     (digitalRead(PIN_3_RELAY_HUMDIDIFIER) == LOW))
  {
    digitalWrite(PIN_3_RELAY_HUMDIDIFIER, HIGH);
    digitalWrite(PIN_5_RELAY_VENTILATOR, HIGH);
    digitalWrite(PIN_4_RELAY_FAN, HIGH);
  }
  else if((active == false) &&
          (digitalRead(PIN_3_RELAY_HUMDIDIFIER) == HIGH))
  {
    digitalWrite(PIN_3_RELAY_HUMDIDIFIER, LOW);
    digitalWrite(PIN_5_RELAY_VENTILATOR, LOW);
  }
  else{}
}

/***************************************************************************//**
 *
 ******************************************************************************/
void ControlVentilator(bool active)
{
  // Only control the vent if the humdifier is turned off
  if(digitalRead(PIN_3_RELAY_HUMDIDIFIER) == LOW)
  {
    if((active == true) &&
      (digitalRead(PIN_5_RELAY_VENTILATOR) == LOW))
    {
      digitalWrite(PIN_5_RELAY_VENTILATOR, HIGH);
    }
    else if((active == false) &&
            (digitalRead(PIN_5_RELAY_VENTILATOR) == LOW))
    {
      digitalWrite(PIN_5_RELAY_VENTILATOR, LOW);
    }
    else{}
  }
  else{}
}