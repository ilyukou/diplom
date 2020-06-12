#define _LCD_TYPE 1
#include <LCD_1602_RUS_ALL.h>

LCD_1602_RUS <LiquidCrystal_I2C> lcd(0x27, 16, 2);

int pin = 6;

long count = 0;

const int pin_relay = 10;

// кнопка включения магнита
const int pinRealyButtonOn = 2;

// кнопка выключения магнита
const int pinRealyButtonOff = 3;

// количество отверстий
const int countOfHoles = 68;    

const double pi = 3.1415926;

// длина окружности 
const double diameter = 2.5; // cm

// ( diameter * pi ) / ( countOfHoles * 2 );
const double step =  0.0577; // cm


void setup() { 
    lcd.begin();
    lcd.backlight(); // Включаем подсветку дисплея
    lcd.print("Зажмите меня");
    Serial.begin(9600);

    pinMode(pin, INPUT);

    pinMode(pin_relay, OUTPUT);

    // кнопка включения магнита
    pinMode(pinRealyButtonOn, INPUT);

    // кнопка выключения магнита
    pinMode(pinRealyButtonOff, INPUT);
}

void loop() {

    if(isAnalogButtonPressed(pinRealyButtonOn)){
        managment();
        
    }

    if(isAnalogButtonPressed(pinRealyButtonOff)){
        setRealyStatus(true);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Зажали");
    }
}

void managment(){
    delay(500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Подсчет");

    countingTime(10);
    delay(3000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Зажали");
}

void countingTime(double length){ // в см

    long count = convertLengthToImpulse(length);

    Serial.println(count);

    long startTime = micros();

    counting(count);

    long endTime = micros();

    printResult(endTime - startTime);
}

void counting(long count){

    int lastState = digitalRead(pin);
    Serial.println(lastState);

    setRealyStatus(false);

    Serial.println("Начало цикла");
    while(count > 0){

        int newState = digitalRead(pin);

        if(newState != lastState){
            count--;

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(count*step);

            lastState = newState;
        }
    }
    setRealyStatus(true);
    Serial.println("Конец");
}

void printResult(long timerTimeToPrint){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(L"ВРЕМЯ ");

  // example. timeToPrint = 5672000 millis
  long second = timerTimeToPrint / 1000000;               // 5
  long millisecond = timerTimeToPrint - second * 1000000; // 672000
  millisecond = millisecond / 100;                        // 6720

  // 5.67
  lcd.print(String(second));
  lcd.print(".");
  lcd.print(String(millisecond));
  lcd.setCursor(0, 1);
  lcd.print(L" сек.");
}

bool isAnalogButtonPressed(byte analogPinNumber){
  return analogRead(analogPinNumber) < 300;
}

int convertLengthToImpulse(double length){ // 20.5  cm
    return length / step; // 177
}

void setRealyStatus(bool status){
  if (status){
    digitalWrite(pin_relay, HIGH);
  } else {
    digitalWrite(pin_relay, LOW);
  }
}

void checkStatusRealyButton_andSetNewStatusToRealy(){
  if (isAnalogButtonPressed(pinRealyButtonOn)){
    setRealyStatus(false);
     
  }

  if (isAnalogButtonPressed(pinRealyButtonOff)){
    setRealyStatus(true);
  }
}