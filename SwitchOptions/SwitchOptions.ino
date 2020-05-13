#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte i2[8] = { // Й
    B01110,
    B00000,
    B10011,
    B10011,
    B10101,
    B10101,
    B11001,
    B00000};

byte g[8] = { // Г
    B11111,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B00000};

byte l[8] = { // Л
    B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B00000};
byte i[8] = { // И
    B10001,
    B10011,
    B10011,
    B10101,
    B10101,
    B11001,
    B10001,
    B00000};

byte zh[8] = { // Ж
    B10101,
    B10101,
    B01110,
    B00100,
    B00100,
    B01110,
    B10101,
    B00000};

byte ya[8] = { // Я
    B01111,
    B10001,
    B10001,
    B01111,
    B00101,
    B01001,
    B10001,
    B00000};

byte d[8] = { // Д
    B00011,
    B00101,
    B01001,
    B01001,
    B01001,
    B11111,
    B10001,
    B00000};

byte p[8] = {
    B11111,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B00000};

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
  lcd.begin();
  lcd.backlight(); // Включаем подсветку дисплея
  lcd.setCursor(8, 1);
  lcd.createChar(0, i2);
  lcd.createChar(1, g);
  lcd.createChar(2, l);
  lcd.createChar(3, d);
  lcd.createChar(4, i);
  lcd.createChar(5, ya);
  lcd.createChar(6, zh);
  lcd.createChar(7, p);

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

void loop()
{

  if (isNeedPreView)
  {
    prtintTitle();
    printOption();
    isNeedPreView = false;
  }

  if (isEncoderButtonPressed)
  {
    goToOption();
    isNeedPreView = true; // указывает что после работы с опцией нужно отрисовать экран заново
  }

  switch (GetEncoderState())
  {
  case eNone:
    return;
  case eButton:
  { // Нажали кнопку
    isEncoderButtonPressed = true;
    break;
  }
  case eLeft:
  { // Энкодер вращается влево
    counter--;
    prtintTitle();
    printOption();
    isEncoderButtonPressed = false;
    break;
  }
  case eRight:
  { // Энкодер вращается вправо
    counter++;
    prtintTitle();
    printOption();
    isEncoderButtonPressed = false;
    break;
  }
  }
  delay(100);
}

void encoder()
{
  switch (GetEncoderState())
  {
  case eNone:
    isEncoderButtonPressed = false;
    return;
  case eButton:
  { // Нажали кнопку
    isEncoderButtonPressed = true;
    break;
  }
  case eLeft:
  { // Энкодер вращается влево
    counter++;
    isEncoderButtonPressed = false;
    break;
  }
  case eRight:
  { // Энкодер вращается вправо
    counter--;
    isEncoderButtonPressed = false;
    break;
  }
  }
}
void prtintTitle()
{
  if (counter > 4)
  {
    counter = 0;
  }

  if (counter < 0)
  {
    counter = 4;
  }

  Serial.println(counter);

  lcd.setCursor(0, 0);
  lcd.clear();

  lcd.print("PE");
  lcd.print(char(6));
  lcd.print(char(4));
  lcd.print("M ");
}

void printOption()
{
  lcd.setCursor(0, 1);

  if (counter > 4)
  {
    counter = 0;
  }

  if (counter < 0)
  {
    counter = 4;
  }

  lcd.print(counter + 1);
  lcd.print(". ");

  switch (counter)
  {
  case 3: // СЕКУНДОМЕР
    lcd.print("CEKYH");
    lcd.print(char(3));
    lcd.print("OMEP");
    break;

  case 4: // ТАЙМЕР
    lcd.print("TA");
    lcd.print(char(0));
    lcd.print("MEP");
    break;
  }
}

void goToOption()
{
  switch (counter)
  {
  case 0: // ЛИНЕЙНОЕ
    linearMovement();
    counter = 0; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 1: // УГЛОВОЕ
    angleMovement();
    counter = 1; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 2: // ПЕРИОД
    oscillation();
    counter = 2; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 3: // СЕКУНДОМЕР
    stopwatch();
    counter = 3; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
    exitFromOption();
    break;

  case 4: // ТАЙМЕР
    timer();
    counter = 4; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
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

void linearMovement()
{ // Линейного перемещения

  const byte stepForLinearMovement = 5; //cm
  long timeForStartCounting = 0;
  long distanceTravelTime = 0;

  int needDistance = 0;

  bool isNeedPreView = true;

  counter = 0;
  int last = counter;

  delay(50);
  do
  {
    encoder();
    if (last != counter || isNeedPreView)
    { 
      counter = counter <= 0 ? 0 : counter;
      last = counter;

      needDistance = last * stepForLinearMovement;
      isNeedPreView = false;

      printLinearMovementDistance(needDistance);
    }
    delay(10);

    if (isPinHigh(pin_Optocoupler) && needDistance > 0) {
      timeForStartCounting = millis();
      while (needDistance > 0)
      {
        if (isPinHigh(pin_Impulse_Counter)){
          needDistance--;
        }
      }
      distanceTravelTime = millis() - timeForStartCounting;// милилсекунды
      printResultDistanceTravelTime(distanceTravelTime);
      isNeedPreView = true;
    }

  } while (!isEncoderButtonPressed);
}

void printResultDistanceTravelTime(long timeToPrint){
  //FIXME вывод времения прохождения груза
}

void printLinearMovementDistance(int distanceToPrint){
  //FIXME вывоз задаваемого расстояния в см
}

void printlinearMovement()
{
  if (counter < 0)
  {
    counter = 0;
  }

  lcd.setCursor(0, 0);
  lcd.clear();

  lcd.print("PACCTO");
  lcd.print(char(5));
  lcd.print("H");
  lcd.print(char(4));
  lcd.print("E");
  lcd.print(" ");
  lcd.print(counter);

  lcd.setCursor(0, 1);
  lcd.print("BPEM");
  lcd.print(char(5));
  lcd.print(" ");
  lcd.print(0);

  lcd.print(" cek.");
}

void angleMovement()
{ // Углового перемещения
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  delay(50);
  do
  {
    encoder();
    if (last != counter || isNeedPrint)
    {
      printAngleMovement();

      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  } while (!isEncoderButtonPressed);
  isNeedPreView = true;
  isEncoderButtonPressed = false;
}

void printAngleMovement()
{
  if (counter < 0)
  {
    counter = 0;
  }

  lcd.setCursor(0, 0);
  lcd.clear();

  lcd.print("Y");
  lcd.print(char(1));
  lcd.print("O");
  lcd.print(char(2));
  lcd.print(" ");
  lcd.print(counter);

  lcd.setCursor(0, 1);
  lcd.print("BPEM");
  lcd.print(char(5));
  lcd.print(" ");
  lcd.print(0);

  lcd.print(" cek.");
}

void oscillation()
{ // Колебания
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  delay(50);
  do
  {
    encoder();
    if (last != counter || isNeedPrint)
    {
      printOscillation();

      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  } while (!isEncoderButtonPressed);
  isNeedPreView = true;
  isEncoderButtonPressed = false;
}

void printOscillation()
{
  if (counter < 0)
  {
    counter = 0;
  }

  lcd.setCursor(0, 0);
  lcd.clear();

  lcd.print("KO");
  lcd.print(char(2));
  lcd.print("E");
  lcd.print("b");
  lcd.print("AH");
  lcd.print(char(4));
  lcd.print(char(5));
  lcd.print(" ");
  lcd.print(counter);

  lcd.setCursor(0, 1);
  lcd.print("BPEM");
  lcd.print(char(5));
  lcd.print(" ");
  lcd.print(0);

  lcd.print(" cek.");
}

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
  lcd.print("TIME ");

  // example. timeToPrint = 5672000 millis
  long second = timeToPrint / 1000000; // 5
  long millisecond = timeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.print(second);
  lcd.print(".");
  lcd.print(millisecond);
  lcd.setCursor(0, 1);
  lcd.print("    seconds.    ");
}

void printPreViewForStopWatch()
{
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("  PLEASE PRESS  ");
  lcd.setCursor(0, 1);
  lcd.print("     START.     ");
}

void printResultTimeForStopWatch(long timeToPrint){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("RESULT ");

  // example. timeToPrint = 5672000 millis
  long second = timeToPrint / 1000000; // 5
  long millisecond = timeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.print(second);
  lcd.print(".");
  lcd.print(millisecond);
  lcd.setCursor(0, 1);
  lcd.print("    seconds.    ");
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
  lcd.print("SET TIME ");
  lcd.print(setTimeToPrint);
  lcd.setCursor(0, 1);
  lcd.print("    seconds.    ");
}

void printRemaimingTimeForTimer(double timerTimeToPrint){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("TIME ");

  // example. timeToPrint = 5672000 millis
  long second = timerTimeToPrint / 1000000; // 5
  long millisecond = timerTimeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  // 5.67
  lcd.print(second);
  lcd.print(".");
  lcd.print(millisecond);
  lcd.setCursor(0, 1);
  lcd.print("    seconds.    ");
}

void printWhenTimeEndsForTimer(){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("      END.      ");
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