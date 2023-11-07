#define POTENTIOMETER_PIN A0
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //ADMUX |= (0<<REFS1) | (1<< REFS0) ;
  //ADMUX |= (0<<ADLAR);
  //ADMUX |= (0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);
  //ADCSRA |= (1<<ADEN);
  //ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

void loop() {
  // put your main code here, to run repeatedly:
  //ADCSRA |= (1<< ADSC);
  //while(!(ADCSRA & (1<<ADIF)));
  uint16_t avalue = analogRead(POTENTIOMETER_PIN);
  Serial.println(avalue);
  delay(100);
}
