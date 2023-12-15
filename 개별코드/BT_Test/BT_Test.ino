#include <SoftwareSerial.h>

SoftwareSerial BTserial(4,5); // RX, TX


void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);
}

void loop() {
  if(BTserial.available()){

  }

  if( Serial.available() ){
    BTserial.write( Serial.read());
    
  }
  Serial.write(BTserial.read());
  int T = 20;
  int H = 22;
  String data = String(T) + "a" + String(H);
  BTserial.print(data);
  delay(1000);
}


