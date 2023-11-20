#include <Stepper.h>

#define STEPPERREV 2048
#define Cds_Photoresistor_PIN A0

const uint8_t pin_in1 = 8;
const uint8_t pin_in2 = 9;
const uint8_t pin_in3 = 10;
const uint8_t pin_in4 = 11;

uint8_t rpm = 8;

Stepper myStepper(STEPPERREV, pin_in1, pin_in2);

void setup() {
  myStepper.setSpeed(rpm);
  Serial.begin(9600);
}

void Curtain() {
  uint16_t aValue = analogRead(Cds_Photoresistor_PIN);
  Serial.println(aValue);
  delay(100);

  if (aValue > 512) {
    // 정방향으로 3초 동안 회전
    myStepper.step(STEPPERREV);
    delay(3000);
  } else {
    // 역방향으로 3초 동안 회전
    myStepper.step(-STEPPERREV);
    delay(3000);
  }
}

void loop() {
  Curtain();
}