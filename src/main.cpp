/******************************************************************************
 * @file      main.cpp
 *
 * @creator   Christoph Seiler
 * @created   2020
 *
 * @brief     TODO: Short description of this module
 *
 * $Id$
 *
 * $Revision$
 *
 ******************************************************************************/
 //192.168.31.173
 /* ****************************************************************************/
 /* ******************************** INCLUDES **********************************/
 /* ****************************************************************************/

#include <Arduino.h>
#include <time.h>
#include <Adafruit_I2CDevice.h>
#include <DHT.h>              // Sensor DHT22
#include <WiFi.h>             // WiFi
#include <FirebaseESP32.h>    // Google Firebase
#include "FS.h"
#include <ESPmDNS.h>          //OTA Update 1
#include <WiFiUdp.h>          //OTA Update 2
#include <ArduinoOTA.h>       //OTA Update 3

/* ****************************************************************************/
/* *************************** DEFINES AND MACROS *****************************/
/* ****************************************************************************/

//defines for I/O pins
#define PIN_DHT1              25
#define PIN_DHT2              26
#define RELAY_HUMIDIFIER      18
#define RELAY_VENTILATOR      19
#define RELAY_HEATMAT         5
#define RELAY_REMOTE          17 

#define WIFI_NETWORK          "********"
#define WIFI_PASSWORD         "*****"
#define WIFI_TIMEOUT_MS       20000
#define FIREBASE_HOST         "*********"
#define FIREBASE_AUTH         "********************************"

//define NTP server
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

//define firebase data objects
FirebaseData firebaseData;    //firebase data for relais
FirebaseData firebaseData2;   //firebase data for sensors
FirebaseData firebaseData3;   //firebase data for max and min value
FirebaseJsonArray arr1;       //array for relais
FirebaseJsonArray arr2;       //array for sensors
FirebaseJsonArray arr3;       //array fot max and min value
String pathRelais = "/Relais";
String pathSensors = "/Sensors";
String pathValues = "/Values";

//initialize DHT
#define DHTTYPE DHT22
DHT dht(PIN_DHT1, DHTTYPE);
DHT dht2(PIN_DHT2, DHTTYPE);

/* ****************************************************************************/
/* ******************************** VARIABLES *********************************/
/* ****************************************************************************/

//define states
bool statusVentilator = false;
bool statusHumidifier = false;
bool statusHeatmat = false;
bool statusStandby = false;
bool statusPumpCooling = true;
bool statusRemote = true;
bool jsonVentilator;
bool jsonHumidifier;
bool jsonHeatmat;
bool jsonStandby;
bool jsonReboot = false;
bool jsonRemote;
bool ventilate;
bool fanDelay;
bool finishedHumidifying = true;
bool ON = LOW;
bool OFF = HIGH;
bool forceStandby = false;

//define limits for humditiy check -> can be changed in app or via touch
int hHighLimit;
int hLowLimit;
int offsetHumidity = 8;
float h = 0;
float t = 0;
float h1;
float h2;
float t1;
float t2;

//setup timestamps and intervalls
const long timerCheckSensors = 10000;
const long timerIdleVentilation = 1800000;
const long timerActiveVentilation = 300000;
const long timerFanDelay = 20000;
const long timerPump = 6000;
const long timerPumpCooling = 90000;
const long timerCheckTime = 600000;
unsigned long previousCheckSensors = 0;
unsigned long previousIdleVentilation = 0;
unsigned long previousActiveVentilation = 0;
unsigned long previousFanDelay = 0;
unsigned long previousPump = 0;
unsigned long previousPumpCooling = 0;
unsigned long previousCheckTime = 0;
unsigned long currentTime;

int localTime = 0;
uint8_t localHour;
int remoteOn;
int remoteOff;

void changeRelais(String relaisItem, int PIN, bool onoff);
void updateFirebase(void);
void actionDHT(void);
void subscribeFirebase(FirebaseData& data);
void subscribeFirebaseLimit(FirebaseData& data);
void initiatePins(void);
void standby(bool activation);
void updateDHT(void);
void connectWiFi(void);
void initiateFirebase(void);
void updateFirebaseSensors(void);
void getLocalTime(void);
void startOTA(void);
void reboot(void);
void timerRemote(void);

void setup()
{
    Serial.begin(115200);

    connectWiFi();

    startOTA();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    initiateFirebase();

    initiatePins();

    dht.begin();
    dht2.begin();

    updateFirebase();
}

void loop()
{
    currentTime = millis();

    ArduinoOTA.handle();

    if (statusPumpCooling == true)
    {
        if ((currentTime - previousPumpCooling) > timerPumpCooling)
        {
            previousPumpCooling = currentTime;
            statusPumpCooling = false;
            Serial.println("Cooling finished!");
            updateFirebase();
        }
        else
        {
            Serial.println("Cooling timer still active!");
        }
    }
    else {}
    if (statusStandby == false)
    {
        if ((currentTime - previousCheckSensors) > timerCheckSensors)
        {
            previousCheckSensors = currentTime;

            updateDHT();
            if (statusPumpCooling == false)
            {
                actionDHT();
            }
            else {}

            updateFirebaseSensors();
        }
        else {}
        //fan delay
        if ((fanDelay == true) && (currentTime - previousFanDelay) > timerFanDelay)
        {
            changeRelais("Ventilator", RELAY_VENTILATOR, OFF);
            updateFirebase();
            fanDelay = false;
        }
        else {}
        //ventilation cycle
        if ((finishedHumidifying == true) && (currentTime - previousIdleVentilation) > timerIdleVentilation)
        {
            changeRelais("Ventilator", RELAY_VENTILATOR, ON);
            updateFirebase();
            finishedHumidifying = false;
            ventilate = true;
            previousActiveVentilation = currentTime;
        }
        else {}
        //ventilation timer
        if ((ventilate == true) && (currentTime - previousActiveVentilation) > timerActiveVentilation)
        {
            changeRelais("Ventilator", RELAY_VENTILATOR, OFF);
            updateFirebase();
            finishedHumidifying = true;
            ventilate = false;
            previousIdleVentilation = currentTime;
        }
        else {}
    }
    else
    {
        if (digitalRead(RELAY_HUMIDIFIER) == ON)
        {
            changeRelais("Humidifier", RELAY_HUMIDIFIER, OFF);
            Serial.println("Standby aktiviert!");
            updateFirebase();
        }
        else {}
        if ((currentTime - previousCheckSensors) > timerCheckSensors)
        {
            previousCheckSensors = currentTime;
            updateDHT();

            updateFirebaseSensors();
        }
        else {}
    }
    if ((currentTime - previousCheckTime) > timerCheckTime)
    {
        timerRemote();
        if (localHour >= remoteOn && localHour < remoteOff)
        {
            changeRelais("Remote", RELAY_REMOTE, ON);
        }
        else
        {
            changeRelais("Remote", RELAY_REMOTE, OFF);
        }
        previousCheckTime = currentTime;
        updateFirebase();
    }
    else {}
    subscribeFirebase(firebaseData);
    subscribeFirebaseLimit(firebaseData3);
}

//////////////////////////////////////////////////
/////////////////LOOP FUNCTIONS///////////////////
//////////////////////////////////////////////////

void actionDHT(void)
{
    if (digitalRead(RELAY_HUMIDIFIER) == ON)
    {
        if ((currentTime - previousPump) > timerPump)
        {
            changeRelais("Humidifier", RELAY_HUMIDIFIER, OFF);
            previousFanDelay = currentTime;
            previousIdleVentilation = currentTime;
            fanDelay = true;
            finishedHumidifying = true;
            Serial.println("Finished humidifying - turning off!");
            Serial.println("Timer set for fan delay & scheduled ventilation!");
            Serial.println("Timer set for pump cooling.");
            updateFirebase();
        }
        else
        {
            Serial.println("6 Seconds haven't passed yet - pump still on!");
        }
    }
    else
    {
        if ((h <= hLowLimit) && (statusPumpCooling == false))
        {
            finishedHumidifying = false;
            changeRelais("Ventilator", RELAY_VENTILATOR, ON);
            changeRelais("Humidifier", RELAY_HUMIDIFIER, ON);
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
    if (relaisItem == "Ventilator")
    {
        if (onoff == HIGH)
        {
            statusVentilator = false;
        }
        else
        {
            statusVentilator = true;
        }
    }
    else if (relaisItem == "Humidifier")
    {
        if (onoff == HIGH)
        {
            statusHumidifier = false;
            statusPumpCooling = true;
            previousPumpCooling = currentTime;
            Serial.println("Starting cooling timer!");
        }
        else
        {
            if (statusPumpCooling == false)
            {
                statusHumidifier = true;
            }
            else {}
        }
    }
    else if (relaisItem == "Heatmat")
    {
        if (onoff == HIGH)
        {
            statusHeatmat = false;
        }
        else
        {
            statusHeatmat = true;
            getLocalTime();
            updateFirebase();
        }
    }
    else if (relaisItem == "Remote")
    {
        if (onoff == HIGH)
        {
            statusRemote = false;
        }
        else
        {
            statusRemote = true;
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
    if (activation == true)
    {
        statusStandby = true;
    }
    else
    {
        statusStandby = false;
    }

}

void updateFirebase(void)
{
    arr1.clear();
    arr1.set("/[0]", statusVentilator);
    arr1.set("/[1]", statusHumidifier);
    arr1.set("/[2]", statusHeatmat);
    arr1.set("/[3]", statusStandby);
    arr1.set("/[4]", localTime);
    arr1.set("/[5]", statusPumpCooling);
    arr1.set("/[6]", jsonReboot);
    arr1.set("/[7]", statusRemote);
    if (Firebase.set(firebaseData, pathRelais, arr1))
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
    if (Firebase.set(firebaseData2, pathSensors, arr2))
    {
        Serial.println("Firebase Sensors updated!");
    }
    else
    {
        Serial.println("Firebase sensor update failed!");
        Serial.println("Reason: " + firebaseData2.errorReason());
    }
}

void subscribeFirebase(FirebaseData& data)
{
    if (Firebase.get(firebaseData, pathRelais))
    {
        FirebaseJsonArray& arr1 = data.jsonArray();
        String arrStr;
        arr1.toString(arrStr, true);
        FirebaseJsonData& jsonData = data.jsonData();
        arr1.get(jsonData, 0);
        if (jsonData.stringValue == "true")
        {
            jsonVentilator = true;
        }
        else
        {
            jsonVentilator = false;
        }
        if (jsonVentilator != statusVentilator)
        {
            if (jsonVentilator == true)
            {
                changeRelais("Ventilator", RELAY_VENTILATOR, ON);
            }
            else
            {
                changeRelais("Ventilator", RELAY_VENTILATOR, OFF);
            }
        }
        else {}
        arr1.get(jsonData, 1);
        if (jsonData.stringValue == "true")
        {
            jsonHumidifier = true;
        }
        else
        {
            jsonHumidifier = false;
        }
        if (jsonHumidifier != statusHumidifier)
        {
            if (jsonHumidifier == true)
            {
                changeRelais("Humidifier", RELAY_HUMIDIFIER, ON);
            }
            else
            {
                changeRelais("Humidifier", RELAY_HUMIDIFIER, OFF);
            }
        }
        else {}
        arr1.get(jsonData, 2);
        if (jsonData.stringValue == "true")
        {
            jsonHeatmat = true;
        }
        else
        {
            jsonHeatmat = false;
        }
        if (jsonHeatmat != statusHeatmat)
        {
            if (jsonHeatmat == true)
            {
                changeRelais("Heatmat", RELAY_HEATMAT, ON);
            }
            else
            {
                changeRelais("Heatmat", RELAY_HEATMAT, OFF);
            }
        }
        else {}
        arr1.get(jsonData, 3);
        if (jsonData.stringValue == "true")
        {
            jsonStandby = true;
        }
        else
        {
            jsonStandby = false;
        }
        if (jsonStandby != statusStandby)
        {
            if (jsonStandby == true)
            {
                standby(true);
            }
            else
            {
                standby(false);
            }
        }
        else {}
        arr1.get(jsonData, 6);
        if (jsonData.stringValue == "true")
        {
            reboot();
        }
        else {}
        arr1.get(jsonData, 7);
        if (jsonData.stringValue == "true")
        {
            jsonRemote = true;
        }
        else
        {
            jsonRemote = false;
        }
        if (jsonRemote != statusRemote)
        {
            if (jsonRemote == true)
            {
                changeRelais("Remote", RELAY_REMOTE, ON);
            }
            else
            {
                changeRelais("Remote", RELAY_REMOTE, OFF);
            }
        }
        else {}
    }

}

void subscribeFirebaseLimit(FirebaseData& data)
{
    if (Firebase.get(firebaseData3, pathValues))
    {
        FirebaseJsonArray& arr3 = data.jsonArray();
        String arrStr;
        int hMaxStr;
        int hMinStr;
        int rOnStr;
        int rOffStr;
        arr3.toString(arrStr, true);
        FirebaseJsonData& jsonData = data.jsonData();
        arr3.get(jsonData, 0);
        hMaxStr = jsonData.stringValue.toInt();
        if (hHighLimit != hMaxStr)
        {
            hHighLimit = hMaxStr;
            Serial.println("Setting new high limit!");
            Serial.println(hHighLimit);
        }
        else {}
        arr3.get(jsonData, 1);
        hMinStr = jsonData.stringValue.toInt();
        if (hLowLimit != hMinStr)
        {
            hLowLimit = hMinStr;
            Serial.println("Setting new low limit!");
            Serial.println(hLowLimit);
        }
        else {}
        arr3.get(jsonData, 2);
        rOnStr = jsonData.stringValue.toInt();
        if (remoteOn != rOnStr)
        {
            remoteOn = rOnStr;
            Serial.println("Setting new switch on time!");
            Serial.print(remoteOn);
        }
        else {}
        arr3.get(jsonData, 3);
        rOffStr = jsonData.stringValue.toInt();
        if (remoteOff != rOffStr)
        {
            remoteOff = rOffStr;
            Serial.println("Setting new switch off time!");
            Serial.println(remoteOff);
        }
        else {}
    }
    else {}
}

void updateDHT(void)
{
    //get temperature and humidity
    Serial.println("Updating DHT22 data!");
    h1 = dht.readHumidity();
    h2 = dht2.readHumidity();
    t1 = dht.readTemperature();
    t2 = dht2.readTemperature();
    if (isnan(t1) || isnan(t2) || isnan(h1) || isnan(h2))
    {
        Serial.println("DHT22 sensor error! Please check the connections and restart the device!");
        // statusStandby = true;
        // forceStandby = true;
    }
    else
    {
        //if(forceStandby == true)
        //{
        //  forceStandby = false;
        //  statusStandby = false;
        //}
        //else{}
        h = ((h1 + h2) / 2) - offsetHumidity;
        t = (t1 + t2) / 2;
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.println(" %");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" °C");
    }
}

void getLocalTime(void)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    else {}
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    localTime = ((timeinfo.tm_hour * 3600) + (timeinfo.tm_min * 60) + (timeinfo.tm_sec));
}

void timerRemote(void)
{
    struct tm timeRemote;
    if (!getLocalTime(&timeRemote))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    else {}
    localHour = timeRemote.tm_hour;
}

void reboot(void)
{
    Serial.println("Rebooting!");
    ESP.restart();
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

    if (!Firebase.beginStream(firebaseData, pathRelais) || !Firebase.beginStream(firebaseData, pathSensors))
    {
        Serial.println("Can't begin Firebase connection!");
        Serial.println("Reason: " + firebaseData.errorReason());
    }
}

void initiatePins(void)
{
    //define relays and turn off (for safety)
    pinMode(RELAY_HUMIDIFIER, OUTPUT);
    pinMode(RELAY_VENTILATOR, OUTPUT);
    pinMode(RELAY_HEATMAT, OUTPUT);
    pinMode(RELAY_REMOTE, OUTPUT);
    digitalWrite(RELAY_HUMIDIFIER, HIGH),
        digitalWrite(RELAY_VENTILATOR, HIGH);
    digitalWrite(RELAY_HEATMAT, HIGH);
    digitalWrite(RELAY_REMOTE, LOW);
}

void startOTA(void)
{
    ArduinoOTA
        .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
            })
        .onEnd([]() {
                Serial.println("\nEnd");
            })
                .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
                    })
                .onError([](ota_error_t error) {
                        Serial.printf("Error[%u]: ", error);
                        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                        else if (error == OTA_END_ERROR) Serial.println("End Failed");
                    });

                    ArduinoOTA.begin();
}
