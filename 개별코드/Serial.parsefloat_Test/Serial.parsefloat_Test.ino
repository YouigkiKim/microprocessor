

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  String state = floatTostring(11.00)+","+floatTostring(12.21)+","+floatTostring(14.59);
  float state[3];
  if(Serial.available()){
    state[0] = Serial.parseFloat();
    state[1] = Serial.parseFloat();
    state[2] = Serial.parseFloat();
    Serial.print(state[0]);
    Serial.print(state[1]);
    Serial.println(state[2]);
  }
}
