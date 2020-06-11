#define _LCD_TYPE 1
#include <LCD_1602_RUS_ALL.h>

LCD_1602_RUS <LiquidCrystal_I2C> lcd(0x27, 16, 2);

int pin = 7;

long count = 0;

const int pin_relay = 10;

// кнопка включения магнита
const int pinRealyButtonOn = 2;

// кнопка выключения магнита
const int pinRealyButtonOff = 3;


void setup() { 
    lcd.begin();
    lcd.backlight(); // Включаем подсветку дисплея
    lcd.print("Зажмите меня");
    Serial.begin(9600);

    // настройка реле
    pinMode(pin_relay, OUTPUT);

    // кнопка включения магнита
    pinMode(pinRealyButtonOn, INPUT);

    // кнопка выключения магнита
    pinMode(pinRealyButtonOff, INPUT);
}

void loop() {
    if(isAnalogButtonPressed(pinRealyButtonOn)){
        counting();
        
    }

    if(isAnalogButtonPressed(pinRealyButtonOff)){
        setRealyStatus(true);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Зажали");
    }
}

void counting(){
    delay(500);
    count = 0;
    setRealyStatus(false);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Подсчет");

    while(!isAnalogButtonPressed(pinRealyButtonOff)){
        int value = analogRead(pin);
 
        if(value > 10){
            count++;
        }
    }

    setRealyStatus(true);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Итог");

    lcd.setCursor(0, 1);
    lcd.print(count);

    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Зажали");
}


bool isAnalogButtonPressed(byte analogPinNumber){
  return analogRead(analogPinNumber) < 300;
}

void setRealyStatus(bool status){
  if (status){
    digitalWrite(pin_relay, HIGH);
  } else {
    digitalWrite(pin_relay, LOW);
  }
}