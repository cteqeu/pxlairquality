#include "App.h"

pxl::App::App(void) :
    gps            {APP_DEFS_GPS_RX_PIN, APP_DEFS_GPS_TX_PIN},
    device         {},
    rtc            {},
    bme            {},
    particleSensor {}
{
    /* Nothing yet */
}

pxl::App::~App(void)
{
    /* Nothing yet */
}

void
pxl::App::printDebugData(void)
{
    /* Clear the terminal screen */
    APP_DEFS_DEBUG_STREAM.write(27);
    APP_DEFS_DEBUG_STREAM.print("[2J");
    APP_DEFS_DEBUG_STREAM.write(27);
    APP_DEFS_DEBUG_STREAM.print("[H");

    /* Print the name of the program */
    APP_DEFS_DEBUG_STREAM.println("/* -------------------------------------- */");
    APP_DEFS_DEBUG_STREAM.println("/* ---------- PXL AIR QUALLITY ---------- */");
    APP_DEFS_DEBUG_STREAM.println("/* ------------ DEBUG SCREEN ------------ */");
    APP_DEFS_DEBUG_STREAM.println("/* -------------------------------------- */");


    /* Print out latest sensor readings */
    /*
    APP_DEFS_DEBUG_STREAM.println("Latitude:    " + (const String)gps.latitude);
    APP_DEFS_DEBUG_STREAM.println("Longitude:   " + (const String)gps.longitude);
    APP_DEFS_DEBUG_STREAM.println("Temperature: " + (const String)bme.readTemperature()        + "*C");
    APP_DEFS_DEBUG_STREAM.println("Humidity:    " + (const String)bme.readHumidity()           + "%");
    APP_DEFS_DEBUG_STREAM.println("Pressure:    " + (const String)(bme.readPressure() / 100.f) + "hPa");
    APP_DEFS_DEBUG_STREAM.println("PM1.0: " + (const String)pm1_0 + " µg/m^3");
    APP_DEFS_DEBUG_STREAM.println("PM2.5: " + (const String)pm2_5 + " µg/m^3");
    APP_DEFS_DEBUG_STREAM.println("PM10:  " + (const String)pm10  + " µg/m^3");
    */

    return;
}

void
pxl::App::uploadData(void)
{
    /*
    payload.reset();

    payload.map(APP_DEFS_N_SENSORS);
    payload.addParticle(pm1_0, pm2_5, pm10,                   "particle-sensor");
    payload.addGPS(gps.latitude, gps.longitude, gps.altitude, "gps");
    payload.addNumber(bme.readTemperature(),                  "temperature");
    payload.addNumber(bme.readHumidity(),                     "humidity");
    payload.addNumber(bme.readPressure() / 100.0f,            "pressure");

    payload.send();
    */

    return;
}
