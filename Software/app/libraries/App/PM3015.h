#ifndef PM3015_H
#define PM3015_H

#include <Arduino.h>
#include <stdint.h>
#include <SoftwareSerial.h>
#include <app_conf.h>

namespace sensors
{
    enum commands {
        READ_PARTICLE_MEASUREMENT_RESULT            = 0u,
        OPEN_PARTICLE_MEASUREMENT                   = 1u,
        CLOSE_PARTICLE_MEASUREMENT                  = 2u,
        SET_UP_MEASURING_TIME                       = 3u,
        READ_MEASURING_TIME                         = 4u,
        SET_UP_TIMING_MEASURING_MODE                = 5u,
        SET_UP_DYNAMIC_MEASURING_MODE               = 6u,
        SET_UP_READ_PARTICLE_CALIBRATED_COEFFICIENT = 7u,
        CLOSE_LASER_DIODE                           = 8u,
        OPEN_LASER_DIODE                            = 9u,
        READ_SOFTWARE_VERSION_NUMBER                = 10u,
        READ_SERIAL_NUMBER                          = 11u,
    };

    class PM3015
    {
        public:
            PM3015(void);
            ~PM3015(void);

            uint8_t getMeasurement(void);
            uint8_t openMeasurement();
            uint8_t closeMeasurement();
            uint8_t readMeasurements(uint32_t *pm1_0, \
                                     uint32_t *pm2_5, \
                                     uint32_t *pm10   \
                                    );

        private:
            uint8_t isDataValid(const uint8_t *data, size_t size);
            void    sendCommand(commands command,  \
                                uint8_t *response, \
                                size_t size        \
                               );

            SoftwareSerial particleSerial;
            uint16_t baudRate;
    };
};

#endif/*PM3015_H*/
