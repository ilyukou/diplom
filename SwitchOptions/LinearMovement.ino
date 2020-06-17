#define _LCD_TYPE 1
#include <LCD_1602_RUS_ALL.h>

// Инициализация портов ввода-вывода

LCD_1602_RUS <LiquidCrystal_I2C> lcd(0x27, 16, 2);

// Цифровые порты
const int digitalPin_2_DT = 2;
const int digitalPin_3_soundSignal = 3;
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
                Result = eRight; // B=1 => энкодер вращается по часовой
                else
                Result = eLeft; // B=0 => энкодер вращается против часовой
            }

            EncoderAPrev = EncoderA; // запомним текущее состояние
        }
    }
    return Result;
}

void setup(){

    lcd.begin(); // Включение экрана
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


    Serial.begin(9600);
    counter = 0;
}

bool isNeedPreView = true;
bool isEncoderButtonPressed = false;

void loop(){

    // Для единоразового обновление экрана при включении или выхода с режима
    if (isNeedPreView){
        prtintTitle();
        printOption();
        isNeedPreView = false;
    }

    // Считывание состояние кнопок отвечающих за реле
    checkStatusRealyButton_andSetNewStatusToRealy();

    // Переход в режим
    if (isEncoderButtonPressed){
        goToOption();
        isNeedPreView = true; // указывает что после работы в режиме нужно отрисовать экран заново
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

void prtintTitle(){

  lcd.setCursor(0, 0);
  lcd.clear();

  lcd.print(L"PEЖИМ");
}

void printOption(){

  lcd.setCursor(0, 1);

  if (counter =! 0){
    counter = 0;
  }

  switch (counter){
    case 0: // ЛИНЕЙНОЕ
        lcd.print(L"1. ЛИНЕЙНОЕ");
        break;
  }
}

void goToOption(){

    switch (counter){

        case 0: // ЛИНЕЙНОЕ
            isEncoderButtonPressed = false;z
            linearMovement();
            counter = 0; // вдруг внутри опции изменялся counter, ему нужно присвоить прежнее значение
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


        if (needDistance > 0 && isAnalogButtonPressed(analogPin_0_startMeasurementButton)){
            countingLinearMovement(needDistance);
        }

        encoder();

    } while (!isEncoderButtonPressed);
}

void countingLinearMovement(double length){

    long count = convertLengthToImpulse(length);

    int countImpulse = 0;
    

    int lastState = digitalRead(digitalPin_8_impulseCounter);

    setRealyStatus(false);

    long startTime = micros();

    while(countImpulse < count){

        int newState = digitalRead(digitalPin_8_impulseCounter);

        if(newState != lastState){
            countImpulse++;

            lastState = newState;
        }
    }

    setRealyStatus(true);

    long endTime = micros();

    printResult(endTime - startTime);
    delay(5000);
}

void printLinearMovementDistance(int distanceToPrint){
    if (distanceToPrint < 0){
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

void printResult(long timerTimeToPrint){

    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.print(L"ВРЕМЯ ");

    long second = timerTimeToPrint / 1000000;               
    long millisecond = timerTimeToPrint - second * 1000000;
    millisecond = millisecond / 100;

    lcd.print(String(second));
    lcd.print(".");
    lcd.print(String(millisecond));
    lcd.setCursor(0, 1);
    lcd.print(L" сек.");
}

/***********************************  LINEINOE PEREME and his methods END ********************/

/***********************************  COMMON methods START ***********************************/

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

// Конвертирует значения длины в количество импульсов
int convertLengthToImpulse(double length){
    return length / step;
}

void encoder(){
    switch (GetEncoderState()){
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

/***********************************  COMMON methods END ***********************************/