#include "PM3015.h"

PM3015::PM3015(uint16_t rx, uint16_t tx, uint16_t baudRate) : _particleSerial(rx, tx)
{
    _particleSerial.begin(baudRate);
}

PM3015::~PM3015() {}

uint8_t PM3015::openMeasurement()
{
    uint8_t openResponse[5];
    memset(openResponse, 0, sizeof(openResponse));
    sendCommand(PM3015_OPEN_PARTICLE_MEASUREMENT, openResponse, sizeof(openResponse));
    return isDataValid(openResponse, sizeof(openResponse));
}

uint8_t PM3015::closeMeasurement()
{
    uint8_t closeResponse[5];
    memset(closeResponse, 0, sizeof(closeResponse));
    sendCommand(PM3015_CLOSE_PARTICLE_MEASUREMENT, closeResponse, sizeof(closeResponse));
    return isDataValid(closeResponse, sizeof(closeResponse));
}

uint8_t PM3015::readMeasurements(uint32_t* pm1_0, uint32_t* pm2_5, uint32_t* pm10)
{
    uint8_t response[56u];
    uint8_t err = 0u;
    sendCommand(PM3015_READ_PARTICLE_MEASUREMENT_RESULT, response, sizeof(response));

    err = isDataValid(response, sizeof(response));
    if (err == 1u)
    {
        uint8_t  df0        = 2u;
        uint32_t pm1_0grimm = response[df0 + 1] * pow(256, 3) + response[df0 + 2]  * pow(256, 2) + response[df0 + 3]  * 256 + response[df0 + 4];
        uint32_t pm2_5grimm = response[df0 + 5] * pow(256, 3) + response[df0 + 6]  * pow(256, 2) + response[df0 + 7]  * 256 + response[df0 + 8];
        uint32_t pm10grimm  = response[df0 + 9] * pow(256, 3) + response[df0 + 10] * pow(256, 2) + response[df0 + 11] * 256 + response[df0 + 12];
        *pm1_0 = pm1_0grimm;
        *pm2_5 = pm2_5grimm;
        *pm10  = pm10grimm;
        return 1u;
    }
    else
    {
        return false;
    }
}

uint8_t PM3015::isDataValid(const uint8_t *data, size_t size)
{
    uint8_t sum      = 0u;
    uint8_t checksum = 0u;
    uint8_t i        = 0u;

    if (data[0] == 0x16u)
    {
        sum = 0u;
        for (i = 0u; i < size - 1u; i++)
        {
            sum += data[i];
        }
        checksum = 256 - sum;
        return (data[size - 1u] == checksum);
    }
    else
    {
        return 0;
    }
}

void PM3015::sendCommand(PM3015_Command command, uint8_t *response, size_t size)
{
    if(command == PM3015_READ_PARTICLE_MEASUREMENT_RESULT)
    {
        uint8_t commandBuffer[] = { 0x11, 0x02, 0x0B, 0x07, 0xDB };
        _particleSerial.write(commandBuffer, sizeof(commandBuffer));
    }
    else if(command == PM3015_OPEN_PARTICLE_MEASUREMENT)
    {
        uint8_t commandBuffer[] = { 0x11, 0x03, 0x0C, 0x02, 0x1E, 0xC0 };
        _particleSerial.write(commandBuffer, sizeof(commandBuffer));
    }
    else if(command == PM3015_CLOSE_PARTICLE_MEASUREMENT)
    {
        uint8_t commandBuffer[] = { 0x11, 0x03, 0x0C, 0x01, 0x1E, 0xC1 };
        _particleSerial.write(commandBuffer, sizeof(commandBuffer));
    }
    else
    {
        return;
    }

    _particleSerial.readBytes(response, size);
    return;
}
