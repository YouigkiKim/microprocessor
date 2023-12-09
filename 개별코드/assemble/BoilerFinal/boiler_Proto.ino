#include <SoftwareSerial.h>
#include <DHT.h>
#define boilerLEDPin 6
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//플래그변수
bool tempflag= 0;
bool boilflag=0;
//온도습도변수

float humidity;
float currentTemp;
float updateTemp;
float Timediff = 0;
float startTempdiff ;
unsigned long boilerstart;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(boilerLEDPin,OUTPUT);
  dht.begin();
}

void loop() {

  // put your main code here, to run repeatedly:
  humidity = dht.readHumidity();
  
  float desiredTemp = 90;
  Serial.print("currentTemp: ");
  Serial.println(currentTemp);
  tempflag = 1;

  //온도 자동조절 확인
  if(tempflag == 1){

    //boilerflag로 보일러 작동시작유무 확인
    if(boilflag == 0){
      boilflag = 1;
      analogWrite(boilerLEDPin, 255);
      currentTemp = dht.readTemperature();
      boilerstart = millis();
      startTempdiff = desiredTemp - currentTemp;
    }
    
    //온도차에 따라 온도감소량과 밝기차이를 구현하는게 목표
    else{
      if(desiredTemp > currentTemp){
        updateTemp = setBoilerTemperature(desiredTemp,currentTemp, Timediff);
        currentTemp = updateTemp;
        //setBoilerBrightness(desiredTemp, currentTemp, boilerstart);
      }
      Timediff = millis();
    }
    //보일러 밝기변화 반영 위의 엘스문 안에? 아니면 따로?
  }
  else{
    tempflag = 0;
  }
  delay(1000);
}

float setBoilerTemperature(float desiredTemp, float currentTemperature, float StartTime){
  float diffTemp = desiredTemp - currentTemperature;
  float updatetemp = currentTemperature + diffTemp/20*(millis()-StartTime)/1000;
  float ratio = diffTemp/startTempdiff;
  Serial.print("ratio: ");
  Serial.println(ratio);
  int k = map(ratio*100, 0, 100, 1, 255);
  analogWrite(boilerLEDPin, k);
  Serial.print("K: ");
  Serial.println(k);
  // if(ratio >0.8)
  //   analogWrite(boilerLEDPin, 255);
  // }
  // else if(ratio > 0.5){
  //   analogWrite(boilerLEDPin, 140);
  // }
  // else if(ratio > 0.3){
  //   analogWrite(boilerLEDPin, 70);
  // }
  // else if(ratio > 0.05){
  //   analogWrite(boilerLEDPin, 32);
  // }
  // else{
  //   analogWrite(boilerLEDPin, 0);
  // }
  Serial.print("updateTemp: ");
  Serial.println(updatetemp);
  return updatetemp;

}

// void boilerstart(float updateTemp){
//   float bright = map(abs(updateTemp), 0, 8000, 255, 1);
//   analogWrite(boilerLEDPin, bright);
// }
// void setBoilerBrightness(float desiredTemp, float currentTemp, unsigned long boilerstart) {
//   float diffTemp = desiredTemp - currentTemp;
//   Serial.println("check setboiler");
//   // fadeSpeed 값을 낮추어 밝기 변화를 느리게 설정
//   float fadeSpeed = map(abs(diffTemp), 0, 60, 3, 1);
//   float update = fadeSpeed *(millis() - boilerstart)/1000;
//   Serial.println(millis());
//   Serial.println(boilerstart);
//   float bright = map(abs(update), 0, 80, 255, 32);
//   Serial.println(bright);
//   analogWrite(boilerLEDPin, bright);
// }