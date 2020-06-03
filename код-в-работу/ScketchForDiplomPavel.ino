#include <LCD_1602_RUS.h>
LCD_1602_RUS lcd(0x27, 16, 2);
#define pin_CLK 4
#define pin_DT 2
#define pin_Btn 9
const int pin_Optocoupler = 5;
const int pin_Impulse_Counter = 6;
const int pin_Sound_Signal = 3;
bool isNeedPreView = true;
bool isEncoderButtonPressed = false;
unsigned long CurrentTime, LastTime;
enum eEncoderState {
  eNone,
  eLeft,
  eRight,
  eButton
};
uint8_t EncoderA, EncoderB, EncoderAPrev;
int8_t counter = 0;
bool ButtonPrev;
int leftBallValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int rightBallValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
long leftBallTimeValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
long rightBallTimeValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
eEncoderState GetEncoderState() {
  eEncoderState Result = eNone;
  CurrentTime = millis();
  if (CurrentTime - LastTime >= 5) {
    LastTime = CurrentTime;
    if (digitalRead(pin_Btn) == LOW) {
      if (ButtonPrev) {
        Result = eButton; // Нажата кнопка
        ButtonPrev = 0;
      }
    }
    else {
      ButtonPrev = 1;
      EncoderA = digitalRead(pin_DT);
      EncoderB = digitalRead(pin_CLK);
      if ((!EncoderA) && (EncoderAPrev)) { 
        if (EncoderB)
          Result = eRight;
        else
          Result = eLeft;
      }
      EncoderAPrev = EncoderA;
    }
  }
  return Result;
}
void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(pin_DT, INPUT);
  pinMode(pin_CLK, INPUT);
  pinMode(pin_Btn, INPUT_PULLUP);
  pinMode(pin_Sound_Signal, OUTPUT); 
  Serial.begin(9600);
  counter = 0;
  pinMode(pin_Optocoupler, INPUT);
  pinMode(pin_Impulse_Counter, INPUT);
}

void loop(){
  if (isNeedPreView) {
    prtintTitle();
    printOption();
    isNeedPreView = false;
  }
  if (isEncoderButtonPressed) {
    goToOption();
    isNeedPreView = true;
  }
  switch (GetEncoderState()){
  case eNone:
    return;
  case eButton:
    isEncoderButtonPressed = true;
    break;
  case eLeft:
    counter--;
    prtintTitle();
    printOption();
    isEncoderButtonPressed = false;
    break;
  case eRight:
    counter++;
    prtintTitle();
    printOption();
    isEncoderButtonPressed = false;
    break;
  }
  delay(100);
}
void encoder(){
  switch (GetEncoderState()) {
  case eNone:
    isEncoderButtonPressed = false;
    return;
  case eButton:
    isEncoderButtonPressed = true;
    break;
  case eLeft:
    counter++;
    isEncoderButtonPressed = false;
    break;
  case eRight:
    counter--;
    isEncoderButtonPressed = false;
    break;
  }
}
void prtintTitle(){
  if (counter > 4){
    counter = 0;
  }
  if (counter < 0){
    counter = 4;
  }
  Serial.println(counter);
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"PEЖИМ");
}
void printOption(){
  lcd.setCursor(0, 1);
  if (counter > 4){
    counter = 0;
  }
  if (counter < 0){
    counter = 4;
  }
  switch (counter){
  case 0:
    lcd.print(L"1. ЛИНЕЙНОЕ");
    break;
  case 1:
    lcd.print(L"2. ПЕРИОД");
    break;
  case 2:
    lcd.print(L"3. УГЛОВОЕ");
    break;    
  }
}
void goToOption(){
  switch (counter){
  case 1:
    linearMovement();
    counter = 0;
    exitFromOption();
    break;
  case 2:
    angleMovement();
    counter = 4;
    exitFromOption();
    break;
  case 3:
    oscillation();
    counter = 3;
    exitFromOption();
    break;
  }
}
void exitFromOption(){
  encoder();
  delay(500);
}
void linearMovement(){
  const byte stepForLinearMovement = 5; //cm
  long timeForStartCounting = 0;
  long distanceTravelTime = 0;
  int needDistance = 0;
  int pinDistanceControlStart = 2 ;
  int pinDistanceControlStop = 3 ;
  isNeedPreView = true;
  counter = 0;
  int last = counter;
  delay(50);
  do { 
    if (last != counter || isNeedPreView) { 
      counter = counter <= 0 ? 0 : counter;
      last = counter;
      needDistance = last * stepForLinearMovement;
      isNeedPreView = false;
      printLinearMovementDistance(needDistance);
    }
    if (isAnalogButtonPressed(pinDistanceControlStart) && needDistance > 0) {
        while (true) {
            if (isPinHigh(pin_Optocoupler)) {
                distanceTravelTime = startLinearDistanceCalculation(needDistance);
                printResultDistanceTravelTime(distanceTravelTime, needDistance);
                delay(2000);
                break;
            }
        }
    }
    encoder();
  } while (!isEncoderButtonPressed);
}
void printResultDistanceTravelTime(long timeToPrint, int distanceToPrint){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"ДИСТАНЦИЯ ");
  lcd.print(String(distanceToPrint));
  lcd.print(L" см");
  lcd.setCursor(0, 1);
  lcd.print("  ");
  long second = timeToPrint / 1000000;
  long millisecond = timeToPrint - second * 1000000;
  millisecond = millisecond / 100;
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));
  lcd.print(L" сек"); 
}
void printLinearMovementDistance(int distanceToPrint){
  if (distanceToPrint < 0) {
    distanceToPrint = 0;
  }
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"РАССТОЯНИЕ");
  lcd.setCursor(0, 1);
  lcd.print("    ");
  lcd.print(String(distanceToPrint));
  lcd.print(L" см");
}
long startLinearDistanceCalculation(int needLinerDistance) {
  long timeForStartCounting = 0;
  long distanceTravelTime = 0;
  while (needLinerDistance > 0) {
    timeForStartCounting = micros();       
    if (isPinHigh(pin_Impulse_Counter)) {
        needLinerDistance-- ; 
        distanceTravelTime = micros() - timeForStartCounting;
        printResultDistanceTravelTime(distanceTravelTime, needDistance);
    }
  }
  return distanceTravelTime;
}
void angleMovement() {
  isNeedPreView = true;
  isEncoderButtonPressed = false;
  int lastLeftBallValues = leftBallValues[0];
  int lastRightBallValues = rightBallValues[0];
  do {
    if (isNeedPreView) {
      printAngleMovement();
      isNeedPreView = false;
    }
    encoderLeftAngle();
    encoderRightAngle();
    encoder();
        if (lastLeftBallValues != leftBallValues[0] || lastRightBallValues != rightBallValues[0]) {
            printInfo(leftBallValues[0], rightBallValues[0]);
            lastLeftBallValues = leftBallValues[0];
            lastRightBallValues = rightBallValues[0];
        }
    delay(100);
  } while (!isEncoderButtonPressed);
  isNeedPreView = true;
}
void encoderLeftAngle(){
  int leftPlusPin = 3;
  int leftMinusPin = 2;
  int step = 1;
  if(isAnalogButtonPressed(leftPlusPin)) {
    changeLeftBallArray(1 * step);
  }
  if(isAnalogButtonPressed(leftMinusPin)) {
    changeLeftBallArray(-1 * step);
  }
}
void changeLeftBallArray(int newLeftValue){
  leftBallValues[0] = leftBallValues[0] + newLeftValue;
  if(leftBallValues[0] < 0) {
    leftBallValues[0] = 0;
  }
  leftBallValues[1] = leftBallValues[0];
  leftBallValues[2] = leftBallValues[1];
  leftBallValues[3] = leftBallValues[2];
  leftBallValues[4] = leftBallValues[3];
  leftBallValues[5] = leftBallValues[4];
  leftBallValues[6] = leftBallValues[5];
  leftBallValues[7] = leftBallValues[6];
  leftBallValues[8] = leftBallValues[7];
  leftBallValues[9] = leftBallValues[8];
}
void encoderRightAngle(){
  int rightPlusPin = 1;
  int rightMinusPin = 0;
  int step = 1;
  if(isAnalogButtonPressed(rightPlusPin)) {
    changeRightBallArray(1 * step);
  }
  if(isAnalogButtonPressed(rightMinusPin)) {
    changeRightBallArray(-1 * step);
  }
}
void changeRightBallArray(int newRightValue){
  rightBallValues[0] = rightBallValues[0] + newRightValue;
  if(rightBallValues[0] < 0){
    rightBallValues[0] = 0;
  }
  rightBallValues[1] = rightBallValues[0];
  rightBallValues[2] = rightBallValues[1];
  rightBallValues[3] = rightBallValues[2];
  rightBallValues[4] = rightBallValues[3];
  rightBallValues[5] = rightBallValues[4];
  rightBallValues[6] = rightBallValues[5];
  rightBallValues[7] = rightBallValues[6];
  rightBallValues[8] = rightBallValues[7];
  rightBallValues[9] = rightBallValues[8];
}
void printAngleMovement(){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"ОТКЛОНИТЕ  ШАРЫ");
}
void printInfo(long left, long right){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(L"ЛЕВЫЙ УГОЛ ");
  lcd.print(String(left));
  lcd.setCursor(0, 1);
  lcd.print(L"ПРАВЫЙ УГОЛ ");
  lcd.print(String(right));
}
void oscillation() {
  long timeForStartCounting = 0;
  long countOfOscillations = 0;
  int startPin = 3;
  int pinImpulse = 2;
  isNeedPreView = true;
  counter = 0;
  int last = counter;
  delay(50);
  do {
    if (last != counter || isNeedPreView) { 
      counter = counter <= 0 ? 0 : counter;
      last = counter;
      isNeedPreView = false;
      printOscillation(0, counter);
    }
    if (isAnalogButtonPressed(startPin) && counter > 0) {
      timeForStartCounting = micros();
      counter *= 2;
      while (counter != 0) {
        if (isAnalogButtonPressed(pinImpulse)) {
          counter--;
        }
        if(counter % 2 == 0) {
          printOscillation(micros() - timeForStartCounting, counter/2);
        } else{
          printOscillation(micros() - timeForStartCounting, (counter + 1)/2);
        } 
        delay(100);    
      }
      isNeedPreView = true;
      delay(2500);
    }
    encoder();
  } while (!isEncoderButtonPressed);
}
void printOscillation(long time, int numberOfOscillation){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"КОЛЕБАНИЯ ");
  lcd.print(" ");
  lcd.print(String(numberOfOscillation));
  lcd.setCursor(0, 1);
  long second = time / 1000000;
  long millisecond = time - second * 1000000;
  millisecond = millisecond / 100;
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));  
  lcd.print(L" сек.");
}
bool isAnalogButtonPressed(byte analogPinNumber) {
  return analogRead(analogPinNumber) < 300;
}
bool isPinHigh(byte pinNumber) {
  return digitalRead(pinNumber) == HIGH;
}
void soundSignal(int numbersOfSound ,int timeBetweenSonds) {
  const int valueOfSignal = 100;
  for (int j = 0; j < numbersOfSound; j++){
    analogWrite(pin_Sound_Signal, 100);
    delay(timeBetweenSonds);
    analogWrite(pin_Sound_Signal, 0);
    delay(timeBetweenSonds);
  }
}