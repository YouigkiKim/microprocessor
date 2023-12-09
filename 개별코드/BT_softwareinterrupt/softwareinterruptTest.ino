#include <SoftwareSerial.h>

SoftwareSerial BTserial(2,3);
char data;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  BTserial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2),SoftwareISR,FALLING);
}
void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    BTserial.write(Serial.read());
  }
  if(BTserial.available()){
    Serial.print(BTserial.read());
  }

  Serial.println(millis());
  delay(1000);
}

void SoftwareISR(){
  while(BTserial.available()){
  data = BTserial.read();
  Serial.print(data);
  }
}