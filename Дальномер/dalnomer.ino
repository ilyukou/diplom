int ya_letter[8] = { // Я
  B01111,
  B10001,
  B10001,
  B01111,
  B00101,
  B01001,
  B10001,
  B00000
};

int i_letter[8] = { // И
  B10001,
    B10011,
    B10011,
    B10101,
    B10101,
    B11001,
    B10001,
    B00000
};


#include <LiquidCrystal_PCF8574.h>
LiquidCrystal_PCF8574 lcd(0x27);// адрес экрана 0x27

#define trigPin 4
#define echoPin 3

#include <NewPing.h>
NewPing sonar(trigPin, echoPin, 400);

float dist_3[3] = {0.0, 0.0, 0.0}; // массив для хранения трёх последних измерений
float middle, dist, dist_filtered, dist_filtered_old;
float k;
byte i, delta;
unsigned long dispIsrTimer, sensTimer;

void setup() {
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  lcd.begin(16, 2);// у нас экран 16 столбцов на 2 строки
  lcd.setBacklight(255); //установить яркость подсветки на максимум
  lcd.createChar(0, ya_letter);
  lcd.createChar(1, i_letter);
  lcd.clear(); // очистить экран и установить курсор в позицию 0, 0
  lcd.setCursor(0, 0);
  lcd.print("PACCTO");
  lcd.print(char(0)); //Я
  lcd.print("H");
  lcd.print(char(1)); //И
  lcd.print("E:");
  lcd.setCursor(14, 1);
  lcd.print("cm");
}
void loop() {
  
  // put your main code here, to run repeatedly:
  if (millis() - sensTimer > 100) {                          // измерение и вывод каждые 50 мс
    // счётчик от 0 до 2
    // каждую итерацию таймера i последовательно принимает значения 0, 1, 2, и так по кругу
    if (i > 1) i = 0;
    else i++;

    dist_3[i] = (float)sonar.ping() / 57.5 ;   // получить расстояние в текущую ячейку массива
       
    dist = middle_of_3(dist_3[0], dist_3[1], dist_3[2]);    // фильтровать медианным фильтром из 3ёх последних измерений

    delta = abs(dist_filtered - dist);                      // расчёт изменения с предыдущим
    if (delta > 1) k = 0.7;                                 // если большое - резкий коэффициент
    else k = 0.1;                                           // если маленькое - плавный коэффициент
    dist_filtered_old = dist_filtered;
    dist_filtered = (dist * k + dist_filtered * (1 - k)) ;     // фильтр "бегущее среднее"

    if (dist_filtered_old != dist_filtered){
      long value = 0;// вывести
      lcd.setCursor(8, 1);
      lcd.print((float)round(dist_filtered * 10) / 10);
      delay(150);
    }
    sensTimer = millis();                                   // сбросить таймер
  }
}

// медианный фильтр из 3ёх значений
float middle_of_3(float a, float b, float c) {
  if ((a <= b) && (a <= c)) {
    middle = (b <= c) ? b : c;
  }
  else {
    if ((b <= a) && (b <= c)) {
      middle = (a <= c) ? a : c;
    }
    else {
      middle = (a <= b) ? a : b;
    }
  }
  return middle;
}
