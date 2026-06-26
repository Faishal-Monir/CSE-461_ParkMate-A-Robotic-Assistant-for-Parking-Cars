#define LED_PIN 8      
#define BUTTON_PIN 2    


void setup() {
  pinMode(LED_PIN, OUTPUT);          
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);                
}


void loop() {
  int buttonState = digitalRead(BUTTON_PIN);


  if (buttonState == LOW) {
    digitalWrite(LED_PIN, HIGH);  
    Serial.println("Button pressed");
  }
 
  else {
    digitalWrite(LED_PIN, LOW);    
    Serial.println("Button released");
  }


  delay(100);
}
