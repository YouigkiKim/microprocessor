#include <Servo.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

Servo windowServo;
Servo airConditionerServo;  // 에어컨을 제어할 별도의 서보모터
SoftwareSerial bluetooth(0, 1);  // RX, TX
int irSensorPin = 2;               // 적외선 센서의 핀
int lightLEDPin = 5;               // 조명에 사용할 LED 핀
int boilerLEDPin = 6;              // 보일러 선 LED가 연결된 핀
int windowServoPin = 7             // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀
int airConditionerServoPin = 9;    // 에어컨 서보모터 핀
int VentmotorPin = 10;             // 환풍기 DC 모터가 연결된 핀
int airConditionerMotorPin = 11;   // 에어컨 DC모터 핀
int airConledPin = 12;             // 에어컨 내부 led 핀
int ledPin = 13;                   // 방범 사이렌 조명 다이오드(LED)가 연결된 핀
int lightBrightness = 0; // 조명의 현재 밝기 설정 (기본값: 0, 꺼짐)

#define DHTPIN 3          // DHT 센서의 데이터 핀
#define DHTTYPE DHT22     // DHT 센서의 타입 (DHT22 또는 DHT11)
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27에 연결된 16x2 LCD

void setup() {
  windowServo.attach(7);  // 서보모터를 7번 핀에 연결  
  pinMode(irSensorPin, INPUT);   // 적외선센서
  pinMode(buzzerPin, OUTPUT);    // 피에조 부저
  pinMode(ledPin, OUTPUT);       // 방범 사이렌 led
  pinMode(VentmotorPin, OUTPUT); // 환풍기 dc모터
  pinMode(boilerLEDPin, OUTPUT); // 보일러 선 led
  pinMode(lightLEDPin, OUTPUT);  // 조명 LED 핀 초기화

  Serial.begin(9600);
  bluetooth.begin(9600);
  lcd.begin(16, 2);
  dht.begin();
}

void loop() {
  if (bluetooth.available() > 0) {
    char command = bluetooth.read();
     if (command == 'O') {
      // 'O' 커맨드가 들어오면 서보모터(창문)를 열음
      openWindow();
    } else if (command == 'C') {
      // 'C' 커맨드가 들어오면 서보모터(창문)를 닫음
      closeWindow();
    } else if (command == 'T') {
      // 'T' 커맨드가 들어오면 선LED(보일러)를 킴
      setBoilerTemperature();
    } else if (command == 'N') {
      // 'N' 커맨드가 들어오면 선LED(보일러)를 끔
      analogWrite(boilerLEDPin, 0);
    } else if (command == 'S') {  // 새로운 커맨드 'S' 추가
      stopAirConditioner();      // 에어컨 정지 함수 호출
    } else if (command == 'L') {
      controlLight();            // L0, L1 ,L2 ,L3 ,L4를 통해 각각 OFF 25% 50% 75% 100% 조명
    }
    
  }

  float currentTemperature = dht.readTemperature();     //dht_22의 실내 온도 측정값
  const float thresholdTemperature = 24.0;            // 일정 온도 수치
  if (currentTemperature > thresholdTemperature) {  // 예시 온도가 24도 이상일 때 작동
    // 온도가 일정 값보다 높으면 에어컨을 작동시킴
    autoActivateAirConditioner();   // 블루투스와 무관한 에어컨 자동 작동 함수(dc모터,서보모터,led) 발동
  }


  int irSensorValue = digitalRead(irSensorPin);   // 적외선 센서 값 
  if (windowIsClosed() && irSensorValue == HIGH) {  // 창문이 닫혀있고 and 적외선 센서값이 high이면 ( 물체가 감지되면)

    activateAlarm();  // 경보 함수 발동
  } 


  displayTemperatureAndHumidity(getDesiredTemperature());  // 희망온도를 변수로 가지는 LCD 온습도계 함수 실행


  float humidity = dht.readHumidity();    // dht_22의 실내 습도 측정값
  if (humidity > 50) {              // 50을 기준으로 환풍기 작동함수, 환풍기 종료함수 실행
    startVentMotor();                
  } else {
    stopVentMotor();
  }


  checkBoilerTemperature();  // 보일러가 켜졋나 안켜졌나를 시리얼 모니터에 표현하는 함수 --> 스마트폰에까지 연결할 예정

  controlAirConditioner(getDesiredTemperature(), dht.readTemperature());   
  //희망온도와 DHT의 현재온도를 변수로 가지는 에어컨 제어 함수 실행

  // 스텝모터와 Stepping Motor Driver: ULN2003 드라이버를 활용한 커튼 작동 함수 추가 예정

  //void loop 종료
}


// 희망온도 설정 함수
int getDesiredTemperature() {              
  if (bluetooth.available() > 0) {
    char command = bluetooth.read();
    if (command == 'T') {                  
      return bluetooth.parseInt();
    }
  }
  return 0;  // 기본값을 0으로 설정하거나 다른 값으로 변경 가능
}

// 방범장치 조건에서 창문 닫혀 있음을 알기위한 함수
bool windowIsClosed() {                    
  return (windowServo.read() == 0);
}


//창문 열기 함수
void openWindow() {                        
  windowServo.write(90);  // 열린 위치에 해당하는 각도
  delay(500);  // 서보모터가 움직이기를 기다림
}


//창문 닫기 함수
void closeWindow() {                      
  windowServo.write(0);  // 닫힌 위치에 해당하는 각도
  delay(500);  // 서보모터가 움직이기를 기다림
}


//방범장치 함수
void activateAlarm() {            
  unsigned long startTime = millis();  // 현재 시간 기록
  tone(buzzerPin, 1000);  // 피에조 부저 켜기
  while (millis() - startTime < 10000) {  // 10초 동안 반복
    delay(500);  // 0.5초 대기
    digitalWrite(ledPin, HIGH);  // 다이오드(LED) 켜기
    delay(500);  // 0.5초 대기
    digitalWrite(ledPin, LOW);   // 다이오드(LED) 끄기
    delay(500);  // 0.5초 대기
  }
  noTone(buzzerPin);  // 피에조 부저 끄기
}


// LCD에 실내 온습도계, 희망온도 표현 함수
void displayTemperatureAndHumidity() {             
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  lcd.clear();
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
  lcd.print(desiredTemperature);
  lcd.print(" C");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Desired Temp: ");
  Serial.println(" °C");
}

//환풍기 작동 함수
void startVentMotor() {                    
  analogWrite(VentmotorPin, 255);  // 환풍기 DC 모터를 최대 속도로 작동
}

//환풍기 종료 함수
void stopVentMotor() {                  
  analogWrite(VentmotorPin, 0);  // 환풍기 DC 모터를 정지
}


//보일러 선 LED 함수
void setBoilerTemperature() {          
  int desiredTemperature = bluetooth.parseInt();  // 스마트폰으로부터 희망 온도 읽기
  float currentTemperature = dht.readTemperature();  // 현재 온도 측정

  const int fadeSpeed = 5;  // 밝기를 변경하는 속도 (낮을수록 빠름)

  if (desiredTemperature > currentTemperature) {
    // 희망 온도가 현재 온도보다 높을 경우 LED를 설정
    analogWrite(boilerLEDPin, 255);  // 최대 밝기로 설정
    delay(1000);  // 최대 밝기에서 1초 대기

    // LED의 밝기를 63까지 부드럽게 감소
    for (int brightness = 255; brightness >= 63; brightness -= fadeSpeed) {
      analogWrite(boilerLEDPin, brightness);  // PWM을 사용하여 LED의 밝기 설정
      delay(10);  // 각 단계마다 일정 시간 동안 대기
    }
  } else {
    // 희망 온도가 현재 온도보다 낮을 경우 LED를 끔
    analogWrite(boilerLEDPin, 0);
  }
}

void checkBoilerTemperature() {
  // 보일러 작동 여부 확인
  // 이 부분은 필요에 따라 추가 해야한다.
  // 스마트폰 화면으로 연결하는 부분을 추가할 예정
  // 이 코드는 시리얼 모니터에서 보일러 작동 여부를 확인하는 코드.
  int boilerStatus = digitalRead(boilerLEDPin);
  if (boilerStatus == HIGH) {
    Serial.println("Boiler is ON");
  } else {
    Serial.println("Boiler is OFF");
  }
}

//에어컨&블루투스 희망온도 제어 작동 함수
void controlAirConditioner(float desiredTemperature, float currentTemperature) {
  const int airConditionerMotorPin = 11;
  const int airConditionerServoPin = 9;
  const int airConledPin = 12;

  if (desiredTemperature < currentTemperature) {
    // 희망 온도가 현재 온도보다 낮을 경우 실외기(DC모터)와 에어컨 표시 LED를 작동시킴
    analogWrite(propellerPin, 255);
    analogWrite(airConditionerMotorPin, 255);  // DC모터를 최대 속도로 작동
    delay(500);

    digitalWrite(airConledPin, HIGH);

    // 에어컨 작동을 표현하기 위해 서보모터를 작동시킴
    airConditionerServo.write(90);
    delay(500);
    airConditionerServo.detach();
  } else {
    // 희망 온도가 현재 온도보다 높거나 같을 경우 실외기(DC모터)와 에어컨 표시 LED를 정지시킴
    analogWrite(airConditionerMotorPin, 0);  // DC모터를 정지
    digitalWrite(airConledPin, LOW);
    Servo airConditionerServo;
    airConditionerServo.attach(airConditionerServoPin);
    airConditionerServo.write(0);  // 반대 방향으로 회전
    delay(500);
    airConditionerServo.detach();  // 서보모터를 해제함
  }
}


// 에어컨 종료 함수
void stopAirConditioner() {
  
  const int airConditionerMotorPin = 11;
  const int airConditionerServoPin = 9;
  const int airConledPin = 12;

  // 실외기(모터)와 에어컨 표시 LED를 정지시킴
  analogWrite(airConditionerMotorPin, 0);

  digitalWrite(airConledPin, LOW);

  // 에어컨을 멈춤을 표현하기위해 서보모터를 반대 방향으로 회전시킴
  Servo airConditionerServo;
  airConditionerServo.attach(airConditionerServoPin);
  airConditionerServo.write(0);  // 반대 방향으로 회전
  delay(500);
  airConditionerServo.detach();  // 서보모터를 해제함
}


// 조명 원격 제어 함수
void controlLight() {
  char brightnessCommand = bluetooth.read();
  int brightnessValue;

  // 'L' 뒤에 따라오는 커맨드에 따라 밝기 설정 L0 L1 L2 L3 L4 등등
  switch (brightnessCommand) {
    case '0':
      brightnessValue = 0;  // 조명 끄기
      break;
    case '1':
      brightnessValue = 64;  // 25% 밝기
      break;
    case '2':
      brightnessValue = 128;  // 50% 밝기
      break;
    case '3':
      brightnessValue = 192;  // 75% 밝기
      break;
    case '4':
      brightnessValue = 255;  // 100% 밝기
      break;
    default:
      brightnessValue = 0;  // 기본값은 조명 끄기
      break;
  }

  // LED 밝기 조절
  analogWrite(lightLEDPin, brightnessValue);

  // 현재 밝기 갱신
  lightBrightness = brightnessValue;
}


// DHT-22의 온도 측정값에 따라서만 작동하는 에어컨 함수-->블루투스 배제
void autoActivateAirConditioner() {
  const int airConditionerMotorPin = 11;
  const int airConledPin = 12;
  const int airConditionerServoPin = 9;

  // 에어컨을 작동하는 코드 추가
  analogWrite(airConditionerMotorPin, 255);  // DC모터를 최대 속도로 작동
  delay(500);

  digitalWrite(airConledPin, HIGH);

  // 에어컨 작동을 표현하기 위해 서보모터를 작동시킴
  Servo airConditionerServo;
  airConditionerServo.attach(airConditionerServoPin);
  airConditionerServo.write(90);
  delay(500);
  airConditionerServo.detach();
}