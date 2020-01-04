#include "Air_Quality_Sensor.h"
#include "PM3015.h"
#include "TinyGPS++.h"
#include "BME280.h"
#include <stdint.h>

// Analog Sensor
AirQualitySensor airQualitySensor(A4);

// Laser Sensor
PM3015 particleSensor(4, 5, 9600);

// GPS Sensor
TinyGPSPlus GPS;

// TPH Sensor
BME280_Class BME280;

// Globals
uint32_t lastUpdate = 0;
char buf[256];
char buf2[20];

void setup()
{
    // We enable the switched row.
    pinMode(GROVEPWR, OUTPUT);
    digitalWrite(GROVEPWR, HIGH);

    // USB Serial for debug and comms
    Serial.begin(9600);

    // GPS Serial
    Serial1.begin(9600);

    // TPH Sensor
    while(!BME280.begin(I2C_FAST_MODE_PLUS_MODE))
    {
        Serial.println("Looking for BME280...");
        delay(3000);
    }
    Serial.println("Setting up BME280...");
    BME280.mode(NormalMode);
    BME280.setOversampling(TemperatureSensor, Oversample16);
    BME280.setOversampling(HumiditySensor, Oversample16);
    BME280.setOversampling(PressureSensor, Oversample16);
    BME280.iirFilter(IIR16);
    BME280.inactiveTime(inactive1000ms);

    // Analog Sensor
    Serial.println("Awaiting startup of Analog Sensor...");
    delay(2500);
    airQualitySensor.init();

    // Laser Sensor
    particleSensor.openMeasurement();
}

void dumpData()
{
    // Output GPS time.
    sprintf(buf, "%02d-%02d-%02dT%02d:%02d:%02d.%02d:\n", 
            GPS.date.year(), 
            GPS.date.month(), 
            GPS.date.day(),
            GPS.time.hour(),
            GPS.time.minute(),
            GPS.time.second(),
            GPS.time.centisecond());
    Serial.print(buf);

    // Output GPS data.
    sprintf(buf, "\tLatitude:\t%s°\n", dtostrf(GPS.location.lat(), 1, 6, buf2));
    Serial.print(buf);
    sprintf(buf, "\tLongitude:\t%s°\n", dtostrf(GPS.location.lng(), 1, 6, buf2));
    Serial.print(buf);
    sprintf(buf, "\tAltitude:\t%s m\n", dtostrf(GPS.altitude.meters(), 1, 2, buf2));
    Serial.print(buf);
    sprintf(buf, "\tSpeed:\t\t%s m/s\n", dtostrf(GPS.speed.mps(), 1, 2, buf2));
    Serial.print(buf);
    sprintf(buf, "\tHeading:\t%s°\n", dtostrf(GPS.course.deg(), 1, 2, buf2));
    Serial.print(buf);
    sprintf(buf, "\tPrecision:\t%ld\n", GPS.hdop.value());
    Serial.print(buf);
    sprintf(buf, "\tSatellites:\t%d\n", GPS.satellites.value());
    Serial.print(buf);
    
    // Analog Sensor
    int airQuality = airQualitySensor.slope();
    sprintf(buf, "\tAnalog Sensor:\t%d (%c)\n", airQualitySensor.getValue(), airQuality == AirQualitySensor::FORCE_SIGNAL || airQuality == AirQualitySensor::HIGH_POLLUTION ? 'H' : airQuality == AirQualitySensor::LOW_POLLUTION ? 'L' : 'C');
    Serial.print(buf);

    // Laser Sensor
    uint32_t PM1_0 = 0;
    uint32_t PM2_5 = 0;
    uint32_t PM10  = 0;
    particleSensor.readMeasurements(&PM1_0, &PM2_5, &PM10);
    sprintf(buf, "\tPM1.0:\t\t%lu μg/m³\n", PM1_0);
    Serial.print(buf);
    sprintf(buf, "\tPM2.5:\t\t%lu μg/m³\n", PM2_5);
    Serial.print(buf);
    sprintf(buf, "\tPM10:\t\t%lu μg/m³\n", PM10);
    Serial.print(buf);

    // TPH
    int32_t temp = 0;
    int32_t pressure = 0;
    int32_t humidity = 0;
    BME280.getSensorData(&temp, &humidity, &pressure);
    sprintf(buf, "\tTemperature:\t%s °C\n", dtostrf(temp/100.0, 1, 2, buf2));
    Serial.print(buf);
    sprintf(buf, "\tHumidity:\t%s%%\n", dtostrf(humidity/100.0, 1, 2, buf2));
    Serial.print(buf);
    sprintf(buf, "\tPressure:\t%s hPa\n\n", dtostrf(pressure/100.0, 1, 2, buf2));
    Serial.print(buf);
}

void loop() 
{
    // Feed the Hungy GPS Object
    while(Serial1.available()) GPS.encode(Serial1.read());

    // Check if a data dump is needed...
    uint32_t currTime = millis();
    if(currTime - lastUpdate > 500)
    {
        lastUpdate = millis();
        dumpData();
    }
}
