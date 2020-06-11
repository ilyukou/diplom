int pin = 4;

long count = 0;

void setup() { 
  Serial.begin(9600);
}

void loop() {

 int count = analogRead(pin);
 
 if(count > 541 || count < 530){
    Serial.println(analogRead(pin));
 }
 
 
 delay(100);
}