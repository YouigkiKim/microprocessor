#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include <Servo.h>

#define irSensorPin A2               // 적외선 센서의 핀
#define DHTPIN 4          // DHT 센서의 데이터 핀
#define DHTTYPE DHT22     // DHT 센서의 타입 (DHT22 또는 DHT11)
DHT dht(DHTPIN, DHTTYPE);
Servo airConditionerServo;
unsigned long startTime = 0; // 시작 시간 저장 변수
bool ventState;

SoftwareSerial BTSerial(2,3); // RX, TX

int boilerLEDPin = 6;              // 보일러 선 LED가 연결된 핀
int windowServoPin = 7;             // 창문 여닫이 서보모터가 연결된 핀

int airConditionerServoPin = 9;    // 에어컨 서보모터 핀
int VentmotorPin = 10;             // 환풍기 DC 모터가 연결된 핀
int airConledPin = 12;             // 에어컨 내부 led 핀

//창문상태지정 True=닫힘 False=열림
float currentTemperature;
// unsigned long startTime;
// unsigned long currentTime;
int brightness; //보일러LED 밝기 변수 선언
// int irSensorValue; // irSensorValue 변수 선언 
// char data;

bool ventflag = 0; 
bool tempflag = 0; // 변경: tempflag를 전역 변수로 변경
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27에 연결된 16x2 LCD

float desiredTemperature;  // 추가: desiredTemperature 변수 선언
float humidity;  // 추가: humidity 변수 선언

void setup() {
  Serial.begin(9600);
  // BTSerial.begin(9600);
  // myStepper.setSpeed(rpm);
  // pinMode(lightLEDPin, OUTPUT);  // 조명 LED 핀
  pinMode(boilerLEDPin, OUTPUT); // 보일러 선 led
  // windowServo.attach(7); // 창문 서보모터를 7번 핀에 연결  
  // pinMode(buzzerPin, OUTPUT);    // 피에조 부저
  airConditionerServo.attach(9);  // 에어컨 서보모터를 7번 핀에 연결
  pinMode(VentmotorPin, OUTPUT); // 환풍기 dc모터
  pinMode(airConledPin, OUTPUT); // 에어컨 내부 led 핀
  lcd.begin(16, 2);
  dht.begin();
  lcd.clear();
  lcd.init();
  lcd.backlight();
}


void loop() {
  float humidity = dht.readHumidity();    // dht_22의 실내 습도 측정값
  float currentTemperature = dht.readTemperature();     //dht_22의 실내 온도
  // if (BTSerial.available()){
  //   float desiredTemperature = BTread.()
  // }
  float desiredTemperature = 44;
  Serial.print("humidity: ");
  Serial.println(humidity);
  Serial.print("currentTemperature: ");
  Serial.println(currentTemperature);

  Serial.print("loop desiredTemperature");
  Serial.println(desiredTemperature);

  lcd.setCursor(0, 0);   //0행 실내온도
  lcd.print("Temp: ");
  lcd.print(currentTemperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);   //1행 실내습도
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  // displayTemperatureAndHumidity();

  if (ventflag == 0 ) {
    Serial.println(" ventflag");
    if (humidity > 70) {
      Serial.println("startVent"); 
      startVentMotor();                 
    } 
    else {
      Serial.println("stopVent");
      stopVentMotor();
    }
  }
  Serial.print("before tempflag ");
  Serial.println(tempflag);

  //온도조절
  if (tempflag == 0) {
    Serial.println("tempflag");
    Serial.print("desiredTemperature");
    Serial.println(desiredTemperature);
    Serial.print("currentTemperature");
    Serial.println(currentTemperature);
    unsigned long startTime = millis();
    if (desiredTemperature > currentTemperature) {
      Serial.println("setBoiler");
      setBoilerTemperature(desiredTemperature, currentTemperature);
      offAirConditioner();
    }
    else{
      Serial.println("offBoiler");
      autoActivateAirConditioner();
      offBoilerTemperature();
    }
  }
  else{
    tempflag = 0;
    ventflag = 0;
  }
}


//보일러 제어 함수
void setBoilerTemperature(float desiredTemperature, float currentTemperature){

  Serial.println("check setBoiler");
  float diffTemp = desiredTemperature - currentTemperature;
  float fadeSpeed = map(abs(diffTemp), 0, 60, 6, 1); // diffTemp에 따라 반비레로 fadeSpeed 변화   
  Serial.print("diffTemp ");
  Serial.println(diffTemp);
  Serial.print("fadeSpeed ");
  Serial.println(fadeSpeed);   

  int brightness = 255; // 초기 밝기 값을 255로 설정

  analogWrite(boilerLEDPin, brightness);  // 최대 밝기로 설정

  // LED의 밝기를 31까지 부드럽게 감소
  if (brightness >= 31) {
    Serial.print("check while brightness: ");
    Serial.println(brightness);

    // 현재 시간과 시작 시간 간의 차이 계산
    unsigned long CurrentTime = millis();
    Serial.print("CurrentTime ");
    Serial.println(CurrentTime);

    Serial.print("millis ");
    Serial.println(millis());

    // 일정 시간이 지나면 루프 종료
    if (millis() - CurrentTime < 500) {  // 100밀리초 대기
      analogWrite(boilerLEDPin, brightness);

      brightness -= fadeSpeed;  // diffTemp에 의해 반비례하는 fadeSpeed 사용
      Serial.print("brightness after ");
      Serial.println(brightness);
      CurrentTime = millis();  // 시작 시간 갱신
      Serial.print("CurrentTime after ");
      Serial.println(CurrentTime);
    }
  }
  if (brightness < 32){
    analogWrite(boilerLEDPin, 31);  // LED의 밝기를 63으로 유지
  }

}

//보일러 종료 함수
void offBoilerTemperature(){        
  analogWrite(boilerLEDPin, 0);  // 최대 밝기로 설정

}

//환풍기 작동 함수
void startVentMotor() {                    
  analogWrite(VentmotorPin, 255);  // 환풍기 DC 모터를 최대 속도로 작동
  ventState= true;
}

//환풍기 종료 함수
void stopVentMotor() {                  
  analogWrite(VentmotorPin, 0);  // 환풍기 DC 모터를 정지
  ventState= false;
}

// 에어컨 함수
void autoActivateAirConditioner() {
 
  // 에어컨을 작동하는 코드 추가
  digitalWrite(airConledPin, HIGH);
  delay(100);

  // 에어컨 작동을 표현하기 위해 서보모터를 작동시킴

  airConditionerServo.write(90);

  
}

void offAirConditioner() {
 
  // 에어컨을 종료하는 코드 추가
  digitalWrite(airConledPin, LOW);
  delay(100);

  // 에어컨 종료를 표현하기 위해 서보모터를 반대로 작동시킴
  airConditionerServo.write(0);
}

//LCD와 시리얼 모니터에 현재 온습도 표현 함수
void displayTemperatureAndHumidity() {             


  
  lcd.setCursor(0, 0);   //0행 실내온도
  lcd.print("Temp: ");
  lcd.print(currentTemperature);
  lcd.print(" C");
  Serial.println(" Check lcd");

  lcd.setCursor(0, 1);   //1행 실내습도
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  Serial.print("Temperature: ");
  Serial.print(currentTemperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}
