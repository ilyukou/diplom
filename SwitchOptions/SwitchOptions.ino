#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

byte i2[8] = { // Й
  B01110,
  B00000,
  B10011,
  B10011,
  B10101,
  B10101,
  B11001,
  B00000
};

byte g[8] = { // Г
  B11111,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B00000    
};

byte l[8] = { // Л
  B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B00000    
};
byte i[8] = { // И
  B10001,
    B10011,
    B10011,
    B10101,
    B10101,
    B11001,
    B10001,
    B00000
};

byte zh[8] ={ // Ж
    B10101,
    B10101,
    B01110,
    B00100,
    B00100,
    B01110,
    B10101,
    B00000
};

byte ya[8] = { // Я
  B01111,
  B10001,
  B10001,
  B01111,
  B00101,
  B01001,
  B10001,
  B00000
};

byte d[8] = { // Д
  B00011,
  B00101,
  B01001,
  B01001,
  B01001,
  B11111,
  B10001,
  B00000
};

byte p[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B00000
};

#define pin_CLK 2
#define pin_DT  4
#define pin_Btn 3

unsigned long CurrentTime, LastTime;
enum eEncoderState {eNone, eLeft, eRight, eButton};
uint8_t EncoderA, EncoderB, EncoderAPrev;
int8_t counter = 0;
bool ButtonPrev;

eEncoderState GetEncoderState() {
  // Считываем состояние энкодера
  eEncoderState Result = eNone;
  CurrentTime = millis();
  if (CurrentTime - LastTime >= 5) {
    // Считываем не чаще 1 раза в 5 мс для уменьшения ложных срабатываний
    LastTime = CurrentTime;
    if (digitalRead(pin_Btn) == LOW ) {
      if (ButtonPrev) {
        Result = eButton; // Нажата кнопка
        ButtonPrev = 0;
      }
    }
    else {
      ButtonPrev = 1;
      EncoderA = digitalRead(pin_DT);
      EncoderB = digitalRead(pin_CLK);
      if ((!EncoderA) && (EncoderAPrev)) { // Сигнал A изменился с 1 на 0
        if (EncoderB) Result = eRight;     // B=1 => энкодер вращается по часовой
        else          Result = eLeft;      // B=0 => энкодер вращается против часовой
      }
      EncoderAPrev = EncoderA; // запомним текущее состояние
    }
  }
  return Result;
}
const int buttonPin = 7;
const int stopButtonPin = 8;

void setup() {
  Wire.begin();
  lcd.begin();                     
  lcd.backlight();// Включаем подсветку дисплея
  lcd.setCursor(8, 1);
  lcd.createChar (0, i2);
  lcd.createChar (1, g);
  lcd.createChar (2, l);
  lcd.createChar (3, d);
  lcd.createChar (4, i);
  lcd.createChar (5, ya);
  lcd.createChar (6, zh);
  lcd.createChar (7, p);   
    
  pinMode(pin_DT,  INPUT);
  pinMode(pin_CLK, INPUT);
  pinMode(pin_Btn, INPUT_PULLUP); // Кнопка не подтянута к +5 поэтому задействуем внутренний pull-up резистор
  Serial.begin(9600);
  counter = 0;

  pinMode(buttonPin, INPUT); 
  pinMode(stopButtonPin, INPUT); 
}

bool isNeedPreView = true;
void loop() {

  if(isNeedPreView){
    prtintTitle();
    printOption();
    isNeedPreView = false;
  }

  if(digitalRead(buttonPin) == HIGH){
    goToOption();
  }
  switch (GetEncoderState()) {
    case eNone:
      return;
    case eButton: { // Нажали кнопку
        counter = 0;
        break;
      }
    case eLeft: {   // Энкодер вращается влево
        counter--;
        prtintTitle();
        printOption();
        break;
        
      }
    case eRight: {  // Энкодер вращается вправо
        counter++;
        prtintTitle();
        printOption();
        break;
      }
  }
  delay(100);
}

void encoder(){
  switch (GetEncoderState()) {
    case eNone:
      return;
    case eButton: { // Нажали кнопку
        counter = 0;
        break;
      }
    case eLeft: {   // Энкодер вращается влево
        counter--;
        break;
        
      }
    case eRight: {  // Энкодер вращается вправо
        counter++;
        break;
      }
  }
}
void prtintTitle(){
  lcd.setCursor(0,0);
  lcd.clear();
  
  lcd.print("PE");
  lcd.print(char(6));
  lcd.print(char(4));  
  lcd.print("M :");
}

void printOption(){
  lcd.setCursor(0,1);

  if(counter > 4){
    counter = 0;
  }

  if(counter < 0){
    counter = 4;
  }

  lcd.print(counter+1);
  lcd.print(". "); 
  switch(counter){
    case 0: // ЛИНЕЙНОЕ
      lcd.print(char(2));
      lcd.print(char(4));
      lcd.print("HE");
      lcd.print(char(0));
      lcd.print("HOE");
      break;
      
    case 1:// УГЛОВОЕ
      lcd.print("Y");
      lcd.print(char(1));
      lcd.print(char(2));
      lcd.print("OBOE");
      break;
      
    case 2: // ПЕРИОД
      lcd.print(char(7));
      lcd.print("EP");
      lcd.print(char(4));
      lcd.print("O");
      lcd.print(char(3));
      break;

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

void goToOption(){
  switch(counter){
    case 0: // ЛИНЕЙНОЕ
      linearMovement();
      break;
      
    case 1:// УГЛОВОЕ
      angleMovement();
      break;
      
    case 2: // ПЕРИОД
      oscillation();
      break;

    case 3: // СЕКУНДОМЕР
      stopwatch();
      break;

    case 4: // ТАЙМЕР
      timer();
      break;
  }
}

void linearMovement(){ // Линейного перемещения
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  
  while(digitalRead(stopButtonPin) == LOW){
    encoder();
    if(last != counter || isNeedPrint){
      printlinearMovement();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }
  isNeedPreView = true;
}

void printlinearMovement(){
   if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,1);
   lcd.clear();

   lcd.print("PACCTO");
   lcd.print(char(5));
   lcd.print("H");
   lcd.print(char(4));
   lcd.print("E");
   lcd.print(" ");
   lcd.print(counter);
}

void angleMovement(){ // Углового перемещения
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  
  while(digitalRead(stopButtonPin) == LOW){
    encoder();
    if(last != counter || isNeedPrint){
      printAngleMovement();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }
  isNeedPreView = true;
}

void printAngleMovement(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,1);
   lcd.clear();

   lcd.print("Y");
   lcd.print(char(1));
   lcd.print("O");
   lcd.print(char(2));
   lcd.print(" ");
   lcd.print(counter);
}

void oscillation(){ // Колебания
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  
  while(digitalRead(stopButtonPin) == LOW){
    encoder();
    if(last != counter || isNeedPrint){
      printOscillation();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }
  isNeedPreView = true;
}

void printOscillation(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,1);
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
}

void stopwatch(){ // Секундомер
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  
  while(digitalRead(stopButtonPin) == LOW){
    encoder();
    if(last != counter || isNeedPrint){
      printStopwatch();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }
  isNeedPreView = true;
}

void printStopwatch(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,1);
   lcd.clear();

   lcd.print("BPEM");
   lcd.print(char(5));
   lcd.print(" ");
   lcd.print(counter);
}

void timer(){ // Таймер
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  
  while(digitalRead(stopButtonPin) == LOW){
    encoder();
    if(last != counter || isNeedPrint){
      printTimer();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }
  isNeedPreView = true;
}

void printTimer(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,1);
   lcd.clear();

   lcd.print("TA");
   lcd.print(char(0));
   lcd.print("MEP");
   lcd.print(" ");
   lcd.print(counter);
}
