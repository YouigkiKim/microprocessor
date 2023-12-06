int windowServoPin = 7;           // 창문 여닫이 서보모터가 연결된 핀
int buzzerPin = 8;                 // 피에조 부저가 연결된 핀

//창문상태지정 True=닫힘 False=열림
bool WindowClose = true;;
bool WindowAuto = true;
bool WindowState = WindowClose;           //창문상태지정 True=닫힘 False=열림
bool WindowState = WindowClose; 
//창문상태지정 True=닫힘 False=열림
bool WindowClose = true;;
bool WindowAuto = true;
bool WindowState = WindowClose;           //창문상태지정 True=닫힘 False=열림
bool WindowState = WindowClose; 
//블루투스 데이터
char BTdata ;
unsigned long startTime;
unsigned long currentTime;

//bt.read()의 값이 int값으로 반환되기 때문에 통신을 통해 제어하는 가장 적합한 방법이 정수형을 통한 제어라고 생각됨
//>>앱 인벤터를 통해 데이터를 보낼경우 텍스트형식이 가장 수월하게 통신됨 그러므로 텍스트통신으로 장치제어

Servo windowServo;

void setup() {
@@ -27,33 +36,74 @@ void setup() {
}

void loop() {

//irSensorValu 업데이트
  int irSensorValue = analogRead(2);
//블루투스를 통해 데이터가 들어온 경우
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
    
//리모컨으로 security작동 시 동작하는 코드
  if(BTdata == 'security'){

    while(BT.available());

    BTdata = BT.read();

    if(BTdata == 'on'){
      Security = SecurityOn;
    }

    else if(BTdata == 'off'){
      Security = SecurityOff;
    }
  }

//방범장치 작동코드 - 창문이 닫혀있고, 방범장치가 켜진 상태에서 물체가 감지되면 lightAlarm,
// 물체가 5초 이상 감지되면 10초간 알람울림
  if ( WindowState == Security) {//창문상태 방범장치 상태 and 확인
    if(irSensorValue < 200 ){ // 창문이 닫혀있고 물체가 감지되면
      activateAlarm();  // 알람작동

      if(StartTime == 0){
        startTime = millis();
        bool SecureFlag = true;
      }

      unsigned long currentTime = millis();

      if(SecureFlag == 1){
        if(currentTime-5 > startTime){
          activateAlarm();  // 알람작동
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
    }

  }

  if(0){
   if(0){
  if(BTdata = 'window'){

    while(!BT.available());
    BTdata = BT.read();

    
   if('open'){
      penWindow();
    }
    else{
      closeWindow();
    }
    
  }
}

void openWindow() {
  WindowState = false; //창문상태 열림]
}

//방범장치 함수
//물체 1번 감지시 lightAlarm작동
void lightAlarm(){
  startTime = millis();
  tone(buzzerPin, 1000);
  delay(500);
  noTone(buzzerPin);
}

//방범장치 함수 - 5초 이상 물체 감지시 10초간 0.5초동안 울렸다 멈췄다 반복
void activateAlarm() {            
  unsigned long startTime = millis();  // 현재 시간 기록
  while (millis() - startTime < 10000) {  // 10초 동안 반복
  unsigned long AlarmStart = millis();  // 현재 시간 기록
  while (millis() - AlarmStart < 10000) {  // 10초 동안 반복
    tone(buzzerPin, 1000);
    delay(500);  // 0.5초 대기
    noTone(buzzerPin);  // 피에조 부저 끄기
    delay(500);  // 0.5초 대기
  }
  AlarmStart = 0;
}