/*  Example by Vincent Claes
 *  vincent [at]cteq.eu
 *  8/02/2019
 *  
 *  Loudness Sensor on SODAQ MBili board
 *  
*/

#define SoundSensorPin A4 

int SoundValue;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SoundSensorPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  SoundValue = analogRead(SoundSensorPin);
  Serial.println(SoundValue);
  delay(10);
}
