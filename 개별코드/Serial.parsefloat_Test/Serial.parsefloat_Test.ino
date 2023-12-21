

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  float state[3];
  if(Serial.available()){
    if(Serial.find('.')){
      state[0] = Serial.parseFloat();
      state[1] = Serial.parseFloat();
      state[2] = Serial.parseFloat();
      Serial.println(state[0]);
      Serial.println(state[1]);
      Serial.println(state[2]);
    }
  }
}
