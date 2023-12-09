#include <SoftwareSerial.h>

SoftwareSerial BTserial(4,5); // RX, TX


void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);
}

void loop() {

  if(BTserial.available()){
    Serial.write(BTserial.read());
  }

  if( Serial.available() ){
    BTserial.write( Serial.read());
  }

  int T = 20;
  int H = 22;
  String data = String(T) + "," + String(H);
  BTserial.println(data);
  delay(1000);
}


