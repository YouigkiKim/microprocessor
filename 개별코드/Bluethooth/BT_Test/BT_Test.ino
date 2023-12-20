#include <SoftwareSerial.h>

SoftwareSerial BTserial(2,3); // RX, TX


void setup() {
  Serial.begin(38400);
  BTserial.begin(38400);
}

void loop() {
  if(BTserial.available()){
    Serial.write(BTserial.read());
    float T = 21.75;
    float H = 12.12;
    BTserial.print(T);
    BTserial.println("C'");
    BTserial.print(H);
    BTserial.println("%");
  }

  if( Serial.available() ){
    BTserial.write( Serial.read());
  }
}


