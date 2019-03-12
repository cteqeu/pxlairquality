#include "PM3015.h"
#include <stdint.h>

/* ----- Global Variables ----- */
static uint32_t PM1_0 = 0;
static uint32_t PM2_5 = 0;
static uint32_t PM10  = 0;
static sensors::PM3015 *particleSensor = new sensors::PM3015;

#define DEBUG_STREAM Serial
#define DELAY_TIME   2000

void
setup(void)
{
    /* put your setup code here, to run once: */
    delay(500);

    pinMode(GROVEPWR, OUTPUT);
    digitalWrite(GROVEPWR, HIGH);
    
    DEBUG_STREAM.begin(57600u);
    particleSensor->openMeasurement();
    
    return;
}

void
loop(void)
{
    /* put your main code here, to run repeatedly: */
    particleSensor->readMeasurements(&PM1_0, &PM2_5, &PM10);

    DEBUG_STREAM.println("PM1.0: " + (const String)PM1_0 + " µg/m^3");
    DEBUG_STREAM.println("PM2.5: " + (const String)PM2_5 + " µg/m^3");
    DEBUG_STREAM.println("PM10:  " + (const String)PM10  + " µg/m^3");
    DEBUG_STREAM.println("|=======================================|");

    delay(DELAY_TIME);
    
    return;
}
