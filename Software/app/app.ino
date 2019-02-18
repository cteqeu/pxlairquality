#include <app_defs.h>
#include <stdint.h>
#include <ATT_GPS.h>
#include <ATT_NBIOT.h>
#include <Wire.h>
#include <stdio.h>
#include <stdbool.h>

#define DEBUG_STREAM      APP_DEFS_DEBUG_STREAM
#define MODEM_STREAM      APP_DEFS_MODEM_STREAM
#define MODEM_ON_OFF_PIN  APP_DEFS_MODEM_ON_OFF_PIN

#if (APP_DEFS_USE_CBOR == 1u)
#include <CborBuilder.h>
#endif

#if (APP_DEFS_USE_BINARY == 1u)
#include <PayloadBuilder.h>
#endif

/* --------- GLOBALS --------- */
ATT_NBIOT device;
ATT_GPS   gps(20, 21);
/* --------------------------- */

void
setup() {
    /* put your setup code here, to run once: */
    delay(500);

    volatile static bool err = false;

    DEBUG_STREAM.begin(57600);
    MODEM_STREAM.begin(9600);

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
        delay(100);
        exit(EXIT_FAILURE); 
    }

    /* Print the name of the program */
    DEBUG_STREAM.println("/* -------------------------------------- */");
    DEBUG_STREAM.println("/* ---------- PXL AIR QUALLITY ---------- */");
    DEBUG_STREAM.println("/* -------------------------------------- */");

    /* Initialize the GPS module */
    err = gps.readCoordinates(30);
    if (err == true)
    {
        DEBUG_STREAM.println("Sending initial fix");
    }
    else
    {
        DEBUG_STREAM.println("No fix found. sending default");
    }

    DEBUG_STREAM.println("/* --------- SETUP COMPLETE ---------- */");

    return;
}

void
loop() {
    /* put your main code here, to run repeatedly: */
    gps.readCoordinates();

    DEBUG_STREAM.println("Latitude:   " + (const String)gps.latitude);
    DEBUG_STREAM.println("Longitude:  " + (const String)gps.longitude);

    delay(500);
    
    return;
}
