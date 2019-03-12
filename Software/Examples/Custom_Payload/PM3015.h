#ifndef PM3015_H
#define PM3015_H

#include <Arduino.h>
#include <SoftwareSerial.h>

#include <stdint.h>
#include "conf.h"

namespace sensors
{
    enum commands {
        PM3015_READ_PARTICLE_MEASUREMENT_RESULT = 0u,
        PM3015_OPEN_PARTICLE_MEASUREMENT        = 1u,
        PM3015_CLOSE_PARTICLE_MEASUREMENT       = 2u,
        PM3015_SET_UP_MEASURING_TIME            = 3u,
        PM3015_READ_MEASURING_TIME              = 4u,
        PM3015_SET_UP_TIMING_MEASURING_MODE     = 5u,
        PM3015_SET_UP_DYNAMIC_MEASURING_MODE    = 6u,
        PM3015_SET_UP_READ_PARTICLE_CALIBRATED_COEFFICIENT = 7u,
        PM3015_CLOSE_LASER_DIODE                = 8u,
        PM3015_OPEN_LASER_DIODE                 = 9u,
        PM3015_READ_SOFTWARE_VERSION_NUMBER     = 10u,
        PM3015_READ_SERIAL_NUMBER               = 11u,
    };

    class PM3015
    {
        public:
            PM3015(void);
            ~PM3015(void);

            uint8_t getMeasurement(void);
            uint8_t openMeasurement();
            uint8_t closeMeasurement();
            uint8_t readMeasurements(uint32_t *pm1_0, uint32_t *pm2_5, uint32_t *pm10);

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
