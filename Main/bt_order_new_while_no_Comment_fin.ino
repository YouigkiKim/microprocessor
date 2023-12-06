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
//float desiredTemperature = 30.0; // 초기 희망온도 값 설정
unsigned long startTime = 0; // 시작 시간 저장 변수

SoftwareSerial BTSerial(2,3); // RX, TX

int lightLEDPin = 5;               // 조명에 사용할 LED 핀
int boilerLEDPin = 6;              // 보일러 선 LED가 연결된 핀
int windowServoPin = 7             // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀
int airConditionerServoPin = 9;    // 에어컨 서보모터 핀
int VentmotorPin = 10;             // 환풍기 DC 모터가 연결된 핀

int airConledPin = 12;             // 에어컨 내부 led 핀
int ledPin = 13;                   // 방범 사이렌 조명 다이오드(LED)가 연결된 핀

int brightness; //보일러LED 밝기 변수 선언
int irSensorValue; // irSensorValue 변수 선언 
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
  pinMode(ledPin, OUTPUT); // 방범 사이렌 led 핀
  
  lcd.begin(16, 2);
  dht.begin();

}

void loop() {
  // 시리얼 모니터로부터 데이터를 읽어서 Bluetooth Serial로 전송
  if (Serial.available()) {
    BTSerial.write(Serial.read());
  }

  // Bluetooth Serial로부터 데이터를 읽어들임
  if (BTSerial.available()) {
    // 읽은 데이터를 변수에 저장
    char data = BTSerial.read();

    // 읽은 데이터를 시리얼 모니터로 출력
    Serial.write(data);

    // 스마트폰 신호가 'a'일 경우 (방범장치 제어)
    if (data == 'a') {
      // Bluetooth Serial로부터 추가 데이터가 있는 동안 반복
      while (BTSerial.available()) {
        // 아날로그 핀을 통해 IR 센서의 값을 읽어들임
        int irSensorValue = analogRead(irSensorPin);

        // 이후 두번째 스마트폰 신호가 '1'인 경우 (방범장치 AUTO모드 작동)
        if (data == '1') {
          // IR 센서 값이 일정 값 미만일 때
          if (irSensorValue < 200) {
            // 알람을 활성화
            activateAlarm();
            
            delay(500);
          } 
          else {
            
            noTone(buzzerPin);
            delay(500);
            digitalWrite(ledPin, LOW);
            delay(500);
          }
        }
        // 이후 두번째 스마트폰 신호가 '0'인 경우 (방범장치 수동 off) 
        else if (data == '0') {
          // 방범 소리 및 빛 전부 끄기
          noTone(buzzerPin);
          delay(500);
          digitalWrite(ledPin, LOW);
          delay(500);
        }
      }  
    }
  float humidity = dht.readHumidity();    // dht_22의 실내 습도 측정값
    // 스마트폰 신호가 'b'일 경우 (환풍기 제어)
    else if (data == 'b') {
      // Bluetooth Serial로부터 추가 데이터가 있는 동안 반복
      while (BTSerial.available()) {
        // 이후 두번째 스마트폰 신호가 'a'인 경우 (환풍기 제어 AUTO모드 작동)
        if (data == 'a') {
          if (humidity > 70) { 
            startVentMotor();                 
          } 
          else {
            stopVentMotor();
          }
        }
        // 이후 두번째 스마트폰 신호가 'b나 c'인 경우 (환풍기 제어 스마트폰 수동 ON/OFF)
        else if (data == 'b') {
          analogWrite(VentmotorPin, 255);
        }
        else if (data == 'c') {
          analogWrite(VentmotorPin, 0);
        }
      }
    }
  // 스마트폰 신호가 'b'일 경우 (환풍기 제어)
    else if (data == 'c') {
      while (BTSerial.available()) {
        if (data == '1') {
          openWindow();
        }
        else if (data == '0') {
          closeWindow();
        }
      }
    }
    float currentTemperature = dht.readTemperature();     //dht_22의 실내 온도 측정값
    // 스마트폰 신호가 'f' 일 경우 (에어컨 제어)
    else if (data == 'f') {
      while (BTSerial.available()) {
    
        if (data == 'a') {
          float desiredTemperature = BTSerial.read();
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
        }
        else if (data == 'c') {
          analogWrite(boilerLEDPin, 0);  //보일러 강제종료
        }
      }
    }
    // 스마트폰 신호가 'e' 일 경우 (원격 조정 조명 밝기 제어)
    else if (data == 'e') {  
    char controlValue = BTSerial.read();
      switch (controlValue) {
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
      while (BTSerial.available()) {
        if (data == 'a') {
          Curtain();
        }
        else if (data == 'b') {
          myStepper.step(STEPPERREV);
          delay(3000);
        }
        else if (data == 'c') {
          myStepper.step(-STEPPERREV);
          delay(3000);
        }
      }
    }
  }
}
//main TX


//커튼 제어 함수
void Curtain() {
  uint16_t aValue = analogRead(Cds_Photoresistor_PIN);
  Serial.println(aValue);
  delay(100);

  if (aValue > 512) {
    // 정방향으로 3초 동안 회전
    myStepper.step(STEPPERREV);
    delay(3000);
  } else {
    // 역방향으로 3초 동안 회전
    myStepper.step(-STEPPERREV);
    delay(3000);
  }
}

//핀 2개만 쓰기위한 라이브러리 설정
Stepper::Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2)
{
  this->step_number = 0;    // which step the motor is on
  this->direction = 0;      // motor direction
  this->last_step_time = 0; // time stamp in us of the last step taken
  this->number_of_steps = number_of_steps; // total number of steps for this motor

  // Arduino pins for the motor control connection:
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);

  // When there are only 2 pins, set the others to 0:
  this->motor_pin_3 = 0;
  this->motor_pin_4 = 0;
  this->motor_pin_5 = 0;

  // pin_count is used by the stepMotor() method:
  this->pin_count = 2;
}

void Stepper::setSpeed(long whatSpeed)
{
  this->step_delay = 60L * 1000L * 1000L / this->number_of_steps / whatSpeed;
}

void Stepper::step(int steps_to_move)
{
  int steps_left = abs(steps_to_move);  // how many steps to take

  // decrement the number of steps, moving one step each time:
  while (steps_left > 0)
  {
    unsigned long now = micros();
    // move only if the appropriate delay has passed:
    if (now - this->last_step_time >= this->step_delay)
    {
      // get the timeStamp of when you stepped:
      this->last_step_time = now;
      // increment or decrement the step number,
      // depending on direction:
      if (steps_to_move > 0)
      {
        this->step_number++;
        if (this->step_number == this->number_of_steps) {
          this->step_number = 0;
        }
      }
      else
      {
        if (this->step_number == 0) {
          this->step_number = this->number_of_steps;
        }
        this->step_number--;
      }
      // decrement the steps left:
      steps_left--;
      // step the motor to step number 0, 1, ..., {3 or 10}
      stepMotor(this->step_number % 4);
    }
  }
}

void Stepper::stepMotor(int thisStep)
{
  switch (thisStep) {
    case 0:  // 01
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, HIGH);
      break;
    case 1:  // 11
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, HIGH);
      break;
    case 2:  // 10
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, LOW);
      break;
    case 3:  // 00
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, LOW);
      break;
  }
}

//보알러 제어 함수
void setBoilerTemperature(float desiredTemperature, float currentTemperature) {        
  const float diffTemp = desiredTemperature - currentTemperature;
  const float fadeSpeed = map(abs(diffTemp), 0, 10, 6, 1); // diffTemp에 따라 fadeSpeed 동적으로 계산

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
  delay(500);
}

//환풍기 종료 함수
void stopVentMotor() {                  
  analogWrite(VentmotorPin, 0);  // 환풍기 DC 모터를 정지
  delay(500);
}

// 에어컨 함수
void autoActivateAirConditioner() {
 
  // 에어컨을 작동하는 코드 추가
  digitalWrite(airConledPin, HIGH);
  delay(500);

  // 에어컨 작동을 표현하기 위해 서보모터를 작동시킴

  airConditionerServo.attach(airConditionerServoPin);
  airConditionerServo.write(90);
  delay(500);
  airConditionerServo.detach();
}

//LCD와 시리얼 모니터에 현재 온습도 표현 함수
void displayTemperatureAndHumidity() {             
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float desiredTemperature = 28.0;
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

  lcd.setCursor(0, 2);   //2행 희망온도
  lcd.print("Desired Temp: ");
  lcd.print("28");
  lcd.print(" C");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Desired Temp: ");
  Serial.print(desiredTemperature);
  Serial.println(" °C");
}

//창문 닫기 함수
void closeWindow() {                      
  windowServo.write(0);  // 닫힌 위치에 해당하는 각도
  delay(500);  // 서보모터가 움직이기를 기다림
}

//창문 열기 함수
void openWindow() {                        
  windowServo.write(90);  // 열린 위치에 해당하는 각도
  delay(500);  // 서보모터가 움직이기를 기다림
}

//방범장치 함수
void activateAlarm() {            
  unsigned long startTime = millis();  // 현재 시간 기록
  while (millis() - startTime < 10000) {  // 10초 동안 반복
    tone(buzzerPin, 1000); // 피에조 부저 켜기
    digitalWrite(ledPin, HIGH);  // 다이오드(LED) 켜기
    delay(500);  // 0.5초 대기
    noTone(buzzerPin);  // 피에조 부저 끄기
    digitalWrite(ledPin, LOW);   // 다이오드(LED) 끄기
    delay(500);  // 0.5초 대기
  }
}