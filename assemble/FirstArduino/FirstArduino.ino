#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include <Servo.h>

#define irSensorPin A2               // 적외선 센서의 핀
#define BTRx 2
#define BTTx 3
#define DHTPIN 4          // DHT 센서의 데이터 핀
#define boilerLEDPin 6
#define buzzerPin 8
#define windowServoPin 9
#define VentmotorPin 10
#define DHTTYPE DHT22     // DHT 센서의 타입 (DHT22 또는 DHT11)

DHT dht(DHTPIN, DHTTYPE);
Servo windowServo;
SoftwareSerial BTserial(BTRx,BTTx);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27에 연결된 16x2 LCD

float currentTemperature;
float updateTemp;
float LCDTemp;
int brightness; //보일러LED 밝기 변수 선언
bool ventState;
bool ventflag = 0;
float desiredTemperature;  // 추가: desiredTemperature 변수 선언
float humidity;  // 추가: humidity 변수 선언
bool Tempflag = false;
char data = 'y';
float BTSerialTemp;
float Serialdata;


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
unsigned long LT = 0;
unsigned long lastSerial ;
unsigned long lastflag ;


int irSensorValue; // irSensorValue 변수 선언 
//방범상태
bool Security = true;
bool SecureFlag;
bool AlarmFlag = false;


void setup(){
  //OUTPUT 핀모드 세팅
  pinMode(VentmotorPin, OUTPUT); // 환풍기 dc모터
  pinMode(buzzerPin, OUTPUT);    // 피에조 부저
  //통신세팅
  Serial.begin(9600);
  BTserial.begin(9600);
  //LCD 세팅
  lcd.begin(16, 2);
  lcd.clear();
  lcd.init();
  lcd.backlight();
  //dht setting
  dht.begin();
}


void loop(){
  humidity = dht.readHumidity();    
  currentTemperature = dht.readTemperature();
  irSensorValue = analogRead(2);

  if(BTserial.available()){
    data = BTserial.read();
  }
  if(!(data == 'y')){
    if(data == 'a'){
      Security = !Security;
      noTone(buzzerPin);
      data = 'y';
    }
    else if(data == 'b'){
      Security = false;
      data = 'y';
    }
    //환풍기 AUTO ON OFF
    else if(data == 'c'){
      ventflag = 1;

      data ='y';
    }
    else if(data == 'd'){
      digitalWrite(VentmotorPin, HIGH);
      ventflag = 0;
      data = 'y';
    }
    else if(data == 'e'){
      analogWrite(VentmotorPin,0);
      ventflag = 0;
      data = 'y';
    }
    else if(data == 'f'){
      openWindow();
      data = 'y';
    }
    else if(data == 'g'){
      closeWindow();
      data = 'y';
    }
    else if(data == 'h'){
      desiredTemperature = BTserial.parseFloat();
      Tempflag = true;
      Serial.write('h');
      data = 'y';
    }
    else if(data =='i'){
      if(lastflag+5000 < millis()){
        Tempflag =false;
        Serial.write(data);
        lastflag = millis();
      }
    }
    else if(data =='j'){
      if(lastflag+5000 < millis()){
        Tempflag =false;
        Serial.write(data);
        lastflag = millis();
      }
    }
    //x를 받으면 현재온습도 앱으로 전송
    else if(data == 'x'){
      
      if(Tempflag == 1){
        BTSerialTemp = updateTemp;
      }
      else{
        BTSerialTemp = currentTemperature;
      }
      BTserial.print(BTSerialTemp);
      BTserial.println("'C");
      BTserial.print(humidity);
      BTserial.println("%");
      data = 'y';
    }
    else{
      Serial.write(data);
      data = 'y' ;
    }
  }

  //두번째 아두이노로 온습도 희망온도 전송 10초마다
  if(exTF < millis()-5000){
    Serial.print('z');
    Serial.print(currentTemperature);
    Serial.print(",");
    Serial.println(desiredTemperature);
    exTF = millis();
  }

  // LCD온습도 출력
  while(Serial.available()){
    Serialdata = Serial.parseFloat();
    if(!(Serialdata == 0)){
      updateTemp = Serialdata;
      Tempflag = 1;
    }
  }

  if(Tempflag == 1){
    LCDTemp = updateTemp;
  }
  else{
    LCDTemp = currentTemperature;
  }

  displayTemperatureAndHumidity();

  // 환풍기제어
  if (ventflag == 1 ) {
    if (humidity > 70) {
      digitalWrite(VentmotorPin, 255);  // 환풍기 DC 모터를 최대 속도로 작동
      ventState= true;
    }
    else {
      digitalWrite(VentmotorPin, 0);  // 환풍기 DC 모터를 정지
      ventState= false;
    }
  }

  //방범
  //창문 제어
  if ( WindowState == WindowClose) {//창문상태 방범장치 상태 and 확인
    if(Security == true){
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
}


// void ISRFunction(){
//   if(BTserial.available()){
//     data = Serial.read();
 
//   }
// }

//LCD와 시리얼 모니터에 현재 온습도 표현 함수
void displayTemperatureAndHumidity(){
  lcd.setCursor(0, 0);//0행 실내온도
  lcd.print("Temp: ");
  lcd.print(LCDTemp);
  // if(Tempflag == true){
  //   lcd.print(LCDTempupdateTemp);
  // }
  // else{
  //   lcd.print(currentTemperature);
  // }
  lcd.print(" 'C ");
  lcd.setCursor(0, 1);   //1행 실내습도
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");
}

void activateAlarm(){
  if(alarmcurrent - actstartTime < 2000){  // 현재 시간 기록 8000 ){
    tone(buzzerPin, 1000);
  }
  else if(alarmcurrent - actstartTime < 4000){
    noTone(buzzerPin); 
  }
  else if(alarmcurrent - actstartTime < 6000){
    tone(buzzerPin, 1000); 
  }
  else if(alarmcurrent - actstartTime < 8000){
    noTone(buzzerPin); 
  }
  else if(alarmcurrent - actstartTime < 1000){
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
  while (millis() - startTimelight < 1000){
    tone(buzzerPin, 200);
    delay(500);
    noTone(buzzerPin);
  }
}

//창문 열기 함수
void openWindow(){
  windowServo.attach(9);
  windowServo.write(0);
  delay(1000); 
  windowServo.detach();
  WindowState= false;
}

//창문 닫기 함수
void closeWindow(){
  windowServo.attach(9);
  windowServo.write(90);  // 닫힌 위치에 해당하는 각도
  delay(1000);
  windowServo.detach();
  WindowState= true;
}