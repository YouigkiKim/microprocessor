#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include "Arduino.h"

#define DHTPIN 4          // DHT 센서의 데이터 핀
#define DHTTYPE DHT22     // DHT 센서의 타입 (DHT22 또는 DHT11)
DHT dht(DHTPIN, DHTTYPE);

unsigned long startTime = 0; // 시작 시간 저장 변수

void setup() {
  pinMode(boilerLEDPin, OUTPUT); // 보일러 선 led
  dht.begin();
}

void loop() {
  float currentTemperature = dht.readTemperature();     //dht_22의 실내 온도 측정값
  float desiredTemperature = 22
  setBoilerTemperature(float desiredTemperature, float boilercurrentTemp);
}


//보일러 제어 함수
void setBoilerTemperature(float desiredTemperature, float boilercurrentTemp){
  if (boilerflag = 1){
    float boilercurrentTemp = dht.readTemperature();
    float diffTemp = desiredTemperature - boilercurrentTemp;
    float fadeSpeed = map(abs(diffTemp), 0, 10, 6, 1); // diffTemp에 따라 fadeSpeed 동적으로 계산
  }        
  int boilerflag = 0;
  float fadeSpeed = 3;
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
    int boilerflag = 1;
  }
}