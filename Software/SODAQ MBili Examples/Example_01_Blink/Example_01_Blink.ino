/*  Example by Vincent Claes
 *  vincent [at]cteq.eu
 *  8/02/2019
 *  
 *  Blinking LED on SODAQ MBili board
 *  
*/
void setup() {
  // put your setup code here, to run once:
  pinMode(LED1, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED1, HIGH);
  delay(500);
  digitalWrite(LED1, LOW);
  delay(500);
}
