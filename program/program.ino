#include "EventQueue.h"
#define airwick 13
EventQueue queue; // gebruikt default constructor
void setup() {
  //Serial.begin(115200); //uses a lot of memory
  queue.Enqueue(new Event(AirwickFire,2000));
  pinMode(airwick,OUTPUT);
}

void loop() {
  queue.PerformEvents();
}

void AirwickFire(){
  digitalWrite(airwick,HIGH);
  queue.Enqueue(new Event(AirwickOff,millis() + 25000));
}
void AirwickOff(){
  digitalWrite(airwick,LOW);
  queue.Enqueue(new Event(AirwickFire,millis() + 25000));
}

void MotionDetected(){
  //motion is detected
}

/*
const int buttonPin = 7;
const int ledPin =  4;

// variables will change:
int buttonState = 0;
int lastButtonState = LOW;
int ledState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 20;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }
  
  if (millis() - lastDebounceTime > debounceDelay) {
    
    if (reading != buttonState)
    {
      buttonState = reading;
      if (buttonState == HIGH)
      {
        ledState = !ledState;
      }
      
    }
  }
  digitalWrite(ledPin, ledState);
  lastButtonState = reading;
}
*/