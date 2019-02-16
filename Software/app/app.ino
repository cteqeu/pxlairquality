#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <app_defs.h>
#include <ATT_GPS.h>
#include <ATT_NBIOT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define DEBUG_STREAM      APP_DEFS_DEBUG_STREAM
#define MODEM_STREAM      APP_DEFS_MODEM_STREAM
#define MODEM_ON_OFF_PIN  APP_DEFS_MODEM_ON_OFF_PIN

#define DELAY_TIME        60000u

/* --------- GLOBALS --------- */
void uploadData();
void printDebugData();

ATT_NBIOT       device;
ATT_GPS         gps(20u, 21u);
Adafruit_BME280 bme;
/* --------------------------- */

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
    delay(500u);
    pinMode(LED1, OUTPUT);
    digitalWrite(LED1, HIGH);

    volatile static bool err = false;
    DEBUG_STREAM.begin(57600u);
    MODEM_STREAM.begin(9600u);

    DEBUG_STREAM.println("/* ---------- INITIALIZING AND CONNECTING ---------- */");

    /* Initialize and connect to the uBlox SARA-N211 */
    device.init(MODEM_STREAM, DEBUG_STREAM, MODEM_ON_OFF_PIN);
    
    err = device.connect();
    if (err == true)
    {
        DEBUG_STREAM.println("/* ---------- CONNECTED ---------- */");
    }
    else
    {
        DEBUG_STREAM.println("/* ---------- CONNECTION FAILED ---------- */");
        DEBUG_STREAM.println("Failed to connect to device");
        delay(100u); /* Using a delay here otherwise the Serial stream will not have enough time to print the message */
        exit(EXIT_FAILURE); 
    }

    /* Initialize the GPS module */
    err = gps.readCoordinates(30u);
    if (err == true)
    {
        DEBUG_STREAM.println("Coordinates read successfully.");
    }
    else
    {
        DEBUG_STREAM.println("Failed to read coordinates, sending defaults.");
    }

    /* Initialize TPH v2 module */
    err = bme.begin();
    if (err == true)
    {
        DEBUG_STREAM.println("BME sensor found");
    }
    else
    {
        DEBUG_STREAM.println("BME sensor not found");
        delay(100u);
        exit(EXIT_FAILURE);
    }

    DEBUG_STREAM.println("/* --------- SETUP COMPLETE ---------- */");

    /* Print the name of the program */
    DEBUG_STREAM.println("/* -------------------------------------- */");
    DEBUG_STREAM.println("/* ---------- PXL AIR QUALLITY ---------- */");
    DEBUG_STREAM.println("/* -------------------------------------- */");

    digitalWrite(LED1, LOW);

    return;
}

void
loop()
{
    /* put your main code here, to run repeatedly: */
    gps.readCoordinates();
    uploadData();
    printDebugData();

    delay(DELAY_TIME);
    
    return;
}

void
uploadData()
{
    digitalWrite(LED1, HIGH);
    DEBUG_STREAM.println("Upload starting");
    payload.reset();

    payload.map(4);
    payload.addGPS(gps.latitude, gps.longitude, gps.altitude, "gps");
    payload.addNumber(bme.readTemperature(),                  "temperature");
    payload.addNumber(bme.readHumidity(),                     "humidity");
    payload.addNumber(bme.readPressure() / 100.0f,            "pressure");

    payload.send();
    
    DEBUG_STREAM.println("Upload completed");
    digitalWrite(LED1, LOW);

    return;
}

void
printDebugData()
{
    DEBUG_STREAM.write(27);
    DEBUG_STREAM.print("[2J");
    DEBUG_STREAM.write(27);
    DEBUG_STREAM.print("[H");

    /* Print the name of the program */
    DEBUG_STREAM.println("/* -------------------------------------- */");
    DEBUG_STREAM.println("/* ---------- PXL AIR QUALLITY ---------- */");
    DEBUG_STREAM.println("/* ------------ DEBUG SCREEN ------------ */");
    DEBUG_STREAM.println("/* -------------------------------------- */");

    DEBUG_STREAM.println("Latitude:    " + (const String)gps.latitude);
    DEBUG_STREAM.println("Longitude:   " + (const String)gps.longitude);
    DEBUG_STREAM.println("Temperature: " + (const String)bme.readTemperature()        + "*C");
    DEBUG_STREAM.println("Humidity:    " + (const String)bme.readHumidity()           + "%");
    DEBUG_STREAM.println("Pressure:    " + (const String)(bme.readPressure() / 100.f) + "hPa");

    return;
}
