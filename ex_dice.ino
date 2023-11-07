// C++ code
//

#define PUSH_BUTTON_PORT 7
#define NUM_LED 2

const uint8_t LED[NUM_LED] = {8, 9};
void setup(){
  pinMode(PUSH_BUTTON_PORT,INPUT_PULLUP);
  for(uint8_t i=0; i < NUM_LED ; i++){
    pinMode(LED[i],OUTPUT);
  }
}

void loop(){
  //randomled on
 uint16_t value = digitalRead(PUSH_BUTTON_PORT);
  if(value !=0){
    uint8_t dice = random(NUM_LED);
    digitalWrite(LED[dice],HIGH);
    delay(500);
    
  }
  //초기화
  else{
    for(uint8_t i=0; i<NUM_LED;i++){
      digitalWrite(LED[i],LOW);
    }
    
  }
}