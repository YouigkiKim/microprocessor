#include <SoftwareSerial.h>

SoftwareSerial BTserial(2,3); // RX, TX
uint8_t data = 0x00;

void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);
}

void loop() {

  if(BTserial.available()){
    Serial.write(BTserial.read());
    
  }

  if( Serial.available() ){
    BTserial.write( Serial.read() );
   
  }
}


//main 에서 뽑아온코드

SoftwareSerial bluetooth(0, 1);  // RX, TX