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

#define CBOR
#include "ATT_NBIOT.h"
#define GPS_STREAM Serial
#define MODEM_STREAM Serial1
#define MODEM_ON_OFF_PIN 23
ATT_NBIOT device;
#include "CborBuilder.h"
CborBuilder payload(device);

#define UPDATERATE 5000

void setup()
{
    // We enable the switched row.
    pinMode(GROVEPWR, OUTPUT);
    digitalWrite(GROVEPWR, HIGH);

    // Start serial connections
    MODEM_STREAM.begin(9600);
    GPS_STREAM.begin(9600);

    // TPH Sensor
    while(!BME280.begin(I2C_FAST_MODE_PLUS_MODE))
    {
        GPS_STREAM.println("Waiting for BME sensor...");
        delay(3000);
    }
    GPS_STREAM.println("Setting up BME sensor...");
    BME280.mode(NormalMode);
    BME280.setOversampling(TemperatureSensor, Oversample16);
    BME280.setOversampling(HumiditySensor, Oversample16);
    BME280.setOversampling(PressureSensor, Oversample16);
    BME280.iirFilter(IIR16);
    BME280.inactiveTime(inactive1000ms);

    // Analog Sensor
    GPS_STREAM.println("Setting up analog sensor...");
    delay(2500);
    airQualitySensor.init();

    // Laser Sensor
    GPS_STREAM.println("Starting particle sensor...");
    particleSensor.openMeasurement();

    GPS_STREAM.print("Connecting to the internet...");
    device.init(MODEM_STREAM, GPS_STREAM, MODEM_ON_OFF_PIN);
    if(device.connect())
    {
        GPS_STREAM.println(" Connected!\033[2J");
    }
    else
    {
        GPS_STREAM.println(" Connection failed!");
        while(true);
    }

    
}

void dumpData()
{
    // Return if there's no fix.
    if(GPS.location.lat() < 0.001) return;
    
    // Output GPS time.
    sprintf(buf, "\033[H%02d-%02d-%02dT%02d:%02d:%02d.%02d:\r\n", 
            GPS.date.year(), 
            GPS.date.month(), 
            GPS.date.day(),
            GPS.time.hour(),
            GPS.time.minute(),
            GPS.time.second(),
            GPS.time.centisecond());
    GPS_STREAM.print(buf);

    // Output GPS data.
    sprintf(buf, "\tLatitude:\t%s degrees\r\n", dtostrf(GPS.location.lat(), 1, 6, buf2));
    GPS_STREAM.print(buf);
    sprintf(buf, "\tLongitude:\t%s degrees\r\n", dtostrf(GPS.location.lng(), 1, 6, buf2));
    GPS_STREAM.print(buf);
    sprintf(buf, "\tAltitude:\t%s m\r\n", dtostrf(GPS.altitude.meters(), 1, 2, buf2));
    GPS_STREAM.print(buf);
    sprintf(buf, "\tSpeed:\t\t%s m/s\r\n", dtostrf(GPS.speed.mps(), 1, 2, buf2));
    GPS_STREAM.print(buf);
    sprintf(buf, "\tHeading:\t%s degrees\r\n", dtostrf(GPS.course.deg(), 1, 2, buf2));
    GPS_STREAM.print(buf);
    sprintf(buf, "\tPrecision:\t%ld\r\n", GPS.hdop.value());
    GPS_STREAM.print(buf);
    sprintf(buf, "\tSatellites:\t%d\r\n", GPS.satellites.value());
    GPS_STREAM.print(buf);
    
    // Analog Sensor
    int airQuality = airQualitySensor.slope();
    sprintf(buf, "\tAnalog Sensor:\t%d (%c)\r\n", airQualitySensor.getValue(), airQuality == AirQualitySensor::FORCE_SIGNAL || airQuality == AirQualitySensor::HIGH_POLLUTION ? 'H' : airQuality == AirQualitySensor::LOW_POLLUTION ? 'L' : 'C');
    Serial.print(buf);

    // Laser Sensor
    uint32_t PM1_0 = 0;
    uint32_t PM2_5 = 0;
    uint32_t PM10  = 0;
    particleSensor.readMeasurements(&PM1_0, &PM2_5, &PM10);
    sprintf(buf, "\tPM1.0:\t\t%lu microgram per square meter\r\n", PM1_0);
    GPS_STREAM.print(buf);
    sprintf(buf, "\tPM2.5:\t\t%lu microgram per square meter\r\n", PM2_5);
    GPS_STREAM.print(buf);
    sprintf(buf, "\tPM10:\t\t%lu microgram per square meter\r\n", PM10);
    GPS_STREAM.print(buf);

    // TPH
    int32_t temp = 0;
    int32_t pressure = 0;
    int32_t humidity = 0;
    BME280.getSensorData(&temp, &humidity, &pressure);
    sprintf(buf, "\tTemperature:\t%s degrees Celsius\r\n", dtostrf(temp/100.0, 1, 2, buf2));
    GPS_STREAM.print(buf);
    sprintf(buf, "\tHumidity:\t%s%%\r\n", dtostrf(humidity/100.0, 1, 2, buf2));
    GPS_STREAM.print(buf);
    sprintf(buf, "\tPressure:\t%s hPa\r\n", dtostrf(pressure/100.0, 1, 2, buf2));
    GPS_STREAM.print(buf);

    // Send payload
    payload.reset();
    payload.map(8);
    payload.addInteger(PM1_0, "pm1");
    payload.addInteger(PM2_5, "pm25");
    payload.addInteger(PM10, "pm10");
    payload.addNumber(humidity/100.0, "humidity");
    payload.addNumber(pressure/100.0, "pressure");
    payload.addNumber(temp/100.0, "temperature");
    payload.addInteger(airQualitySensor.getValue(), "air_quality");
    payload.addGPS(GPS.location.lat(), GPS.location.lng(), GPS.altitude.meters(), "gps");
    payload.send();
}

void loop() 
{
    // Feed the Hungy GPS Object
    while(GPS_STREAM.available()) GPS.encode(GPS_STREAM.read());

    // Check if a data dump is needed...
    uint32_t currTime = millis();
    if(currTime - lastUpdate > UPDATERATE)
    {
        lastUpdate = millis();
        dumpData();
    }
}
