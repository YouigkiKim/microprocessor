#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "Stepper.h"
#include <DHT.h>
#define pin_in1 4  
#define pin_in2 11
#define irSensorPin A2               // 적외선 센서의 핀
#define DHTPIN 4          // DHT 센서의 데이터 핀
#define DHTTYPE DHT22     // DHT 센서의 타입 (DHT22 또는 DHT11)
#define STEPPERREV 2048

//class설정

SoftwareSerial BTserial(2,3); // RX, TX
Servo windowServo;
Servo airConditionerServo;
Stepper myStepper(STEPPERREV, pin_in1, pin_in2);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27에 연결된 16x2 LCD
DHT dht(DHTPIN, DHTTYPE);

//변수설정

int lightLEDPin = 5;               // 조명에 사용할 LED 핀
int boilerLEDPin = 6;              // 보일러 선 LED가 연결된 핀
int windowServoPin = 7;             // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀
int airConditionerServoPin = 9;    // 에어컨 서보모터 핀
int VentmotorPin = 10;             // 환풍기 DC 모터가 연결된 핀
int airConledPin = 12;             // 에어컨 내부 led 핀
//보일러 변수
int brightness; //보일러LED 밝기 변수 선언
int irSensorValue; // irSensorValue 변수 선언 
char data;
int tempflag = 0;
//환풍기 변수
int ventflag = 0;
//방범상태
bool Security = true;
bool SecureFlag;
bool AlarmFlag = false;
//창문상태
bool WindowClose = true;
bool WindowAuto = true;
bool WindowState = WindowClose; 
//시간변수
unsigned long startTime = 0; // 시작 시간 저장 변수
unsigned long alarmStart = 0;
unsigned long startTimelight;
unsigned long actstartTime;
unsigned long alarmcurrent;
unsigned long currentTime;
uint8_t desiredTemperature = 22;
//stepper rpm
uint8_t rpm = 8;



void setup() {
  noInterrupts();
  Serial.begin(9600);
  BTserial.begin(9600);
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
  interrupts();
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

void autoActivateAirConditioner() {
  // 에어컨을 작동하는 코드 추가
  digitalWrite(airConledPin, HIGH);
  delay(100);
  // 에어컨 작동을 표현하기 위해 서보모터를 작동시킴
  airConditionerServo.attach(airConditionerServoPin);
  airConditionerServo.write(90);
  delay(100);
  airConditionerServo.detach();
}

//LCD와 시리얼 모니터에 현재 온습도 표현 함수
void displayTemperatureAndHumidity() {             
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  lcd.clear();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);   //0행 실내온도
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);   //1행 실내습도
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" °C");
}




void SoftwareInterrupt(){

  char data;
  data = UDR0;

  if(data == 'a'){Security = true;}
  else if(data == 'b'){Security = false;}
  //환풍기 AUTO ON OFF
  else if(data == 'c'){ventflag = 1;}
  else if(data == 'd'){
    analogWrite(VentmotorPin, 255);
    ventflag = 0;
  }
  else if(data == 'e'){
    analogWrite(VentmotorPin, 0);
    ventflag = 0;
  }
  else if(data == 'f'){openWindow();}
  else if(data == 'g'){closeWindow();}
  else if(data == 'h'){
    int tempflag = 1;
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
  else if(data == '0'){analogWrite(lightLEDPin,LOW);}//off
  else if(data == '1'){analogWrite(lightLEDPin,32);}//off
  else if(data == '2'){analogWrite(lightLEDPin,64);}//off
  else if(data == '3'){analogWrite(lightLEDPin,128);}//off
  else if(data == '4'){analogWrite(lightLEDPin,196);}//off
  else if(data == '5'){analogWrite(lightLEDPin,255);}//off
}