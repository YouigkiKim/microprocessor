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
Servo windowServo;
SoftwareSerial BTserial(2,3);

int boilerLEDPin = 6;              // 보일러 선 LED가 연결된 핀
int VentmotorPin = 10;             // 환풍기 DC 모터가 연결된 핀
float currentTemperature;
float updateTemp;
int brightness; //보일러LED 밝기 변수 선언
bool ventState;
bool ventflag = 0; 
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27에 연결된 16x2 LCD
float desiredTemperature;  // 추가: desiredTemperature 변수 선언
float humidity;  // 추가: humidity 변수 선언
bool Tempflag;

//창문변수들
//창문상태
bool WindowClose = true;
bool WindowAuto = true;
bool WindowState = WindowClose; 

//방범변수들
//시간변수
unsigned long startTime = 0; // 시작 시간 저장 변수
unsigned long alarmStart = 0;
unsigned long startTimelight;
unsigned long actstartTime;
unsigned long alarmcurrent;
unsigned long currentTime;
unsigned long exTF;
//핀설정
int windowServoPin = 7;             // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀
int irSensorValue; // irSensorValue 변수 선언 
//방범상태
bool Security = true;
bool SecureFlag;
bool AlarmFlag = false;


void setup(){
  // put your setup code here, to run once:
  pinMode(VentmotorPin, OUTPUT); // 환풍기 dc모터
  pinMode(buzzerPin, OUTPUT);    // 피에조 부저
  Serial.begin(9600);
  BTserial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.init();
  lcd.backlight();
  dht.begin();
  attachInterrupt(digitalPinToInterrupt(2),SoftwareISR,FALLING);
  attachInterrupt(digitalPinToInterrupt(0),SoftwareISRRx,FALLING);
}

void loop(){
  float humidity = dht.readHumidity();    
  float currentTemperature = dht.readTemperature();     
  
  //두번째 아두이노로 온습도 희망온도 전송 1초마다
  if(exTF> millis()-1000){
    Serial.print(currentTemperature);
    Serial.print(",");
    Serial.print(humidity);
    Serial.print(",");
    Serial.println(desiredTemperature);
  }
  //LCD온습도 출력
  displayTemperatureAndHumidity(currentTemperature,humidity);

  //환풍기제어
  if (ventflag == 1 ) {
    if (humidity > 70) {
      analogWrite(VentmotorPin, 255);  // 환풍기 DC 모터를 최대 속도로 작동
      ventState= true;
    }
    else {
      analogWrite(VentmotorPin, 0);  // 환풍기 DC 모터를 정지
      ventState= false;
    }
  }

  //방범
  //창문 제어
  if ( WindowState == Security) {//창문상태 방범장치 상태 and 확인
    if(irSensorValue < 200 ){ // 창문이 닫혀있고 물체가 감지되면
      if(startTime == 0){
        startTime = millis();
        SecureFlag = true;
      }

      unsigned long currentTime = millis();
      if(SecureFlag == true){
        if(currentTime - 5000 > startTime){
          if(AlarmFlag == 0){
            actstartTime = millis();
            AlarmFlag = true;
            alarmStart = millis();  // 현재 시간 기록
          } 
          if(AlarmFlag == true){
            alarmcurrent = millis();
            activateAlarm();  // 알람작동
          }
        }
        else if(currentTime < startTime + 500 ){
          lightAlarm();
        }
      }
      
    } 
    
    else{
      startTime = 0;
      currentTime = 0;
      SecureFlag = false;
      AlarmFlag = false;
      noTone(buzzerPin);
    }
  }
}

void SoftwareISRRx(){
  char data;
  while(BTserial.available()){
    data = BTserial.read();
  }
  updateTemp = data.toFloat();
}

void SoftwareISR(){
  while(BTserial.available()){

    char data = BTserial.read();

    if(data == 'a'){Security = true;}
    else if(data == 'b'){Security = false;}
    //환풍기 AUTO ON OFF
    else if(data == 'c'){ventflag = 1;}
    else if(data == 'd'){
      analogWrite(VentmotorPin, 255);
      ventflag = 0;
    }
    else if(data == 'e'){
      analogWrite(VentmotorPin,0);
      ventflag = 0;
    }
    else if(data == 'f'){
      openWindow();
    }
    else if(data == 'g'){
      closeWindow();
    }
    else if(data == 'h'){
      tempflag = 1;
      Serial.print(h);
    }
  //   else if(data == 'i'){
  //     digitalWrite(airConledPin, LOW);
  //     airConditionerServo.attach(airConditionerServoPin);
  //     airConditionerServo.write(0);
  //     delay(500);
  //     airConditionerServo.detach();
  //     int tempflag = 0;
  //   }
  //   else if(data == 'j'){
  //     analogWrite(boilerLEDPin, 0);  //보일러 강제종료
  //     int tempflag = 0;
  //   }
  //   else if(data == 'k'){}//에어컨 터보
  //   else if(data == 'l'){}//보일러 터보
  //   else if(data == 'm'){}
  //   else if(data == 'n'){}
  //   else if(data == 'o'){}

  //   //조명
  //   else if(data == '0'){analogWrite(lightLEDPin,LOW);}//off
  //   else if(data == '1'){analogWrite(lightLEDPin,32);}//off
  //   else if(data == '2'){analogWrite(lightLEDPin,64);}//off
  //   else if(data == '3'){analogWrite(lightLEDPin,128);}//off
  //   else if(data == '4'){analogWrite(lightLEDPin,196);}//off
  //   else if(data == '5'){analogWrite(lightLEDPin,255);}//off
  //}
    else if(Serial.find(".")){
      desiredTemperature = toFloat(data);
      Serial.println(desireTemperature);
      Tempflag = 1;
      Serial.print("z");
    }
    else if(data == 'x'){
      BTserial.print(currentTemperature);
      BTserial.println("'C");
      BTserial.print(humidity);
      BTserial.println("%");
    }
    else{
      Serial.print(data);
    }
  }
}

//LCD와 시리얼 모니터에 현재 온습도 표현 함수
void displayTemperatureAndHumidity(float currentTemperature,float humidity ){
  lcd.setCursor(0, 0);//0행 실내온도
  lcd.print("Temp: ");
  if(Tempflag == True){
    lcd.print(updateTemp);
  }
  else{
    lcd.print(currentTemp);
  }
  lcd.print(currentTemperature);
  lcd.print(" C");
  Serial.println(" Check lcd");
  lcd.setCursor(0, 1);   //1행 실내습도
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");
}

void activateAlarm(){
  if(alarmcurrent - actstartTime < 10000){  // 현재 시간 기록 8000 ){
    tone(buzzerPin, 1000); 
  }
  else{
    noTone(buzzerPin);
    alarmStart = 0 ;
    AlarmFlag = 0;
  }
}

//방범장치 함수
void lightAlarm(){
  startTimelight = millis();
  Serial.println("liht arlded: ");
  while (millis() - startTimelight < 1000){
    tone(buzzerPin, 1000);
    delay(500);
    noTone(buzzerPin);
  }
}

//창문 열기 함수
void openWindow(){
  windowServo.attach(7);
  delay(1000); 
  detach();
  windowServo.WindowState= false;
}

//창문 닫기 함수
void closeWindow(){
  windowServo.attach(7);
  windowServo.write(0);  // 닫힌 위치에 해당하는 각도
  delay(1000);
  windowServo.detach();
  WindowState= true;
}