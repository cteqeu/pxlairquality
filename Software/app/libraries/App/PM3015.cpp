#include "PM3015.h"

sensors::PM3015::PM3015() : particleSerial(APP_DEFS_PARTICLE_RX_PIN, APP_DEFS_PARTICLE_TX_PIN)
{
    baudRate = APP_DEFS_PARTICLE_BAUD;
    particleSerial.begin(baudRate);
    particleSerial.setTimeout(1000);
}

sensors::PM3015::~PM3015()
{

}

uint8_t sensors::PM3015::openMeasurement()
{
    uint8_t openResponse[5];
    memset(openResponse, 0, sizeof(openResponse));



    return;
}


uint8_t sensors::PM3015::closeMeasurement()
{
    return;
}
