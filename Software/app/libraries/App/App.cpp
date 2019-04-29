#include "App.h"

pxl::App::App(void) :
    pm1_0         (0u),
    pm2_5         (0u),
    pm10          (0u),
    curTime       (0u),
    ctime         (0u),
    century       (false),
    device        (),
    gps           (20, 21),
    rtc           (),
    bme           (),
    particleSensor(),
    payload       (device)
{
    /* Nothing yet */
}

pxl::App::~App(void)
{
    /* Nothing yet */
}

void
pxl::App::setup(void)
{
    volatile static bool err = false;

    delay(APP_CONF_STARTUP_DELAY);
    pinMode(LED1,          OUTPUT);
    pinMode(GROVEPWR,      OUTPUT);
    digitalWrite(GROVEPWR, HIGH);

    Wire.begin();
    DEBUG_STREAM.begin(APP_CONF_DEBUG_STREAM_BAUD);
    MODEM_STREAM.begin(APP_CONF_MODEM_STREAM_BAUD);

    DEBUG_STREAM.println("/* ----- INITIALIZING AND CONNECTING ----- */");
    device.init(MODEM_STREAM, DEBUG_STREAM, APP_CONF_MODEM_ON_OFF_PIN);

    err = device.connect();
    if(err != true)
    {
        DEBUG_STREAM.println("/* ---------- CONNECTION FAILED ---------- */");
        DEBUG_STREAM.println("Failed to connect to device");
        delay(100u); /* Using a delay here otherwise the Serial stream will not have enough time to print the message */
        exit(EXIT_FAILURE);
    }
    DEBUG_STREAM.println("/* ---------- CONNECTED ---------- */");

    DEBUG_STREAM.println("/* ---------- GETTING GPS FIX ---------- */");
    err = gps.readCoordinates(10u);
    if(err != true)
    {
        DEBUG_STREAM.println("Failed to read coordinates, sending defaults.");
    }
    DEBUG_STREAM.println("Coordinates read successfully.");

    err = bme.begin();
    if(err != true)
    {
        DEBUG_STREAM.println("BME sensor not found");
    }
    else
    {
        DEBUG_STREAM.println("BME sensor found");
    }

    particleSensor.openMeasurement();

    DEBUG_STREAM.println("/* --------- SETUP COMPLETE ---------- */");

    return;
}

void
pxl::App::loop(void)
{
    updateData();
    uploadData();
    printDebugData();

    return;
}

void
pxl::App::updateData(void)
{
    particleSensor.readMeasurements(&pm1_0, &pm2_5, &pm10);
    //gps.readCoordinates();

    return;
}

void
pxl::App::printDebugData(void)
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

    DEBUG_STREAM.println("BattLevel = " + (const String)(getBattLevel()));

    /* Print out latest sensor readings */
    DEBUG_STREAM.println("Latitude:    " + (const String)gps.latitude);
    DEBUG_STREAM.println("Longitude:   " + (const String)gps.longitude);
    DEBUG_STREAM.println("Temperature: " + (const String)bme.readTemperature()        + "*C");
    DEBUG_STREAM.println("Humidity:    " + (const String)bme.readHumidity()           + "%");
    DEBUG_STREAM.println("Pressure:    " + (const String)(bme.readPressure() / 100.f) + "hPa");
    DEBUG_STREAM.println("PM1.0: " + (const String)pm1_0 + " µg/m^3");
    DEBUG_STREAM.println("PM2.5: " + (const String)pm2_5 + " µg/m^3");
    DEBUG_STREAM.println("PM10:  " + (const String)pm10  + " µg/m^3");

    return;
}

void
pxl::App::uploadData(void)
{

    DEBUG_STREAM.println("/* STARTING UPLOAD */");
    payload.reset();

    payload.map(APP_CONF_N_SENSORS);
    payload.addParticle(pm1_0, pm2_5, pm10,                   "particle_sensor");
    payload.addGPS(gps.latitude, gps.longitude, gps.altitude, "gps");
    payload.addNumber(bme.readTemperature(),                  "temperature");
    payload.addNumber(bme.readHumidity(),                     "humidity");
    payload.addNumber(bme.readPressure() / 100.0f,            "pressure");

    payload.send();

    return;
}

uint8_t
pxl::App::getBattLevel(void)
{
    uint8_t level = analogRead(A6);

    return level;
}
