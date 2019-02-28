#ifndef APP_H
#define APP_H

#include <Arduino.h>

#include <stdbool.h>
#include <stdint.h>
#include <app_defs.h>
#include <ATT_GPS.h>
#include <ATT_NBIOT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DS3231.h>
#include <PM3015.h>

namespace pxl
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
            /* --------- Constructor / Destructor ---------- */
            App(void);
            ~App(void);

            void uploadData(void);
            void printDebugData(void);

            /* ----------  Getters ---------- */
            dateTime_t getDateTime(void);

            /* ----------  Setters ---------- */
            uint8_t setDateTime(dateTime_t dateTime);

        private:
            /* ---------- Class variables  ----------  */
            static uint32_t curTime;
            static uint32_t ctime;
            static bool     century;
            static uint32_t pm1_0;
            static uint32_t pm2_0;
            static uint32_t pm10;

            ATT_NBIOT       device;
            ATT_GPS         gps;
            DS3231          rtc;
            Adafruit_BME280 bme;
            sensors::PM3015 particleSensor;
    };
};

#endif/*APP_H*/
