#include <SeeedOLED.h> // Für OLED
#include <Wire.h> // Für OLED
#include <DHT.h> // Sensor

int trigger=12;                       // Trigger Pin Wasserstand
int echo=13;                          // Echo Pin Wasserstand
int prozent=0;                        // Wasserstand anlegen
long dauer=0;                         // Dauer anlegen für Wasserstand
long entfernung=0;                    // Entfernung anlegen für Wasserstand
int maxwater=40;                       // Entfernung Maximaler Wasserstand (mm)
int minwater=130;                      // Entfernung Minimaler Wasserstand (mm)
int prozentsatz = 100/(minwater-maxwater);  // Prozentsatz ausrechnen
int entfernungsprozent = 0;            // Prozent der Befüllung anlegen
int relayHumidifier = 3;               // Relay 1 Humidifier
int relayFan = 4;                      // Relay 2 Lüfter am Humidifier 
int relayVentilator = 5;               // Relay 3 Lüfter im Gewächshaus
int relay4 = 6;                        // Wird noch nicht gebraucht

// Initialisierung für DHT
#define DHTPIN 2                      // Welcher Pin wird für das DHT benutzt?
#define DHTTYPE DHT11                 // Wir haben erstmal "nur" das DHT11
DHT dht(DHTPIN, DHTTYPE);

// Humidity Kontroll-Werte
#define SETPOINT 74                   // Mittelwert Luftfeuchtigkeit
#define DEADBAND 4                    // Schwellenwert Luftfeuchtigkeit

// Zeitmessung wird vorbereitet
unsigned long previousTemp = 0;
unsigned long previousInterVent = 0;
unsigned long previousVent = 0;
unsigned long previousDelay = 0;

const long timerTemp = 10000;        //Intervall für Messung 10 Sekunden
const long timerInterVent = 1800000; //Intervall für Lüftung 30 Minuten
const long timerVent = 300000;       //Intervall für Lüftungsdauer 5 Minuten
const long timerDelay = 20000;       //Intervall für Nachlüftung 20 Sekunden



void setup() {
// OLED "einrichten"
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

// Alle Relays definieren und ausschalten
pinMode(relayHumidifier, OUTPUT);
pinMode(relayFan, OUTPUT);
pinMode(relayVentilator, OUTPUT);
pinMode(relay4, OUTPUT);
digitalWrite(relayHumidifier, LOW);
digitalWrite(relayFan, LOW);
digitalWrite(relayVentilator, LOW);
digitalWrite(relay4, LOW);            //Unbenutzt

dht.begin();                          //der DHT soll starten

// Wassersensor einrichten
pinMode(trigger, OUTPUT);
pinMode(echo, INPUT);     
}



void loop() {

unsigned long currentTime = millis();           //Zeitzähler wird "gestartet"

// Abfrage, ob Gehäuselüfter ausgeschaltet werden muss
if (digitalRead(relayVentilator) == HIGH && digitalRead(relayHumidifier) == LOW)
{
  if (currentTime - previousVent >= timerVent)
  {
    digitalWrite(relayVentilator, LOW);
  }
}

// Abfrage, ob Lüfter vom Humidifier ausgeschaltet werden muss
if (digitalRead(relayFan) == HIGH && digitalRead(relayHumidifier) == LOW)
{
  if (currentTime - previousDelay >= timerDelay)
  {
    digitalWrite(relayFan, LOW);
  }
}

// Abfrage für Temp & Humidity alle 10 Sekunden
if (currentTime - previousTemp >= timerTemp)   
{                                               
  previousTemp = currentTime;                  //Neue Referenz-Zeit

// Wasserstand abrufen und anzeigen
  digitalWrite(trigger, LOW);         //Keinen Ton senden
  delay(5);                           //5 Millisekunden warten
  digitalWrite(trigger, HIGH);        //Ultraschall senden
  delay(10);                          //10 Millisekunden Ton senden
  digitalWrite(trigger, LOW);         //Ton abschalten
  dauer = pulseIn(echo, HIGH);        //Dauer, bis der Ton zurückkommt - in Mikrosekunden
  entfernung = (dauer/2) * 0.3432;    //Entfernung in Millimetern
  entfernungsprozent = 100-((entfernung-maxwater)*prozentsatz);      //Umrechnen in Prozent
  char pstring[10];
  dtostrf(entfernungsprozent,3, 0, pstring);
    if (entfernungsprozent <= 15)               //Wenn das gemessene Wasser unter 15% liegt
    {
      SeeedOled.setTextXY(6, 0);                  
      SeeedOled.putString("    WATER LOW    ");   
      SeeedOled.setTextXY(5, 7);                  
      SeeedOled.putString("    ");                
      SeeedOled.setTextXY(5, 8);                  
      SeeedOled.putString(pstring);               
      digitalWrite(4, LOW);                       
      return;
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

// Temperatur und Humidity auslesen
  float h = dht.readHumidity();
  float t = dht.readTemperature();

// Temperatur und Humidity in einen String umwandeln
  char hstring[10];
  dtostrf(h,2, 0, hstring);
  char tstring[10];
  dtostrf(t,2, 0, tstring);

// Aktuellen Wert auf dem OLED anzeigen lassen
  SeeedOled.setTextXY(0, 11);            
  SeeedOled.putString(hstring);         
  SeeedOled.setTextXY(3, 11);            
  SeeedOled.putString(tstring);         

// Abfrage, ob Befeuchter ausgeschaltet werden muss
    if(digitalRead(relayHumidifier) == HIGH) 
    {
      if(h > SETPOINT + DEADBAND) 
      {
        digitalWrite(relayHumidifier, LOW);
        digitalWrite(relayVentilator, LOW);
        SeeedOled.setTextXY(6, 0);            
        SeeedOled.putString("OFF");
        previousDelay = currentTime;  //Für den Lüfter vom Humidifier
        previousInterVent = currentTime;   //Für das halbstündige Lüften
      }
    }
    else
    {
      if(h < SETPOINT - DEADBAND) 
      {
        digitalWrite(relayHumidifier, HIGH);
        digitalWrite(relayVentilator, HIGH);
        digitalWrite(relayFan, HIGH);
        SeeedOled.setTextXY(6, 0);            
        SeeedOled.putString("ON ");
      }
    }    
}
if (digitalRead(relayVentilator) == LOW)
{
  if (currentTime - previousInterVent >= timerInterVent)   //Alle 30 Minuten lüften
  {
    digitalWrite(relayVentilator, HIGH);
    previousInterVent = currentTime;             
    previousVent = currentTime;                                                                
  }
}

}
