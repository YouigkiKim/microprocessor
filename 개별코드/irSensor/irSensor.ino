#include <Servo.h>
#include <SoftwareSerial.h>

int irSensorPin = 2;               // 적외선 센서의 핀
int windowServoPin = 7;           // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀


Servo windowServo;

void setup() {
  windowServo.attach(7);  // 서보모터를 7번 핀에 연결 :
  pinMode(irSensorPin, INPUT);   // 
  Serial.begin(9600);

}

void loop() {
  int irSensorValue = analogRead(2);
  Serial.println(irSensorValue);
  if ( irSensorValue <200) {  // 창문이 닫혀있고 and 적외선 센서값이 high이면 ( 물체가 감지되면 )
  activateAlarm();  // 경보 함수 발동
  } 
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

// 방범장치 조건에서 창문 닫혀 있음을 알기위한 함수
bool windowIsClosed() {    
  return (windowServo.read() == 0);
}

//방범장치 함수
void activateAlarm() {            
  unsigned long startTime = millis();  // 현재 시간 기록
  while (millis() - startTime < 10000) {  // 10초 동안 반복
    tone(buzzerPin, 1000);
    delay(500);  // 0.5초 대기
    noTone(buzzerPin);  // 피에조 부저 끄기
    delay(500);  // 0.5초 대기
  }
}
