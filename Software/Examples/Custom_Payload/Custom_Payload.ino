#include "PM3015.h"
#include "conf.h"

#include <CborBuilder.h>
#include <ATT_NBIOT.h>
#include <stdint.h>
#include <stdlib.h>

/* ----- Function Prototypes ----- */
void readSensorValues  (void);
void uploadSensorValues(void);
void printSensorValues (void);

/* ----- Global Variables ----- */
static uint32_t PM1_0 = 0;
static uint32_t PM2_5 = 0;
static uint32_t PM10  = 0;

static sensors::PM3015 *particleSensor = new sensors::PM3015;
static ATT_NBIOT       *device         = new ATT_NBIOT;
static CborBuilder     *payload        = new CborBuilder(*device);

void
setup(void)
{
    /* put your setup code here, to run once: */
    delay(500);
  
    pinMode     (GROVEPWR, OUTPUT);
    digitalWrite(GROVEPWR, HIGH);
  
    DEBUG_STREAM.begin(57600);
    MODEM_STREAM.begin(9600);
  
    device->init(MODEM_STREAM, DEBUG_STREAM, MODEM_ON_OFF_PIN);
  
    if (device->connect() < 1)
    {
        DEBUG_STREAM.println("Connection failed!");
        delay(500);
        exit(EXIT_FAILURE);
    }
    DEBUG_STREAM.println("Connected!");
  
    particleSensor->openMeasurement();
  
    return;
}

void
loop(void)
{
    /* put your main code here, to run repeatedly: */
    
    readSensorValues();
    printSensorValues();
    uploadSensorValues();
  
    delay(DELAY_TIME);
  
    return;
}

void
readSensorValues(void)
{
    particleSensor->readMeasurements(&PM1_0, &PM2_5, &PM10);
    
    return;
}

void
uploadSensorValues(void)
{
    payload->reset();
    payload->map(1);
    payload->addParticle(PM1_0, PM2_5, PM10, (const String)"particle-sensor");
    payload->send();

    return;
}

void
printSensorValues(void)
{
    DEBUG_STREAM.println("PM1.0: " + (const String)PM1_0 + " µg/m^3");
    DEBUG_STREAM.println("PM2.5: " + (const String)PM2_5 + " µg/m^3");
    DEBUG_STREAM.println("PM10:  " + (const String)PM10  + " µg/m^3");
    DEBUG_STREAM.println("|=======================================|");
    
    return;
}
