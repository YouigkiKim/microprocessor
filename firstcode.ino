#include <DHT.h>
#include <Servo.h>

#define DHT_PIN 2  // DHT11 센서 데이터 핀
#define DHT_TYPE DHT11  // DHT11 센서 유형
#define MOTOR_PIN 8  // DC 모터 핀

DHT dht(DHT_PIN, DHT_TYPE);
Servo myServo;  // 서보모터 객체 생성

int photoresistorOutdoor = A0;  // 실외 조도센서 핀
int photoresistorIndoor = A1;   // 실내 조도센서 핀
int threshold = 100;  // 조도센서 임계값
int ledPin = 13;  // LED 핀 (예시로 13번 핀 사용)

unsigned long previousMillisTemp = 0;
unsigned long previousMillisLight = 0;
const long intervalTemp = 2000;  // 온습도 센서 작업 간격 (밀리초)
const long intervalLight = 2000; // 조도센서 작업 간격 (밀리초)

void setup() {
  Serial.begin(9600);
  dht.begin();
  myServo.attach(9); // 서보모터 핀
  pinMode(ledPin, OUTPUT);  // LED 핀을 출력으로 설정
  pinMode(MOTOR_PIN, OUTPUT);  // DC 모터 핀을 출력으로 설정
}

void loop() {
  unsigned long currentMillis = millis();

  // 온습도 센서 작업
  if (currentMillis - previousMillisTemp >= intervalTemp) {
    previousMillisTemp = currentMillis;
    // 온습도 센서에서 읽기 작업 수행
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    Serial.print("습도: ");
    Serial.print(humidity);
    Serial.print("%, 온도: ");
    Serial.println(temperature);
  
    // 예시: 온도가 28도를 넘거나 습도가 60%를 넘을 때 모터를 켜세요
    if (temperature > 28 || humidity > 60) {
      digitalWrite(MOTOR_PIN, HIGH); // 모터 켜기
    } else {
      digitalWrite(MOTOR_PIN, LOW); // 모터 끄기
    }
  }
