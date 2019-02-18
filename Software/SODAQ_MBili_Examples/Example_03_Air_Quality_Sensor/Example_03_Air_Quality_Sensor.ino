
/*  Example by Vincent Claes
 *  vincent [at]cteq.eu
 *  8/02/2019
 *  
 *  Indoor Air Quality Sensor on SODAQ MBili board
 *  
*/
#include "AirQuality2.h"

#define AirQualitySensorPin A0

AirQuality2 AirQualitySensor;

int16_t AirQualityValue=-1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
 // pinMode(AirQualitySensorPin, INPUT);
 Serial.println("Before Init...");
  AirQualitySensor.init(AirQualitySensorPin);
  Serial.println("Init Done...");
}

void loop() {
  // put your main code here, to run repeatedly:
  AirQualityValue = AirQualitySensor.getRawData();
  Serial.print("Air Quality: ");
  Serial.print(AirQualityValue);
  Serial.println(" (0-1023)");
  Serial.print("Quality Index: ");
  Serial.print(AirQualitySensor.evaluate());
  Serial.println(" (0 (No) / 1 (Low Pollution) / 2 (High Pollution) / 3 (Bad Air)");
  delay(500);
}
