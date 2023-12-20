#include <SoftwareSerial.h>

SoftwareSerial BTserial(2,3); // RX, TX


void setup() {
  Serial.begin(38400);
  BTserial.begin(38400);
}

void loop() {
  if(BTserial.available()){
    Serial.write(BTserial.read());
  }

  if( Serial.available() ){
    BTserial.write( Serial.read());
  }
}


