/*************************************************************
Motor Shield Stepper Demo
by Randy Sarafan

For more information see:
https://www.instructables.com/id/Arduino-Motor-Shield-Tutorial/

*************************************************************/
#define Cds_Photoresistor_PIN A0

#include <Stepper.h>
#include <SoftwareSerial.h>
#define STEPS 2048

Stepper stepper(STEPS, 8, 10, 9, 11);
int curtain_flag = 0;

void setup() {
  stepper.setSpeed(12);
  Serial.begin(9600);
}


void Curtainup() {
  Serial.print("curtain_flag : ");
  Serial.println(curtain_flag);

  if (curtain_flag == 0){
    stepper.step(STEPS);
    stepper.step(STEPS);
    delay(1000);
    curtain_flag = 1;
  }
  Serial.print("after curtain_flag : ");
  Serial.println(curtain_flag);
}

void Curtaindown() {
  Serial.print("curtain_flag : ");
  Serial.println(curtain_flag);

  if (curtain_flag == 1){
    // 역방향으로 3초 동안 회전
    stepper.step(-STEPS);
    stepper.step(-STEPS);
    stepper.step(-STEPS);
    stepper.step(-STEPS);
    stepper.step(-STEPS);
    stepper.step(-STEPS);
    delay(1000);
    curtain_flag = 0;
  }
  Serial.print("after curtain_flag : ");
  Serial.println(curtain_flag);

}

void loop() {

  uint16_t aValue = analogRead(Cds_Photoresistor_PIN);

  if (aValue < 400){
    Curtainup();
  }
  else {
    Curtaindown();
  }

}
