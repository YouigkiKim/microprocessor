#include <Servo.h>
#include <SoftwareSerial.h>

//리모컨 제작 후 리모컨 송신코드에 따른 창문조작코드 추가 필요
SoftwareSerial BT(2,3); //rx,tx

int irSensorPin = 2;               // 적외선 센서의 핀
int windowServoPin = 7;           // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀

bool WindowClose = true;;
bool WindowAuto = true;
bool WindowState = WindowClose;           //창문상태지정 True=닫힘 False=열림

bool SecurityOn = true;
bool SecurityOff = false;
bool Security = SecurityOff;
uint8_t BTdata = 0;   
//bt.read()의 값이 int값으로 반환되기 때문에 통신을 통해 제어하는 가장 적합한 방법이 정수형을 통한 제어라고 생각됨
Servo windowServo;

void setup() {
  windowServo.attach(7);  // 서보모터를 7번 핀에 연결 :
  pinMode(irSensorPin, INPUT);
  Serial.begin(9600);
  BT.begin(9600);
}

void loop() {

  if(BT.available()){
    BTdata = BT.read();
    Serial.println(BTdata);
  }

  int irSensorValue = analogRead(2);

  if(BTdata == 1){
    Security = SecurityOn;
  }
  else if(BTdata ==2){
    Security = SecurityOff;
  }

  if ( WindowState == Security) {//창문상태 방범장치 상태 and 확인
    if(irSensorValue < 200 ){ // 창문이 닫혀있고 물체가 감지되면
      activateAlarm();  // 알람작동
    }
  }
  
  if(0){
   if(0){
        openWindow();
    }
    else{
      closeWindow();
    }
  }
}

//창문 닫기 함수
void closeWindow() {                      
  windowServo.write(0);  // 닫힌 위치에 해당하는 각도
  delay(500);  // 서보모터가 움직이기를 기다림
  WindowState = true; // 창문상태 닫힘
}

//창문 열기 함수
void openWindow() {                        
  windowServo.write(90);  // 열린 위치에 해당하는 각도
  delay(500);  // 서보모터가 움직이기를 기다림
  WindowState = false; //창문상태 열림]
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
