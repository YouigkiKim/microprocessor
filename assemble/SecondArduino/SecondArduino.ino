#include <SoftwareSerial.h>
#include <Wire.h>
#include "Arduino.h"
#include <Servo.h>
#include <Stepper.h>

#define Cds_Photoresistor_PIN A0
#define airConledPin 7
#define lightLEDPin 5
#define airConditionerServoPin 6
#define boilerLEDPin 3
#define STEPS 2048

Stepper stepper(STEPS, 8, 10, 9, 11);

Servo airConditionerServo;

//전역변수
float humidity;
float currentTemp;
float updateTemp;
float Timediff = 0;
float startTempdiff ;
float desiredTemp;
float lastTx=0;
float diffTemp;
float startTemp;
unsigned long boilerstart;

//상태변수
int curtain_flag = 0;
int cycle = 4;
bool autoflag;
bool tempflag= 0;
bool boilflag=0;
bool airconflag =0;
char data= 'y';


void setup() {
  Serial.begin(9600);
  stepper.setSpeed(12);
}

void loop() {
  if(Serial.available()){
    data = Serial.read();
    if(data == 'z'){
      float state[2];
      state[0] = Serial.parseFloat();
      state[1] = Serial.parseFloat();
      currentTemp = state[0];
      desiredTemp=state[1];
      data = 'y';
    }

  }

  //2번아두이노데이터처리
  if(!(data=='y')){
    if(data == 'h'){
      tempflag = 1;
      data = 'y';
    }
    else if(data == 'i'){
      digitalWrite(airConledPin, LOW);
      airConditionerServo.attach(airConditionerServoPin);
      airConditionerServo.write(20);
      delay(500);
      airConditionerServo.detach();
      updateTemp = currentTemp;
      tempflag = 0;
      data = 'y';
    }
    else if(data == 'j'){
      analogWrite(boilerLEDPin, 0);  //보일러 강제종료
      tempflag = 0;
      updateTemp = currentTemp;
      data = 'y';
    }
    else if(data == 'k'){
      analogWrite(airConledPin, 255);
      airConditionerServo.attach(airConditionerServoPin);
      airConditionerServo.write(90);
      delay(500);
      airConditionerServo.detach();
      tempflag = 0;
      data = 'y';
    }
    //에어컨 터보
    else if(data == 'l'){
      analogWrite(boilerLEDPin, 255);  //보일러 강제종료
      tempflag = 0;
      data = 'y';
    }
    //보일러 터보
    else if(data =='j'){
      analogWrite(boilerLEDPin,0);
      tempflag = 0;
      data = 'y';
    }
    //커튼 제어
    else if(data == 'm'){
      autoflag = true;
    }
    else if(data == 'n'){
      autoflag = false;
      ManualCurtainup();
    }
    else if(data == 'o'){
      autoflag = false;
      ManualCurtaindown();
    }
    //조명
    else if(data == '0'){
      analogWrite(lightLEDPin,LOW);
      data = 'y';
      }//off
    else if(data == '1'){
      analogWrite(lightLEDPin,32);
      data = 'y';
      }//off
    else if(data == '2'){
      analogWrite(lightLEDPin,64);
      data = 'y';
      }//off
    else if(data == '3'){
      analogWrite(lightLEDPin,128);
      data = 'y';
      }//off
    else if(data == '4'){
      analogWrite(lightLEDPin,196);
      data = 'y';
      }//off
    else if(data == '5'){
      analogWrite(lightLEDPin,255);
      data = 'y';
      }//off
    else{
      data='y';
    }
  }

  //온도 자동조절 확인
  if(tempflag == 1){
    //boilerflag로 보일러 작동시작유무 확인
    if(desiredTemp > currentTemp){
      if(boilflag == 0){
        boilflag = 1;
        analogWrite(boilerLEDPin, 255);
        boilerstart = millis();
        startTemp = currentTemp;
        startTempdiff = desiredTemp - currentTemp;
        updateTemp = currentTemp;
      }
      else{
        updateTemp = setBoilerTemperature(desiredTemp,updateTemp, Timediff);
        Timediff = millis();
        if(Timediff >= lastTx+3000){
          Serial.println(updateTemp);
          lastTx = millis();
        }
      }
    }
    //에어컨 작동
    else {
      if(airconflag == 0){
        airconflag =1;
        autoActivateAirConditioner();
        boilerstart = millis();
        startTemp = currentTemp;
        updateTemp = currentTemp;
        startTempdiff = desiredTemp - currentTemp;
      }
      else{
        autoActivateAirConditioner();
        updateTemp = setAirconTemperature(desiredTemp,currentTemp,Timediff);
        Timediff = millis();
        if(Timediff >= lastTx+3000){
          Serial.println(updateTemp);
          lastTx = millis();
        }
      }
    }
  }

  //커튼 제어
  uint16_t aValue = analogRead(Cds_Photoresistor_PIN);
  if(autoflag == true){
    if (aValue < 400){
      Curtainup();
    }
    else {
      Curtaindown();
    }
  }
}

// 에어컨 함수
void autoActivateAirConditioner() {
  digitalWrite(airConledPin, HIGH);
  delay(100);
  airConditionerServo.write(30);
}
void offAirConditioner() {
  digitalWrite(airConledPin,LOW);
  delay(100);
  airConditionerServo.write(0);
}

// float setAirconTemperature(float desiredTemp, float currentTemperature, float StartTime){
//   updateTemp = startTemp + startTempdiff/20*(millis()-StartTime)/1000;
//   return updateTemp;
// }

float setAirconTemperature(float desiredTemp, float currentTemperature, float StartTime){
  float diffTemp = desiredTemp - currentTemperature;
  updateTemp = currentTemperature + diffTemp/20*(millis()-StartTime)/150;
  return updateTemp;
}



float setBoilerTemperature(float desiredTemp, float currentTemperature, float StartTime){
  float diffTemp = desiredTemp - currentTemperature;
  updateTemp = currentTemperature + diffTemp/20*(millis()-StartTime)/150;
  float ratio = diffTemp/startTempdiff;
  int k = map(ratio*100, 0, 100, 1, 255);
  analogWrite(boilerLEDPin, k);
  return updateTemp;
}

//커튼함수
void Curtainup(){
  uint8_t i=0;
  if (curtain_flag == 0){
    for(i=1;i<cycle;i++){
      stepper.step(STEPS);
    }
    curtain_flag = 1;
  }
}

void Curtaindown() {
  uint8_t i=0;
  if (curtain_flag == 1){
    // 역방향으로 3초 동안 회전
    for (i=1;i<cycle;i++){
      stepper.step(-STEPS);
    }
    curtain_flag = 0;
  }
}
void ManualCurtainup() {
  uint8_t i=0;
  if(curtain_flag == 0 ){
    for (i=1;i<cycle;i++){
      stepper.step(STEPS);
    }
  }
  curtain_flag = 1;
}
void ManualCurtaindown() {
  // 역방향으로 3초 동안 회전
  uint8_t i=0;
  if(curtain_flag == 1){
    for (i=1;i<cycle;i++){
      stepper.step(-STEPS);
    }
  }
  curtain_flag = 0;
}