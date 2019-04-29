#ifndef APP_H
#define APP_H

#include <app_conf.h>
#define MODEM_STREAM  APP_CONF_MODEM_STREAM
#define DEBUG_STREAM  APP_CONF_DEBUG_STREAM

#include <Arduino.h>
#include <Wire.h>

#include <stdbool.h>
#include <stdint.h>

#include <DS3231.h>
#include <PM3015.h>
#include <ATT_GPS.h>
#include <ATT_NBIOT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <CborBuilder.h>

namespace pxl
{
    class App
    {
        public:
            /* --------- Constructor / Destructor ---------- */
             App(void);
            ~App(void);

            void    setup         (void);
            void    loop          (void);
            void    updateData    (void);
            void    uploadData    (void);
            void    printDebugData(void);
            uint8_t getBattLevel  (void);

            /* ----------  Getters ---------- */

            /* ----------  Setters ---------- */

        private:
            /* ---------- Class variables  ----------  */
            uint32_t pm1_0;
            uint32_t pm2_5;
            uint32_t pm10;

            size_t curTime;
            size_t ctime;
            bool   century;

            ATT_NBIOT       device;
            ATT_GPS         gps;
            DS3231          rtc;
            Adafruit_BME280 bme;
            sensors::PM3015 particleSensor;
            CborBuilder     payload;
    };
};

#endif/*APP_H*/
