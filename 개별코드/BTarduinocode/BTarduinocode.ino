#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"

#define irSensorPin A2               // 적외선 센서의 핀
#define DHTPIN 4          // DHT 센서의 데이터 핀
#define DHTTYPE DHT22     // DHT 센서의 타입 (DHT22 또는 DHT11)
DHT dht(DHTPIN, DHTTYPE);

unsigned long startTime = 0; // 시작 시간 저장 변수

SoftwareSerial BTSerial(2,3); // RX, TX

int lightLEDPin = 5;               // 조명에 사용할 LED 핀
int boilerLEDPin = 6;              // 보일러 선 LED가 연결된 핀
int windowServoPin = 7             // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀
int airConditionerServoPin = 9;    // 에어컨 서보모터 핀
int VentmotorPin = 10;             // 환풍기 DC 모터가 연결된 핀
int airConledPin = 12;             // 에어컨 내부 led 핀

//창문상태지정 True=닫힘 False=열림
bool WindowClose = true;
bool WindowAuto = true;
bool WindowState = WindowClose;           //창문상태지정 True=닫힘 False=열림
unsigned long startTime;
unsigned long currentTime;
int brightness; //보일러LED 밝기 변수 선언
int irSensorValue; // irSensorValue 변수 선언 
char data;
int ventflag = 0;

const uint8_t pin_in1 = 4;  
const uint8_t pin_in2 = 11;
uint8_t rpm = 8;

Stepper myStepper(STEPPERREV, pin_in1, pin_in2);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27에 연결된 16x2 LCD

ISR(USART_RX_vect){

  char data;
  data = BTserial.read();

  if(data == 'a'){Security = SecurityOn;}
  else if(data == 'b'){Security = SecurityOff;}
  //환풍기 AUTO ON OFF
  else if(data == 'c'){ventflag = 1;}
  else if(data == 'd'){
    analogWrite(VentmotorPin, 255);
    int ventflag = 0;
  }
  else if(data == 'e'){
    analogWrite(VentmotorPin, 0);
    int ventflag = 0;
  }
  else if(data == 'f'){openWindow();}
  else if(data == 'g'){closeWindow();}
  else if(data == 'h'){
    int tempflag = 1;
    if (desiredTemperature > currentTemperature){
          setBoilerTemperature(desiredTemperature, currentTemperature);
        }
    else if(desiredTemperature<currentTemperature){
      autoActivateAirConditioner();
    }
  }
  else if(data == 'i'){
    digitalWrite(airConledPin, LOW);
    airConditionerServo.attach(airConditionerServoPin);
    airConditionerServo.write(0);
    delay(500);
    airConditionerServo.detach();
    int tempflag = 0;
  }
  else if(data == 'j'){
    analogWrite(boilerLEDPin, 0);  //보일러 강제종료
    int tempflag = 0;
  }
  else if(data == 'k'){}//에어컨 터보
  else if(data == 'l'){}//보일러 터보
  else if(data == 'm'){}
  else if(data == 'n'){}
  else if(data == 'o'){}
  
  //조명
  else if(data == '0'){analogWrite(lightLEDPin,LOW)}//off
  else if(data == '1'){analogWrite(lightLEDPin,32)}//off
  else if(data == '2'){analogWrite(lightLEDPin,64)}//off
  else if(data == '3'){analogWrite(lightLEDPin,128)}//off
  else if(data == '4'){analogWrite(lightLEDPin,196)}//off
  else if(data == '5'){analogWrite(lightLEDPin,255)}//off
}

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  myStepper.setSpeed(rpm);
  pinMode(lightLEDPin, OUTPUT);  // 조명 LED 핀
  pinMode(boilerLEDPin, OUTPUT); // 보일러 선 led
  windowServo.attach(7); // 창문 서보모터를 7번 핀에 연결  
  pinMode(buzzerPin, OUTPUT);    // 피에조 부저
  airConditionerServo.attach(9);  // 에어컨 서보모터를 7번 핀에 연결
  pinMode(VentmotorPin, OUTPUT); // 환풍기 dc모터
  pinMode(airConledPin, OUTPUT); // 에어컨 내부 led 핀
  lcd.begin(16, 2);
  dht.begin();
}


void loop() {

}


//창문 닫기 함수
void closeWindow() {                      
  windowServo.write(0);  // 닫힌 위치에 해당하는 각도
  delay(500);  
}

//창문 열기 함수
void openWindow() {                        
  windowServo.write(90);  // 열린 위치에 해당하는 각도
  delay(100);  
}

//방범장치 함수
//물체 1번 감지시 lightAlarm작동
void lightAlarm(){
  startTime = millis();
  tone(buzzerPin, 1000);
  delay(500);
  noTone(buzzerPin);
}

//방범장치 함수 - 5초 이상 물체 감지시 10초간 0.5초동안 울렸다 멈췄다 반복
void activateAlarm(){            
  unsigned long startTime = millis();  // 현재 시간 기록
  while (millis() - startTime < 10000) {  // 10초 동안 반복
    unsigned long AlarmStart = millis();  // 현재 시간 기록
    tone(buzzerPin, 1000);
    delay(500);  // 0.5초 대기
    noTone(buzzerPin);  // 피에조 부저 끄기
    delay(500);  // 0.5초 대기 
    AlarmStart = 0;
  }
}