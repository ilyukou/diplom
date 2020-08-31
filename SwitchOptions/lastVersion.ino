#define _LCD_TYPE 1
#include <LCD_1602_RUS_ALL.h>

// Инициализация портов ввода-вывода

LCD_1602_RUS <LiquidCrystal_I2C> lcd(0x27, 16, 2);

// Цифровые порты
const int digitalPin_2_DT = 2;
const int digitalPin_3_soundSignal = 3;
// !!! Изменил с 4 на 6, поскольку 4 пин плохо работает
const int digitalPin_4_CLK = 4;
const int digitalPin_5_Optocoupler = 5;
const int digitalPin_6 = 6;
const int digitalPin_7 = 7;
const int digitalPin_8_impulseCounter = 8;
const int digitalPin_9_BTN = 9;
const int digitalPin_10_relay = 10;
const int digitalPin_11 = 11;
const int digitalPin_12 = 12;
const int digitalPin_13 = 13;

// Аналоговые порты
const int analogPin_0_startMeasurementButton = 0; // Измерение пуск
const int analogPin_1_stopMeasurementButton = 1; // Измерение стоп
const int analogPin_2_startRemoteControlButton = 2; // ДУ пуск
const int analogPin_3_stopRemoteControlButton = 3; // ДУ стоп

const int analogPin_6 = 6; // внешнее ДУ
const int analogPin_7 = 7; // внешнее ДУ

// Инициализация глобальных переменных
int8_t counter = 0;
const double step =  0.0828; // cm

// Функции энкодера
unsigned long CurrentTime, LastTime;
enum eEncoderState{
  eNone,
  eLeft,
  eRight,
  eButton
};

uint8_t EncoderA, EncoderB, EncoderAPrev;
bool ButtonPrev;

int leftBallValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int rightBallValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

long leftBallTimeValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

long rightBallTimeValues[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool isNeedPreView = true;
bool isEncoderButtonPressed = false;

eEncoderState GetEncoderState(){

    // Считываем состояние энкодера
    eEncoderState Result = eNone;
    CurrentTime = millis();

    if (CurrentTime - LastTime >= 5){

        // Считываем не чаще 1 раза в 5 мс для уменьшения ложных срабатываний
        LastTime = CurrentTime;

        if (digitalRead(digitalPin_9_BTN) == LOW){
            if (ButtonPrev){
                Result = eButton; // Нажата кнопка
                ButtonPrev = 0;
            }

        } else {

            ButtonPrev = 1;
            EncoderA = digitalRead(digitalPin_2_DT);
            EncoderB = digitalRead(digitalPin_4_CLK);

            if ((!EncoderA) && (EncoderAPrev)){ // Сигнал A изменился с 1 на 0
                if (EncoderB)
                Result = eLeft; // B=1 => энкодер вращается по часовой
                else
                Result = eRight; // B=0 => энкодер вращается против часовой
            }

            EncoderAPrev = EncoderA; // запомним текущее состояние
        }
    }
    return Result;
}

void setup(){

    lcd.init(); // Включение экрана
    lcd.backlight(); // Включаем подсветку дисплея


    pinMode(digitalPin_2_DT, INPUT); // Настройка энкодера

    pinMode(digitalPin_3_soundSignal, OUTPUT); // Настройка звукового излучателя

    pinMode(digitalPin_4_CLK, INPUT); // Настройка энкодера

    pinMode(digitalPin_5_Optocoupler, INPUT); // Настройка оптопары

    //pinMode(digitalPin_6, INPUT);

    //pinMode(digitalPin_7, INPUT);

    pinMode(digitalPin_8_impulseCounter, INPUT); // Настройка датчика подсчёта импульсов

    pinMode(digitalPin_9_BTN, INPUT_PULLUP); // Настройка энкодера. Кнопка не подтянута к +5 поэтому задействуем внутренний pull-up резистор

    pinMode(digitalPin_10_relay, OUTPUT); // Настройка реле

    isEncoderButtonPressed = false;
    Serial.begin(9600);
    counter = 0;
}

void loop(){
    // Для единоразового обновление экрана при включении или выхода с режима
    if (isNeedPreView){
        prtintTitle();
        printOption();
        isNeedPreView = false;
    }

    // Считывание состояние кнопок отвечающих за реле
    //checkStatusRealyButton_andSetNewStatusToRealy();
    
    // Переход в режим
    if (isEncoderButtonPressed){
        isEncoderButtonPressed = false;
        Serial.println(" в переходе ");
        goToOption();
        isNeedPreView = true; // указывает что после работы в режиме нужно отрисовать экран заново
    }
    // работа с энкодером, а именно чтение состояния энкодера + вывод необходимого режима работы
    switch (GetEncoderState()){
        case eNone:
            return;
        case eButton: // Нажали кнопку
            isEncoderButtonPressed = true;
            break;

        case eLeft:
            // Энкодер вращается влево
            Serial.println(" Энкодер вращается влево ");
            counter--;
            prtintTitle();
            printOption();
            isEncoderButtonPressed = false;
            break;

        case eRight:
            // Энкодер вращается вправо
            Serial.println(" Энкодер вращается вправо ");
            counter++;
            prtintTitle();
            printOption();
            isEncoderButtonPressed = false;
            break;
        }
    delay(200);
}

void prtintTitle(){

  lcd.setCursor(0, 0);
  lcd.clear();

  lcd.print(L"PEЖИМ");
}

void printOption(){

  lcd.setCursor(0, 1);
  
  if (counter > 5){
    counter = 0;
  } else if (counter < 0) {
    counter = 5;
  }


  Serial.println(counter);
  switch (counter){
    case 0: // ЛИНЕЙНОЕ 1
        lcd.print(L"1. ЛИНЕЙНОЕ МАГН");
        break;

    case 1: // ЛИНЕЙНОЕ 2 
        lcd.print(L"2. ЛИНЕЙНОЕ ОПТП");
        break;

    case 2: // УГЛОВОЕ
        lcd.print(L"3. УГЛОВОЕ");
        break;    

    case 3: // ПЕРИОД
        lcd.print(L"4. ПЕРИОД");
        break;

    case 4: // СЕКУНДОМЕР
        lcd.print(L"5. СЕКУНДОМЕР");
        break;

    case 5: // ТАЙМЕР
        lcd.print(L"6. ТАЙМЕР");
        break;
  }
}

void goToOption(){

    switch (counter){

        case 0: // ЛИНЕЙНОЕ
            linearMovement(0);
            counter = 0; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
            exitFromOption();
            break;

        case 1: // ЛИНЕЙНОЕ
            linearMovement(1);
            counter = 1; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
            exitFromOption();
            break;

        case 2: // УГЛОВОЕ
            //angleMovement();
            counter = 2; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
            exitFromOption();
            break;

        case 3: // ПЕРИОД
            // функция вроде бы должна работать, но там нужно перебить порты 
            // которые используются с аналоговых на цифру. Но большой вопрос стоит ли это делать
            // поскольку пока что период никто не будет считать 
            // Поэтому делай наверное сразу функции для линейного перемещения!!!
            oscillation();
            counter = 3; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
            exitFromOption();
            break;

        case 4: // СЕКУНДОМЕР
            stopwatch();
            counter = 4; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
            exitFromOption();
            break;

        case 5: // ТАЙМЕР
            timer();
            counter = 5; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
            exitFromOption();
            break;
  }
}

void exitFromOption(){

  // две строчки снизу помогают выйти из режима
  encoder();
  delay(500);
}

/*********************************** НАЧАЛО МЕТОДОВ ЛИНЕЙН ПЕРЕМЕЩ ***********************************/

void linearMovement(int typeMode){ // Линейного перемещения

    double needDistance = 0;

    double stepForLinearMovement = 1; // cm

    isNeedPreView = true;

    counter = 0;
    int last = counter;

    delay(50);
    do {
        
        // Считывание состояние кнопок отвечающих за реле
        checkStatusRealyButton_andSetNewStatusToRealy();

        // Задача расстояния
        if (last != counter || isNeedPreView){ 
            counter = counter <= 0 ? 0 : counter;
            last = counter;

            needDistance = last * stepForLinearMovement;
            isNeedPreView = false;

            printLinearMovementDistance(needDistance);
        }

        Serial.println(digitalRead(digitalPin_5_Optocoupler));

        if(needDistance > 0 ){
            // Старт по кнопке отсчёта движения
            if (typeMode == 0 && isAnalogButtonPressed(analogPin_0_startMeasurementButton)){
                printLinearMovementRun();
                countingLinearMovement(needDistance);
            }

            //  Старт по оптопаре отсчёта движения
            if (typeMode == 1 && digitalRead(digitalPin_5_Optocoupler) == LOW){
                printLinearMovementRun();
                countingLinearMovement(needDistance);
            }
        }

        encoder();

    } while (!isEncoderButtonPressed);
}

void countingLinearMovement(double length){

    Serial.println("В методе подсчёта импульсов");

    // Переводит длину в требуемое количество импульсов для окончания счёта
    long count = convertLengthToImpulse(length);

    // Количество пришедших импульсов
    int countImpulse = 0;

    // Опрос последнего состояния энкодера
    int lastState = digitalRead(digitalPin_8_impulseCounter);

    // Отпускает магнит
    setRealyStatus(false);

    // Запоминает время начала отсчёта
    long startTime = micros();

    Serial.println("Количество требуемых импульсов ");
    Serial.print(count);

    // Работает до момент когда пришедших импульсов будет столько же сколько надо для данной длины
    while(countImpulse < count){

        // Опрашивает состояние энкодера
        int newState = digitalRead(digitalPin_8_impulseCounter);

        // Пример: Если перед запуском было состояние 0, а новое 0 - значит энкодер еще не повернулся
        // Если же его состояние 1 - значит произошел поворот энкодера и он поменял состояние
        if(newState != lastState){

            // Счётчик прибавляет единицу
            countImpulse++;

            // Новое значение присваиваетя старому значению  
            lastState = newState;

            Serial.println(countImpulse);
        }

        // Остановка магнита по кнопки
        if (isAnalogButtonPressed(analogPin_3_stopRemoteControlButton)){
          setRealyStatus(true);
          countImpulse = count;
        }
    }

    // После остановки счёта срабатывает реле и останавливает груз
    setRealyStatus(true);

    // Получаем время окончания отсчёта
    long endTime = micros();

    // Находим время движения груза и выводим его на экран
    printResult(endTime - startTime);

    // Ожидание 5с и возвращение в исходное состояние
    delay(5000);
}

void printLinearMovementRun(){

    lcd.setCursor(0, 0);
    lcd.clear();

    lcd.print(L"   ПОДСЧЁТ...   ");
}

void printLinearMovementDistance(int distanceToPrint){
    if (distanceToPrint < 0){
        distanceToPrint = 0;
    }

    lcd.setCursor(0, 0);
    lcd.clear();

    lcd.print(L"   РАССТОЯНИЕ   ");

    printLength(distanceToPrint, 1);
}

void printResult(long timerTimeToPrint){

    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.print(L"ВРЕМЯ ");

    printTime(timerTimeToPrint, 1);
}

/*********************************** ОКОНЧАНИЕ МЕТОДОВ ЛИНЕЙН ПЕРЕМЕЩ ***********************************/

/*********************************** НАЧАЛО МЕТОДОВ КОЛЕБАНИЙ ***********************************/


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

  printTime(time, 1);
}

/*********************************** ОКОНЧАНИЕ МЕТОДОВ КОЛЕБАНИЯ ***********************************/


/*********************************** НАЧАЛО МЕТОДОВ СЕКУНДОМЕРА ***********************************/

// нужно взять 2 кнопки (аналоговые) и проверять нажаты ли они

void stopwatch(){ // Секундомер

  const byte analogPinForStartButtonOfStopWatch = analogPin_0_startMeasurementButton; //номер порта для старта
  const byte analogPinForStopButtonOfStopWatch = analogPin_1_stopMeasurementButton;  //номер порта для стопа

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

  printTime(timeToPrint, 1);
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

  printTime(timeToPrint, 1);
}

/*********************************** ОКОНЧАНИЕ МЕТОДОВ СЕКУНДОМЕРА ***********************************/


/*********************************** НАЧАЛО МЕТОДОВ ТАЙМЕРА ***********************************/

void timer(){ // Таймер
  const int numberSound = 2;        //количество звуковых сигналов 
  const int delayBetweenSound = 500;// время между звуковыми сигналами при остановке

  byte analogPinForStartButtonOfTimer = analogPin_0_startMeasurementButton; //номер порта для старта
  
  double stepForTimer = 0.5;  // шаг при задании необходимого времени для секунды
  double setTimeForTimer = 0; // предварительная настройка задаваемого времени
  double currentTime = 0;     // предварительная настройка текущего времени

  double needTimeForTimer = 0;

  bool isNeedPreView = true;

  counter = 0; //

  int last = counter;
  bool isNeedPrint = true;

  delay(50);
  do {
    if (last != counter || isNeedPreView) {

      counter = counter <= 0 ? 0 : counter;
      last = counter;

      setTimeForTimer = last * stepForTimer;

      printPreViewForTimer(setTimeForTimer);

      isNeedPreView = false;
    }
    if (isAnalogButtonPressed(analogPinForStartButtonOfTimer)) {
      needTimeForTimer = micros() + ( setTimeForTimer * 1000000 );
      while (needTimeForTimer > micros())
      { 
        delay(100); // обновлять значения времени через 100мс, чтобы экран не дребежал
        currentTime = needTimeForTimer - micros();
        printRemaimingTimeForTimer(currentTime);
      }
     
      printWhenTimeEndsForTimer();
      soundSignal(numberSound, delayBetweenSound);
      delay(1000); // показывать уведомление об окончании времени 1с

      // возвращение значений в исходное положение
      counter = 0;
      isNeedPreView = true;
    }

    encoder(); // проверяем вдруг кнопка будет нажата, тогда цикл while прекратиться

  } while (!isEncoderButtonPressed);
}

void printPreViewForTimer(double setTimeToPrint){
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

  printTime(timerTimeToPrint, 1);
}

void printWhenTimeEndsForTimer(){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"      СТОП!    ");
}

/*********************************** ОКОНЧАНИЕ МЕТОДОВ ТАЙМЕРА ***********************************/


/*********************************** НАЧАЛО БАЗОВЫХ МЕТОДОВ ***********************************/

bool isAnalogButtonPressed(byte analogPinNumber){
  return analogRead(analogPinNumber) < 300;
}

bool isPinHigh(byte pinNumber){
  return digitalRead(pinNumber) == HIGH;
}

void soundSignal(int numbersOfSound, int timeBetweenSonds){
    const int valueOfSignal = 100;

    for (int j = 0; j < numbersOfSound; j++){

        analogWrite(digitalPin_3_soundSignal, 100); // Включаем звук
        delay(timeBetweenSonds);
        analogWrite(digitalPin_3_soundSignal, 0); // Выключаем звук
        delay(timeBetweenSonds);
    }
}

void setRealyStatus(bool status){
    if (status){
        digitalWrite(digitalPin_10_relay, HIGH);
    } else {
        digitalWrite(digitalPin_10_relay, LOW);
    }
}

// Проверяет какой статус у кнопок управления магнитом и управляет магнитом
void checkStatusRealyButton_andSetNewStatusToRealy(){
    if (isAnalogButtonPressed(analogPin_2_startRemoteControlButton)){
        setRealyStatus(false);   
    }

    if (isAnalogButtonPressed(analogPin_3_stopRemoteControlButton)){
        setRealyStatus(true);
    }
}

// Переводит длину в требуемое количество импульсов для окончания счёта
int convertLengthToImpulse(double length){
    return length / step;
}

void encoder(){
    isEncoderButtonPressed = false;

    switch (GetEncoderState()){
        case eNone:
        
        return;

    case eButton: // Нажали кнопку
        Serial.println(" Нажали кнопку энкодера ");
        isEncoderButtonPressed = true;
        break;

    //!!!! пины вращения подключены неправильно, а именно при вращении вправо мы переходим в eLeft и наоборот
    // поэтому ставлю при вращении влево counter++;
    case eLeft:
        // Энкодер вращается влево
        Serial.println(" Энкодер вращается влево ");

        counter--;
        break;

    case eRight:
        // Энкодер вращается вправо
        Serial.println(" Энкодер вращается вправо ");

        counter++;
        break;
    }
}

void printLength(double length, byte line){
  // Проверка чтобы значение было либо 1, либо 0
  line = line != 0 ? 1 : line;

  // Очистка линии дисплея от прошлых значений
  cleanLineOnLCD(line);

  lcd.setCursor(0, line);
  Serial.println(length);
  lcd.print(String(length));
  lcd.print(L" см");
}

void printTime(long time, byte line){
  // Проверка чтобы значение было либо 1, либо 0
  line = line != 0 ? 1 : line;

  // Очистка линии дисплея от прошлых значений
  cleanLineOnLCD(line);

  // Устанаваливаем курсор на нужную линию
  lcd.setCursor(0, line);

  lcd.print(convertTimeToString(time));
  lcd.print(L" сек.");
  
}

String convertTimeToString(long time){
  // example. time = 5672000 millis
  long second = time / 1000000; // 5
  long millisecond = time - second * 1000000; // 672000
  millisecond = millisecond / 100; // 6720

  return String(second) + "." + String(millisecond);
}

void cleanUpperLine(){
  cleanLineOnLCD(0);
}

void cleanBottomLine(){
  cleanLineOnLCD(1);
}

void cleanLineOnLCD(byte line){
  // Проверка чтобы значение было либо 1, либо 0
  line = line != 0 ? 1 : line;

  lcd.setCursor(0, line);
  lcd.print("                ");
}

/*********************************** ОКОНЧАНИЕ БАЗОВЫХ МЕТОДОВ ***********************************/