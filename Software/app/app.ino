//#include <App.h>
#include <PM3015.h>
//#include <app_defs.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <ATT_GPS.h>
#include <ATT_NBIOT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DS3231.h>

#define DEBUG_STREAM      Serial
#define MODEM_STREAM      Serial1
#define MODEM_ON_OFF_PIN  23u

#define DELAY_TIME        3000u
#define STARTUP_DELAY     500u

/* --------- GLOBALS --------- */
void uploadData();
void printDebugData();

ATT_NBIOT       device;
ATT_GPS         gps(20u, 21u);
DS3231          rtc;
Adafruit_BME280 bme;
sensors::PM3015 particleSensor;

static uint32_t curTime = 0u;
static uint32_t ctime   = 0u;
static bool     century = false;

static uint32_t pm1_0 = 0u;
static uint32_t pm2_5 = 0u;
static uint32_t pm10  = 0u;
/* --------------------------- */

#define APP_DEFS_USE_CBOR  1u

#if (APP_DEFS_USE_CBOR == 1u)
    #include <CborBuilder.h>
    CborBuilder payload(device);
#elif (APP_DEFS_USE_BINARY == 1u)
    #include <PayloadBuilder.h>
    PayloadBuilder payload(device);
#else
    #error At least one way to format the data has to be defined!
#endif

void
setup()
{
    /* put your setup code here, to run once: */
    delay(STARTUP_DELAY);
    pinMode(LED1,     OUTPUT);
    pinMode(GROVEPWR, OUTPUT);
    digitalWrite(GROVEPWR, HIGH);

    volatile static bool err = false;

    Wire.begin();
    DEBUG_STREAM.begin(57600u);
    MODEM_STREAM.begin(9600u);

    DEBUG_STREAM.println("/* ---------- INITIALIZING AND CONNECTING ---------- */");

    particleSensor.openMeasurement();

    DEBUG_STREAM.println("/* --------- SETUP COMPLETE ---------- */");

    return;
}

void
loop()
{
    /* put your main code here, to run repeatedly: */
    curTime = millis() + DELAY_TIME;

    if (curTime > (ctime + DELAY_TIME))
    {
        particleSensor.readMeasurements(&pm1_0, &pm2_5, &pm10);
        printDebugData();

        ctime = curTime;
    }
    
    return;
}

void
printDebugData()
{
    /* Clear the terminal screen */
    DEBUG_STREAM.write(27);
    DEBUG_STREAM.print("[2J");
    DEBUG_STREAM.write(27);
    DEBUG_STREAM.print("[H");

    /* Print the name of the program */
    DEBUG_STREAM.println("/* -------------------------------------- */");
    DEBUG_STREAM.println("/* ---------- PXL AIR QUALLITY ---------- */");
    DEBUG_STREAM.println("/* ------------ DEBUG SCREEN ------------ */");
    DEBUG_STREAM.println("/* -------------------------------------- */");

    /* Print out latest sensor readings */
    DEBUG_STREAM.println("PM1.0: " + (const String)pm1_0 + " µg/m^3");
    DEBUG_STREAM.println("PM2.5: " + (const String)pm2_5 + " µg/m^3");
    DEBUG_STREAM.println("PM10:  " + (const String)pm10  + " µg/m^3");
    return;
}
