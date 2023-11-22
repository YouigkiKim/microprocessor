
#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>



int boilerLEDPin = 6 ;             // 보일러 선LED        _PWM핀 사용해야하므로    ~가 있는 6번핀 사용
int brightness;

#define DHTPIN 4          // DHT 센서의 데이터 핀
#define DHTTYPE DHT22     // DHT 센서의 타입 (DHT22 또는 DHT11)
DHT dht(DHTPIN, DHTTYPE);
float desiredTemperature = 30.0; // 초기 희망온도 값 설정
unsigned long startTime = 0; // 시작 시간 저장 변수

void setup() {
  pinMode(boilerLEDPin, OUTPUT); // 보일러 선 led
  Serial.begin(9600);
  dht.begin();
  startTime = millis(); // 초기 시작 시간 설정
}

void loop() {
  float currentTemperature = dht.readTemperature();     //dht_22의 실내 온도 측정값

  // 아두이노에서 Serial 통신을 통해 희망온도 설정
if (Serial.available() > 0) {
    float newDesiredTemperature = Serial.parseFloat();

    if (newDesiredTemperature != desiredTemperature) { //만약 새롭게 설정한 값이 기존값과 다르면
      desiredTemperature = newDesiredTemperature;
      analogWrite(boilerLEDPin, 255);   //최대 밝기로 선LED 작동
      startTime = millis();      // 시작 시간 갱신
    }
  }
  

  if (desiredTemperature > currentTemperature) {
    setBoilerTemperature(desiredTemperature, currentTemperature);
    
  } 
  else {
    analogWrite(boilerLEDPin, 0);
  }
  
  Serial.print("Temperature: ");
  Serial.print(currentTemperature);
  Serial.print(" °C, Desired Temp: ");
  Serial.print(desiredTemperature);
  Serial.println(" °C");
  
}


  


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
