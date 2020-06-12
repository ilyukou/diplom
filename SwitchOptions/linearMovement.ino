int pin = 7;

long count = 0;

// количество отверстий
const int countOfHoles = 68;    

const double pi = 3.1415926;

// длина окружности 
const double diameter = 2.5 // cm

// ( diameter * pi ) / countOfHoles;
const double step =  0.1155 // cm

void setup() { 
  Serial.begin(9600);
}

void loop() {

 int count = digitalRead(pin);
 
    Serial.println(count);

 
 
 delay(5);
}

void countingTime(double length){ // в см
    long count = convertLengthToImpulse(length);

}

void counting(long count){

    bool lastState = digitalRead(pin); // init 

    setRealyStatus(false);

    while(count > 0){

        bool newState = digitalRead(pin);

        if(newState != lastState){
            
            count--;
            lastState = newState;
        }
    }

    setRealyStatus(true);
}

void printRemaimingTimeForTimer(double timerTimeToPrint){
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