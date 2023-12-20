#include <SoftwareSerial.h>

SoftwareSerial BTserial(2,3); // RX, TX

char data = 'y';
float T = 21.99;
float H = 12.12;


void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2),SoftwareISR,RISING); //Falling은 비트손실로 인해 데이터전송이 원활하지 않은듯함
}

void loop() {

  if( Serial.available() ){
    BTserial.write( Serial.read());
  }

  if(!(data == 'y')){
    if(data == 'x'){
      Serial.println("데이터 전송 시작");
      BTserial.print(T);
      BTserial.println("C'");
      BTserial.print(H);
      BTserial.println("%");
      Serial.println("checkx, data=");
      Serial.println("데이터 전송 끝");
      data = 'y';
    }
    else{
    Serial.print("else: ");
    Serial.println(data);
    data = 'y';
    }
  }

  delay(3000);
}

void SoftwareISR(){
  while(BTserial.available()){
    data = BTserial.read();
  }
}


