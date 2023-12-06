#include <SoftwareSerial.h>

SoftwareSerial BTserial(2,3); // RX, TX
//uint8_t data = 0x00;


char data ; 

void setup() {

  Serial.begin(9600);
  BTserial.begin(9600);
}

void loop() {

  if(BTserial.available()){
    data = BTserial.read() ;
    Serial.write(data);
  }

  if( Serial.available() ){
    BTserial.write( Serial.read() );
  }
}