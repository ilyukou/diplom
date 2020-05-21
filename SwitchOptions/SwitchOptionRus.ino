#include <LCD_1602_RUS.h>

LCD_1602_RUS lcd(0x27, 16, 2);

// Энкодер
#define pin_CLK 2
#define pin_DT 4
#define pin_Btn 3

const int pin_Optocoupler = 5; //пин для оптопары начинающей отсчет, при нажатии происходит старт подсчета импульсов
const int pin_Impulse_Counter = 6; //пин для подсчета испульсов, при нажатии происходит подсчет импульсов

unsigned long CurrentTime, LastTime;
enum eEncoderState
{
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

eEncoderState GetEncoderState()
{
  // Считываем состояние энкодера
  eEncoderState Result = eNone;
  CurrentTime = millis();
  if (CurrentTime - LastTime >= 5)
  {
    // Считываем не чаще 1 раза в 5 мс для уменьшения ложных срабатываний
    LastTime = CurrentTime;
    if (digitalRead(pin_Btn) == LOW)
    {
      if (ButtonPrev)
      {
        Result = eButton; // Нажата кнопка
        ButtonPrev = 0;
      }
    }
    else
    {
      ButtonPrev = 1;
      EncoderA = digitalRead(pin_DT);
      EncoderB = digitalRead(pin_CLK);
      if ((!EncoderA) && (EncoderAPrev))
      { // Сигнал A изменился с 1 на 0
        if (EncoderB)
          Result = eRight; // B=1 => энкодер вращается по часовой
        else
          Result = eLeft; // B=0 => энкодер вращается против часовой
      }
      EncoderAPrev = EncoderA; // запомним текущее состояние
    }
  }
  return Result;
}

void setup()
{
  Wire.begin();
  lcd.init();
  lcd.backlight(); // Включаем подсветку дисплея

  // настройка для энкодера
  pinMode(pin_DT, INPUT);
  pinMode(pin_CLK, INPUT);
  pinMode(pin_Btn, INPUT_PULLUP); // Кнопка не подтянута к +5 поэтому задействуем внутренний pull-up резистор
  Serial.begin(9600);
  counter = 0;

  // настройка для линейного перемещения
  pinMode(pin_Optocoupler, INPUT);
  pinMode(pin_Impulse_Counter, INPUT);
  
}

bool isNeedPreView = true;
bool isEncoderButtonPressed = false;

void loop(){

  if (isNeedPreView){
    prtintTitle();
    printOption();
    isNeedPreView = false;
  }

  if (isEncoderButtonPressed){
    goToOption();
    isNeedPreView = true; // указывает что после работы с опцией нужно отрисовать экран заново
  }

  switch (GetEncoderState()){

  case eNone:
    return;
  case eButton: // Нажали кнопку
    isEncoderButtonPressed = true;
    break;
  
  case eLeft:
   // Энкодер вращается влево
    counter--;
    prtintTitle();
    printOption();
    isEncoderButtonPressed = false;
    break;
  
  case eRight:
   // Энкодер вращается вправо
    counter++;
    prtintTitle();
    printOption();
    isEncoderButtonPressed = false;
    break;
  
  }
  delay(100);
}

void encoder(){
  switch (GetEncoderState())
  {
  case eNone:
    isEncoderButtonPressed = false;
    return;

  case eButton: // Нажали кнопку
    isEncoderButtonPressed = true;
    break;
  
  case eLeft:
   // Энкодер вращается влево
    counter++;
    isEncoderButtonPressed = false;
    break;
  
  case eRight:
   // Энкодер вращается вправо
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

  case 0: // ЛИНЕЙНОЕ
    lcd.print(L"1. ЛИНЕЙНОЕ");
    break;

  case 1: // СЕКУНДОМЕР
    lcd.print(L"5. СЕКУНДОМЕР");
    break;

  case 2: // ТАЙМЕР
    lcd.print(L"4. ТАЙМЕР");
    break;

  case 3: // ПЕРИОД
    lcd.print(L"3. ПЕРИОД");
    break;

  case 4: // УГЛОВОЕ
    lcd.print(L"2. УГЛОВОЕ");
      break;    
  }
}

void goToOption(){

  switch (counter){

  case 0: // ЛИНЕЙНОЕ
    linearMovement();
    counter = 0; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 4: // УГЛОВОЕ
    angleMovement();
    counter = 4; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 3: // ПЕРИОД
    oscillation();
    counter = 3; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 1: // СЕКУНДОМЕР
    stopwatch();
    counter = 1; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 2: // ТАЙМЕР
    timer();
    counter = 2; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;
  }
}

void exitFromOption(){
  // две строчки снизу помогают выйти из режима
  encoder();
  delay(500);
}

/***********************************  LINEINOE PEREME and his methods START ***********************************/

void linearMovement(){ // Линейного перемещения

  const byte stepForLinearMovement = 5; //cm
  long timeForStartCounting = 0;
  long distanceTravelTime = 0;

  int needDistance = 0;

  int optocouplerPin = 3;
  int pinImpulse = 2;

  isNeedPreView = true;

  counter = 0;
  int last = counter;

  delay(50);
  do
  {
    if (last != counter || isNeedPreView)
    { 
      counter = counter <= 0 ? 0 : counter;
      last = counter;

      needDistance = last * stepForLinearMovement;
      isNeedPreView = false;

      printLinearMovementDistance(needDistance);
    }

    if (isAnalogButtonPressed(optocouplerPin) && needDistance > 0) {
      timeForStartCounting = micros();
      while (needDistance > 0)
      {
        if (isAnalogButtonPressed(pinImpulse)){
          needDistance--;
          
        }
        distanceTravelTime = micros() - timeForStartCounting;// милилсекунды
        printResultDistanceTravelTime(distanceTravelTime, needDistance);
        delay(50);    
      }
      isNeedPreView = true;
      delay(2500);
    }
    encoder();
  } while (!isEncoderButtonPressed);
}

void printResultDistanceTravelTime(long timeToPrint, int distanceToPrint){
  //FIXME вывод времения прохождения груза
  lcd.setCursor(0, 0);
  lcd.clear();

  
lcd.print(L"ДИСТАНЦИЯ ");
  
  lcd.print(String(distanceToPrint));
  
lcd.print(L" см");

  lcd.setCursor(0, 1);

  lcd.print("  ");

  // example. timeToPrint = 5672000 millis
  long second = timeToPrint / 1000000; // 5
  long millisecond = timeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));
  
  lcd.print(L" сек");
  
}

void printLinearMovementDistance(int distanceToPrint){
  if (distanceToPrint < 0)
  {
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

/***********************************  LINEINOE PEREME and his methods END ***********************************/


/***********************************  ANGLE MOVEMENT and his methods START ***********************************/

void angleMovement()
{ // Углового перемещения

  isNeedPreView = true;
  isEncoderButtonPressed = false;

  int lastLeftBallValues = leftBallValues[0];
  int lastRightBallValues = rightBallValues[0];


  do{
    
    if (isNeedPreView){
      printAngleMovement();
      isNeedPreView = false;
    }

    encoderLeftAngle();
    encoderRightAngle();

    encoder();

        if(lastLeftBallValues != leftBallValues[0] || lastRightBallValues != rightBallValues[0]){
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

  if(isAnalogButtonPressed(leftPlusPin)){
    changeLeftBallArray(1 * step);
  }

  if(isAnalogButtonPressed(leftMinusPin)){
    changeLeftBallArray(-1 * step);
  }
}


// Добавляет в массив новое значение
void changeLeftBallArray(int newLeftValue){

  leftBallValues[0] = leftBallValues[0] + newLeftValue;

  if(leftBallValues[0] < 0){
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

  if(isAnalogButtonPressed(rightPlusPin)){
    changeRightBallArray(1 * step);
  }

  if(isAnalogButtonPressed(rightMinusPin)){
    changeRightBallArray(-1 * step);
  }
}

// Добавляет в массив новое значение
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


/***********************************  ANGLE MOVEMENT and his methods END ***********************************/

/***********************************  OSCILLATION and his methods START ***********************************/


void oscillation(){ // Колебания
  long timeForStartCounting = 0;
  long countOfOscillations = 0;

  int startPin = 3;
  int pinImpulse = 2;

  isNeedPreView = true;

  counter = 0;
  int last = counter;

  delay(50);
  do
  {
    if (last != counter || isNeedPreView)
    { 
      counter = counter <= 0 ? 0 : counter;
      last = counter;

      isNeedPreView = false;
      printOscillation(0, counter);
    }

    if (isAnalogButtonPressed(startPin) && counter > 0) {
      timeForStartCounting = micros();
      counter *= 2;
      while (counter != 0)
      {
        if (isAnalogButtonPressed(pinImpulse)){
          counter--;

        }
        
        if(counter % 2 == 0){
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
  // example. timeToPrint = 5672000 millis
  long second = time / 1000000; // 5
  long millisecond = time - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));
  
  
  lcd.print(L" сек.");
}

/***********************************  OSCILLATION and his methods END ***********************************/

/***********************************  STOP WATCH and his methods START ***********************************/

// нужно взять 2 кнопки (аналоговые) и проверять нажаты ли они

void stopwatch()
{ // Секундомер

  const byte analogPinForStartButtonOfStopWatch = 3; //номер порта для старта
  const byte analogPinForStopButtonOfStopWatch = 2;  //номер порта для стопа

  long timeOfStopWatch = 0;
  long currentTime = 0;

  bool isNeedPreView = true;

  delay(100);
  do
  {
    if (isNeedPreView)
    {
      // выводим стандартвое меню
      printPreViewForStopWatch();
      isNeedPreView = false;
    }
    // нажмите пуск

    if (isAnalogButtonPressed(analogPinForStartButtonOfStopWatch))
    {
      timeOfStopWatch = micros();
      while (!isAnalogButtonPressed(analogPinForStopButtonOfStopWatch))
      {

        delay(100); // обновлять значения времени через 100мс, чтобы экран не дребежал
        currentTime = micros() - timeOfStopWatch;

        printTimeForStopWatch(currentTime);
      }
      printResultTimeForStopWatch(currentTime);

      delay(2500); // 2.5c время показки значения, перед выходов обратно в меню секундомера

      // возвращение значений в исходное положение
      isNeedPreView = true;
    }

    encoder(); // проверяем вдруг кнопка будет нажата, тогда цикл while прекратиться

  } while (!isEncoderButtonPressed);
}

void printTimeForStopWatch(long timeToPrint)
{
  lcd.setCursor(0, 0);
  lcd.clear();
  
  lcd.print(L"ВРЕМЯ ");

  // example. timeToPrint = 5672000 millis
  long second = timeToPrint / 1000000; // 5
  long millisecond = timeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));
  lcd.setCursor(0, 1);
  
  lcd.print(L"     секунд     ");
}

void printPreViewForStopWatch()
{
  lcd.setCursor(0, 0);
  lcd.clear();
  
  lcd.print(L"   ДЛЯ НАЧАЛА   ");
  lcd.setCursor(0, 1);
  lcd.print(L" НАЖМИТЕ  СТАРТ ");
}

void printResultTimeForStopWatch(long timeToPrint){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"   РЕЗУЛЬТАТ:   ");

  // example. timeToPrint = 5672000 millis
  long second = timeToPrint / 1000000; // 5
  long millisecond = timeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.setCursor(0, 1);
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));
  lcd.print(L" сек.");
}

/***********************************  STOP WATCH and his methods END ***********************************/


/***********************************  TIMER and his methods START ***********************************/

void timer()
{ // Таймер

  byte analogPinForStartButtonOfTimer = 3; //номер порта для старта
  double stepForTimer = 0.5;               //секунды
  double setTimeForTimer = 0;
  double currentTime = 0;

  double needTimeForTimer = 0;

  bool isNeedPreView = true;

  counter = 0; //

  int last = counter;
  bool isNeedPrint = true;

  delay(50);
  do
  {
    if (last != counter || isNeedPreView) {

      counter = counter <= 0 ? 0 : counter;
      last = counter;

      setTimeForTimer = last * stepForTimer;

      printPreViewForTimer(setTimeForTimer);

      isNeedPreView = false;
    }
    if (isAnalogButtonPressed(analogPinForStartButtonOfTimer))
    {
      needTimeForTimer = micros() + ( setTimeForTimer * 1000000 );
      while (needTimeForTimer > micros())
      { 
        delay(100); // обновлять значения времени через 100мс, чтобы экран не дребежал
        currentTime = needTimeForTimer - micros();
        printRemaimingTimeForTimer(currentTime);
      }
      printWhenTimeEndsForTimer();
      delay(1000); // показывать уведомление об окончании времени 1с

      // возвращение значений в исходное положение
      counter = 0;
      isNeedPreView = true;
    }

    encoder(); // проверяем вдруг кнопка будет нажата, тогда цикл while прекратиться

  } while (!isEncoderButtonPressed);
}

void printPreViewForTimer(double setTimeToPrint)
{
  lcd.setCursor(0, 0);
  lcd.clear();
  
  lcd.print(L"УСТАНОВИТЕ ВРЕМЯ");
  
  lcd.setCursor(0, 1);
  lcd.print(String(setTimeToPrint));
  lcd.print(L" сек.");
}

void printRemaimingTimeForTimer(double timerTimeToPrint){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"ВРЕМЯ ");

  // example. timeToPrint = 5672000 millis
  long second = timerTimeToPrint / 1000000; // 5
  long millisecond = timerTimeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));
  lcd.setCursor(0, 1);
  lcd.print(L" сек.");
}

void printWhenTimeEndsForTimer(){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"     ФИНИШ!     ");
}

/***********************************  TIMER and his methods END ***********************************/



/***********************************  COMMON methods START ***********************************/

bool isAnalogButtonPressed(byte analogPinNumber)
{
  return analogRead(analogPinNumber) < 300;
}

bool isPinHigh(byte pinNumber){
  return digitalRead(pinNumber) == HIGH;
}

/***********************************  COMMON methods END ***********************************/