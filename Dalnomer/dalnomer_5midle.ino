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

float dist_5[5] = {0.0, 0.0, 0.0, 0.0, 0.0}; // массив для хранения трёх последних измерений
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
  if (millis() - sensTimer > 50) {                          // измерение и вывод каждые 50 мс
    // счётчик от 0 до 5
    // каждую итерацию таймера i последовательно принимает значения 0, 1, 2, 3, 4 и так по кругу
    if (i > 3) i = 0;
    else i++;

    float value = (float)sonar.ping() / 57.5;
      // получить расстояние в текущую ячейку массива
    while (value == 0) {
      delay(3);
      value = (float)sonar.ping() / 57.5;
    }
    dist_5[i] = value + 1.1 ; 

    dist = middleArray();

    delta = abs(dist_filtered - dist);                      // расчёт изменения с предыдущим
    if (delta > 1) k = 0.7;                                 // если большое - резкий коэффициент
    else k = 0.1;                                           // если маленькое - плавный коэффициент

    dist_filtered_old = dist_filtered;
    dist_filtered = dist * k + dist_filtered * (1 - k);

    dist_filtered = (float)round( dist_filtered * 10) / 10;

    float infelicity = abs(dist_filtered - dist_filtered_old);
    
  if (infelicity <= 0.1){
     
      long value = dist_filtered * 100;// вывести

      Serial.println();
      Serial.println(dist_filtered);
      Serial.println();

      lcd.setCursor(8, 1);
      lcd.print(dist_filtered);
      delay(200);
    }
    
    sensTimer = millis();                                   // сбросить таймер
  }
}

float middleArray(){   

    int lt_length = sizeof(dist_5) / sizeof(dist_5[0]);  
    float result = 0.0;
 
    qsort(dist_5, lt_length, sizeof(dist_5[0]), sort_desc);
    print_array();
    
    return dist_5[2];
}

// qsort requires you to create a sort function
int sort_desc(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  float a = *((float *)cmp1);
  float b = *((float *)cmp2);
  // The comparison
  return a < b ? -1 : (a > b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}

void print_array(){
  int lt_length = sizeof(dist_5) / sizeof(dist_5[0]);  
  for (int i = 0 ; i < lt_length ; i++){
    Serial.print(dist_5[i]);
    Serial.print(" ");
  }
  Serial.println("");
}