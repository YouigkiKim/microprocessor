#include <SoftwareSerial.h>
#include <Servo.h>

#include <Wire.h>

#include "Arduino.h"
#include "Stepper.h"

Servo windowServo;

#define irSensorPin A2               // 적외선 센서의 핀
//시간변수
unsigned long startTime = 0; // 시작 시간 저장 변수
unsigned long alarmStart = 0;
unsigned long startTimelight;
unsigned long actstartTime;
unsigned long alarmcurrent;
unsigned long currentTime;
//핀설정
int windowServoPin = 7;             // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀
int irSensorValue; // irSensorValue 변수 선언 

//창문상태
bool WindowClose = true;
bool WindowAuto = true;
bool WindowState = WindowClose; 
//방범상태
bool Security = true;
bool SecureFlag;
bool AlarmFlag = false;



void setup() {
  pinMode(buzzerPin, OUTPUT);    // 피에조 부저

  Serial.begin(9600);

  windowServo.attach(7);  // 창문 서보모터를 7번 핀에 연결  
  
}

void loop(){

  //AUTO모드 제어
  openWindow();
  closeWindow();
  
  irSensorValue = analogRead(irSensorPin);
  Serial.print("WindowState: ");
  Serial.println(WindowState);
  Serial.print("Security: ");
  Serial.println(Security);
  Serial.print("IR Sensor Value: ");
  Serial.println(irSensorValue);
  

  //창문 제어
  if ( WindowState == Security) {//창문상태 방범장치 상태 and 확인
    if(irSensorValue < 200 ){ // 창문이 닫혀있고 물체가 감지되면
      Serial.println(startTime);
      if(startTime == 0){
        Serial.println("startTime ");
        startTime = millis();
        SecureFlag = true;
      }

      unsigned long currentTime = millis();
      Serial.println(SecureFlag);
      if(SecureFlag == true){
        Serial.println("SecureFlag ");

        if(currentTime - 5000 > startTime){
          Serial.println("currentTime - 5000");

            if(AlarmFlag == 0){
              actstartTime = millis();
              AlarmFlag = true;
              Serial.println(AlarmFlag);
              alarmStart = millis();  // 현재 시간 기록
            }
            if(AlarmFlag == true){
            Serial.print("activateAlarm ");
            alarmcurrent = millis();
            activateAlarm();  // 알람작동
            }
        }
        else if(currentTime < startTime + 500 ){
          lightAlarm();
        }
      }
      
    } 
    
    else{
      startTime = 0;
      currentTime = 0;
      SecureFlag = false;
      AlarmFlag = false;
      noTone(buzzerPin);
    }
  }



}


//창문 닫기 함수
void closeWindow() {                      
  
  windowServo.write(0);  // 닫힌 위치에 해당하는 각도
  delay(1000); 
  WindowState= true;
}

//창문 열기 함수
void openWindow() {
  windowServo.write(90);  // 열린 위치에 해당하는 각도
  delay(1000); 
  WindowState= false;  
}

//방범장치 함수
void lightAlarm(){
  startTimelight = millis();
  Serial.println("liht arlded: ");
  while (millis() - startTimelight < 1000){
    tone(buzzerPin, 1000);
    delay(500);
    noTone(buzzerPin);
  }
  
}

//방범장치 함수 - 5초 이상 물체 감지시 10초간 0.5초동안 울렸다 멈췄다 반복
void activateAlarm(){   

  Serial.println("alarm");
  Serial.println(alarmcurrent);
  Serial.println(actstartTime);
  if(alarmcurrent - actstartTime < 10000){  // 현재 시간 기록 8000 ){
    Serial.println("current");
    tone(buzzerPin, 1000); 
  }
  else{
    Serial.println("noTo");
    noTone(buzzerPin);
    alarmStart = 0 ;
    AlarmFlag = 0;
  }
 
}

