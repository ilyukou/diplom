int pin = 7;

long count = 0;

void setup() { 
  Serial.begin(9600);
}

void loop() {

 int count = digitalRead(pin);
 
  Serial.println(count);

 
 
 delay(5);
}