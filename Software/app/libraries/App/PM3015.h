#ifndef PM3015_H
#define PM3015_H

#include <Arduino.h>
#include <stdint.h>
#include <SoftwareSerial.h>
#include <app_defs.h>

namespace sensors
{
    enum commands {
        PM3015_READ_MEASUREMENT = 0x0u,
        PM3015_OPEN_MEASUREMENT,
        PM3015_CLOSE_MEASUREMENT,
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
