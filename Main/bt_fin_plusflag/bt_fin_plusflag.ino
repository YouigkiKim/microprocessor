#include <SoftwareSerial.h>
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "Stepper.h"

Servo windowServo;
Servo airConditionerServo;

#define STEPPERREV 2048
#define Cds_Photoresistor_PIN A0
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
bool WindowClose = true;;
bool WindowAuto = true;
bool WindowState = WindowClose;           //창문상태지정 True=닫힘 False=열림
bool WindowState = WindowClose; 
//창문상태지정 True=닫힘 False=열림
bool WindowClose = true;;
bool WindowAuto = true;
bool WindowState = WindowClose;           //창문상태지정 True=닫힘 False=열림
bool WindowState = WindowClose; 
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

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  myStepper.setSpeed(rpm);
  pinMode(lightLEDPin, OUTPUT);  // 조명 LED 핀
  pinMode(boilerLEDPin, OUTPUT); // 보일러 선 led
  windowServo.attach(7);  // 창문 서보모터를 7번 핀에 연결  
  pinMode(buzzerPin, OUTPUT);    // 피에조 부저
  airConditionerServo.attach(9);  // 에어컨 서보모터를 7번 핀에 연결
  pinMode(VentmotorPin, OUTPUT); // 환풍기 dc모터
  pinMode(airConledPin, OUTPUT); // 에어컨 내부 led 핀
  lcd.begin(16, 2);
  dht.begin();
}

void loop(){
  // 시리얼 모니터로부터 데이터를 읽어서 Bluetooth Serial로 전송
  float humidity = dht.readHumidity();    // dht_22의 실내 습도 측정값
  float currentTemperature = dht.readTemperature();     //dht_22의 실내 온도 측정값
  float desiredTemperature = 22

  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }

  // Bluetooth Serial로부터 데이터를 읽어들임
  if (BTSerial.available()) {

    // 읽은 데이터를 변수에 저장
    data = BTSerial.read();

    // 읽은 데이터를 시리얼 모니터로 출력, 테스트용 코드
    Serial.write(data);

    // 스마트폰 신호가 'a'일 경우 (방범장치 제어)
    if (data == 'a') {

      while(!BTSerial.available()); // polling방식으로 두번째 데이터를 기다림
      data = BTSerial.read();

      if(data == '1'){
        Security = SecurityOn;
      }
      else if(data == '0'){
        Security = SecurityOff;
      }
    }
  
    // 스마트폰 신호가 'b'일 경우 (환풍기 제어)
    else if (data == 'b') {
      while (!BTSerial.available());
      // 이후 두번째 스마트폰 신호가 'a'인 경우 (환풍기 제어 AUTO모드 작동)
      if (data == 'a') {
        ventflag = 1;
      else{
        ventflag = 0;
      }
      }

      // 이후 두번째 스마트폰 신호가 'b나 c'인 경우 (환풍기 제어 스마트폰 수동 ON/OFF)
      else if (data == 'b') {
        analogWrite(VentmotorPin, 255);
        int ventflag = 0;
      }
      else if (data == 'c') {
        analogWrite(VentmotorPin, 0);
        int ventflag = 0;
      }
    }

    // 스마트폰 신호가 'c'일 경우 (window 제어)
    else if (data == 'c') {
      while (!BTSerial.available()); //polling방식으로 두번째 데이터 수신을 기다림.
      data = BTSerial.read();
      if(data = ){
        openWindow();
      }
      else{
        closeWindow();
      }
    }
    
    // 스마트폰 신호가 'f' 일 경우 (보일러와 에어컨 제어)
    else if (data == 'f') {
      while (!BTSerial.available());
      data = BTSerial.read();

      if (data == 'a') {
        int tempflag = 1;
        if (desiredTemperature > currentTemperature) {
          setBoilerTemperature(desiredTemperature, currentTemperature);
        }
        else{
          autoActivateAirConditioner();
        }
      }
   
      else if (data == 'b') { //에어컨 강제 종료
        digitalWrite(airConledPin, LOW);
        airConditionerServo.attach(airConditionerServoPin);
        airConditionerServo.write(0);
        delay(500);
        airConditionerServo.detach();
        int tempflag = 0;  
      }
      else if (data == 'c') {
        analogWrite(boilerLEDPin, 0);  //보일러 강제종료
        int tempflag = 0;
      }
    }
  
    // 스마트폰 신호가 'e' 일 경우 (원격 조정 조명 밝기 제어)
    else if (data == 'e') {
      while(!BT.available);
      data = BT.read();

      //data의 string을 int로 변환하거나 문자열 if문으로 변경이 필요함
      switch (data) {
        case '5':
          analogWrite(lightLEDPin, 255);
          break;
        case '4':
          analogWrite(lightLEDPin, 192);
          break;
        case '3':
          analogWrite(lightLEDPin, 128);
          break;
        case '2':
          analogWrite(lightLEDPin, 64);
          break;
        case '1':
          analogWrite(lightLEDPin, 32);
          break;
        case '0':
          analogWrite(lightLEDPin, 0);
          break;
      }
    }

    // 스마트폰 신호가 'g' 일 경우 (커튼 제어)
    else if (data == 'g') {
      while (!BTSerial.available();

      if (data == 'a') {
        int curtainflag = 1;
        Curtain();
      }
      else if (data == 'b') {
        int curtainflag = 0;
        myStepper.step(STEPPERREV);
        delay(3000);
      }
      else if (data == 'c') {
        int curtainflag = 0;
        myStepper.step(-STEPPERREV);
        delay(3000);
      }
    }
  }
  //블루투스 종료

  //AUTO모드 제어

  //창문 제어
  if ( WindowState == Security) {//창문상태 방범장치 상태 and 확인
    if(irSensorValue < 200 ){ // 창문이 닫혀있고 물체가 감지되면
      activateAlarm();  // 알람작동

      if(StartTime == 0){
        startTime = millis();
        bool SecureFlag = true;
      }

      unsigned long currentTime = millis();

      if(SecureFlag == 1){
        if(currentTime - 5000 > startTime){
          activateAlarm();  // 알람작동
        }
        else if(currentTime < startTime + 500 ){
          lightAlarm();
        }
      }
    }
  }
  else{
    startTime = 0;
    currentTime = 0;
    SecureFlag = false;
  }

  //환풍기 제어
  if (ventflag == 1 ) {
    if (humidity > 70) { 
      startVentMotor();                 
    } 
    else {
      stopVentMotor();
    }
 }

  //온도조절
  if (tempflag == 1) {
    if (desiredTemperature > currentTemperature) {
      setBoilerTemperature(desiredTemperature, currentTemperature);
    }
    else{
      autoActivateAirConditioner();
    }
  }
  //커튼제어
  if (curtainflag == 1) {
    Curtain();
  }
}

//커튼 제어 함수
void Curtain() {
  uint16_t aValue = analogRead(Cds_Photoresistor_PIN);
  Serial.println(aValue);
  delay(100);
  if (aValue > 512) {
    // 정방향으로 3초 동안 회전
    myStepper.step(STEPPERREV);
    delay(3000);
  } 
  else {
    // 역방향으로 3초 동안 회전
    myStepper.step(-STEPPERREV);
    delay(3000);
  }
}


//보일러 제어 함수
void setBoilerTemperature(float desiredTemperature, float currentTemperature){        
  float diffTemp = desiredTemperature - currentTemperature;
  float fadeSpeed = map(abs(diffTemp), 0, 10, 6, 1); // diffTemp에 따라 fadeSpeed동적으로 계산

  int brightness = 255; // 초기 밝기 값을 255로 설정
  analogWrite(boilerLEDPin, brightness);  // 최대 밝기로 설정

  // LED의 밝기를 31까지 부드럽게 감소
  while (brightness >= 31) {
    // 현재 시간과 시작 시간 간의 차이 계산
    unsigned long CurrentTime = millis() - startTime;

    // 일정 시간이 지나면 루프 종료
    if (CurrentTime >= 100) {  // 100밀리초 대기
      analogWrite(boilerLEDPin, brightness);
      brightness -= fadeSpeed;  // diffTemp에 의해 반비례하는 fadeSpeed 사용
      startTime = millis();  // 시작 시간 갱신
    }
  }
  if (brightness < 32){
    analogWrite(boilerLEDPin, 31);  // LED의 밝기를 63으로 유지
  }

}

//환풍기 작동 함수
void startVentMotor() {                    
  analogWrite(VentmotorPin, 255);  // 환풍기 DC 모터를 최대 속도로 작동
}

//환풍기 종료 함수
void stopVentMotor() {                  
  analogWrite(VentmotorPin, 0);  // 환풍기 DC 모터를 정지
}

// 에어컨 함수
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