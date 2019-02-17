#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>
#include <app_defs.h>
#include <ATT_GPS.h>
#include <ATT_NBIOT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DS3231.h>

namespace app
{
    typedef struct DateTime
    {
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t dow;
        uint8_t hours;
        uint8_t Minutes;
        uint8_t seconds;
    } dateTime_t;

    class App
    {
        public:
            uploadData();
            printDebugData();

        private:
            ATT_NBIOT       device;
            ATT_GPS         gps(20u, 21u);
            DS3231          rtc;
            Adafruit_BME280 bme;
            dateTime_t      dateTime;

            static uint32_t curTime = 0u;
            static uint32_t ctime   = 0u;
            static bool     century = false;
    };
};

#endif/*APP_H*/
