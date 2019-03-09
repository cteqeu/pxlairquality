#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <Wire.h>

#include <app_defs.h>
#include <stdbool.h>
#include <stdint.h>
#include <DS3231.h>
#include <PM3015.h>
#include <ATT_GPS.h>
#include <ATT_NBIOT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

namespace pxl
{
    class App
    {
        public:
            /* --------- Constructor / Destructor ---------- */
             App(void);
            ~App(void);

            void uploadData    (void);
            void printDebugData(void);

            /* ----------  Getters ---------- */

            /* ----------  Setters ---------- */

        private:
            /* ---------- Class variables  ----------  */
            uint32_t pm1_0;
            uint32_t pm2_0;
            uint32_t pm10;

            ATT_NBIOT       device;
            ATT_GPS         gps;
            DS3231          rtc;
            Adafruit_BME280 bme;
            sensors::PM3015 particleSensor;
    };
};

#endif/*APP_H*/
