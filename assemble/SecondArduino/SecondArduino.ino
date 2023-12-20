#include <SoftwareSerial.h>
#include <Wire.h>
#include "Arduino.h"
#include <Servo.h>

#define airConledPin 7
#define lightLEDPin 8
#define airConditionerServoPin 9
#define boilerLEDPin 10
#define ASRx 11
#define ASTx 12

SoftwareSerial AS(ASRx,ASTx);//ArduinoSerial

Servo airConditionerServo;
float desiredTemperature;  

//전역변수
float humidity;
float currentTemp;
float updateTemp;
float Timediff = 0;
float startTempdiff ;
float desiredTemp;
float lastTx=0;
unsigned long boilerstart;
//상태변수
bool tempflag= 0;
bool boilflag=0;
bool airconflag =0;
char data='y';


void setup() {
  AS.begin(9600);
  attachInterrupt(digitalPinToInterrupt(ASRx),SoftwareISR,RISING); //Falling은 비트손실로 인해 데이터전송이 원활하지 않은듯함
}

void loop() {
  //온도 자동조절 확인
  if(tempflag == 1){
    //boilerflag로 보일러 작동시작유무 확인
    if(desiredTemp > currentTemp){
      if(boilflag == 0){
        boilflag = 1;
        analogWrite(boilerLEDPin, 255);
        boilerstart = millis();
        startTempdiff = desiredTemp - currentTemp;
        updateTemp = currentTemp;
      }
      else{
        updateTemp = setBoilerTemperature(desiredTemp,updateTemp, Timediff);
        Timediff = millis();
        if(Timediff >= lastTx+2000){
          AS.print(updateTemp);
        }
      }
    }
    //에어컨 작동
    else if(desiredTemp < currentTemp){
      if(airconflag ==0){
        airconflag =1;
        autoActivateAirConditioner();
        boilerstart = millis();
        startTempdiff = currentTemp - desiredTemp;
      }
      else{
        updateTemp = setAirconTemperature(desiredTemp,currentTemp,Timediff);
        Timediff = millis();
        if(Timediff >= lastTx+2000){
          AS.print(updateTemp);
        }
      }
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
      airConditionerServo.write(0);
      delay(200);
      airConditionerServo.detach();
      tempflag = 0;
      data = 'y';
    }
    else if(data == 'j'){
      analogWrite(boilerLEDPin, 0);  //보일러 강제종료
      tempflag = 0;
      data = 'y';
    }
    else if(data == 'k'){
      analogWrite(airConledPin, 255);
      airConditionerServo.attach(airConditionerServoPin);
      airConditionerServo.write(0);
      delay(500);
      airConditionerServo.detach();
      tempflag = 0;
      data = 'y';
    }
    //에어컨 터보
    else if(data == 'l'){
      analogWrite(boilerLEDPin, 255);  //보일러 강제종료
      int tempflag = 0;
      data = 'y';
    }
    //보일러 터보
    else if(data == 'm'){}
    else if(data == 'n'){}
    else if(data == 'o'){}
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
    else if(data == 'z'){
      tempflag =1;
      data = 'y';
    }
    else{
      float state[3];
      state[0] = Serial.parseFloat();
      state[1] = Serial.parseFloat();
      state[2] = Serial.parseFloat();
      currentTemp = state[0];
      humidity = state[1];
      desiredTemperature=state[2];
    }
  }
}

void SoftwareISR(){
  while(Serial.available()){
    data = Serial.read();
  }
}

// 에어컨 함수
void autoActivateAirConditioner() {
  digitalWrite(airConledPin, HIGH);
  delay(100);
  airConditionerServo.write(90);
}

void offAirConditioner() {
  digitalWrite(airConledPin,LOW);
  delay(100);
  airConditionerServo.write(0);
}

float setBoilerTemperature(float desiredTemp, float currentTemperature, float StartTime){
  float diffTemp = desiredTemp - currentTemperature;
  updateTemp = currentTemperature + diffTemp/20*(millis()-StartTime)/1000;
  float ratio = diffTemp/startTempdiff;
  int k = map(ratio*100, 0, 100, 1, 255);
  analogWrite(boilerLEDPin, k);
  return updateTemp;
}

float setAirconTemperature(float desiredTemp, float currentTemperature, float StartTime){
  float diffTemp = desiredTemp - currentTemperature;
  updateTemp = currentTemperature + diffTemp/20*(millis()-StartTime)/1000;
  return updateTemp;
}