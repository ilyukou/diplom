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
}

bool isNeedPreView = true;
bool isEncoderButtonPressed = false;
void loop() {

  if(isNeedPreView){
    prtintTitle();
    printOption();
    isNeedPreView = false;
  }

  if(isEncoderButtonPressed){
    goToOption();
  }

  switch (GetEncoderState()) {
    case eNone:
      return;
    case eButton: { // Нажали кнопку
        isEncoderButtonPressed = true;
        break;
      }
    case eLeft: {   // Энкодер вращается влево
        counter--;
        prtintTitle();
        printOption();
        isEncoderButtonPressed = false;
        break;
        
      }
    case eRight: {  // Энкодер вращается вправо
        counter++;
        prtintTitle();
        printOption();
        isEncoderButtonPressed = false;
        break;
      }
  }
  delay(100);
}

void encoder(){
  switch (GetEncoderState()) {
    case eNone:
      isEncoderButtonPressed = false;
      return;
    case eButton: { // Нажали кнопку
        isEncoderButtonPressed = true;
        break;
      }
    case eLeft: {   // Энкодер вращается влево
        counter--;
        isEncoderButtonPressed = false;
        break;
        
      }
    case eRight: {  // Энкодер вращается вправо
        counter++;
        isEncoderButtonPressed = false;
        break;
      }
  }
}
void prtintTitle(){
  if(counter > 4){
    counter = 0;
  }

  if(counter < 0){
    counter = 4;
  }
  
  Serial.println(counter);

  lcd.setCursor(0,0);
  lcd.clear();
  
  lcd.print("PE");
  lcd.print(char(6));
  lcd.print(char(4));  
  lcd.print("M ");
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
  delay(50);
  do {
    encoder();
    if(last != counter || isNeedPrint){
      printlinearMovement();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }while (!isEncoderButtonPressed);
  isNeedPreView = true;
  isEncoderButtonPressed = false;
}

void printlinearMovement(){
   if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,0);
   lcd.clear();

   lcd.print("PACCTO");
   lcd.print(char(5));
   lcd.print("H");
   lcd.print(char(4));
   lcd.print("E");
   lcd.print(" ");
   lcd.print(counter);

   lcd.setCursor(0,1);
   lcd.print("BPEM");
   lcd.print(char(5));
   lcd.print(" ");
   lcd.print(0);
   
   lcd.print(" сек.");
}

void angleMovement(){ // Углового перемещения
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  delay(50);
  do {
    encoder();
    if(last != counter || isNeedPrint){
      printAngleMovement();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }while (!isEncoderButtonPressed);
  isNeedPreView = true;
  isEncoderButtonPressed = false;
}

void printAngleMovement(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,0);
   lcd.clear();

   lcd.print("Y");
   lcd.print(char(1));
   lcd.print("O");
   lcd.print(char(2));
   lcd.print(" ");
   lcd.print(counter);

   lcd.setCursor(0,1);
   lcd.print("BPEM");
   lcd.print(char(5));
   lcd.print(" ");
   lcd.print(0);
   
   lcd.print(" сек.");
}

void oscillation(){ // Колебания
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  delay(50);
  do {
    encoder();
    if(last != counter || isNeedPrint){
      printOscillation();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }while (!isEncoderButtonPressed);
  isNeedPreView = true;
  isEncoderButtonPressed = false;
}

void printOscillation(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,0);
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

   lcd.setCursor(0,1);
   lcd.print("BPEM");
   lcd.print(char(5));
   lcd.print(" ");
   lcd.print(0);
   
   lcd.print(" сек.");
}

void stopwatch(){ // Секундомер
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  delay(50);
  do {
    encoder();
    if(last != counter || isNeedPrint){
      printStopwatch();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }while (!isEncoderButtonPressed);
  isNeedPreView = true;
  isEncoderButtonPressed = false;
}

void printStopwatch(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,0);
   lcd.clear();

  lcd.print("HA");
  lcd.print(char(6));
  lcd.print("M");
  lcd.print(char(4));
  lcd.print("TE");

  lcd.print(" ");

  lcd.print("KHO");
  lcd.print(char(7));
  lcd.print("KY");


  lcd.setCursor(0,1);
  lcd.print("      ");
  lcd.print(char(7));
  lcd.print("YCK");
  lcd.print(" ");
}

void timer(){ // Таймер
  counter = 0;
  int last = counter;
  bool isNeedPrint = true;
  delay(50);
  do {
    encoder();
    if(last != counter || isNeedPrint){
      printTimer();
      
      last = counter;
      isNeedPrint = false;
    }
    delay(10);
  }while (!isEncoderButtonPressed);
  isNeedPreView = true;
  isEncoderButtonPressed = false;
}

void printTimer(){
  if(counter < 0 ){
     counter = 0;
   }
   
   lcd.setCursor(0,0);
   lcd.clear();
  lcd.print("YCTAHOB");
  lcd.print(char(4));
  lcd.print("TE"); 

  lcd.setCursor(0,1);
   lcd.print("BPEM");
   lcd.print(char(5));
   lcd.print(" ");
   lcd.print(counter);
   lcd.print(" сек.");
}
 
