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
  int preValue = analogRead(2);
  Serial.println(preValue);
  if ( irSensorValue <200) {  // 창문이 닫혀있고 and 적외선 센서값이 high이면 ( 물체가 감지되면 )
  activateAlarm();  // 경보 함수 발동
  } 
}
