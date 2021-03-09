#include "Arduino.h"

#define debounceMS 20

class Button {
public:
  int pin;
  int buttonID; 
  unsigned long lastSteadyTime;
  int lowerBound;
  int upperBound;
  bool pressed = false;
  bool pressedonce = false;

  Button() {
    pin = 0;
  };
  void Init(int pin, int buttonID){ // on multiple button pins the buttonID is the order in which the buttons are connected, 1-based
    pinMode(pin, INPUT);
    this->pin = pin;
    
    int estimateVolt = 1024 - (1024 / buttonID);
    lowerBound = estimateVolt - 50;
    upperBound = estimateVolt + 50;
  }

  // Gets if a button is down
  bool GetDown(){
    int value = analogRead(pin);
    if (value > lowerBound && value < upperBound)
    {
      if(pressed && pressedonce && lastSteadyTime - millis() > debounceMS){
        Serial.println("BUTTON WAS PRESSED");
        pressedonce = false;
        return true;
      }
      else if(!pressed) {
        lastSteadyTime = millis();
        pressed = true;
        pressedonce = true;
      }
    }
    else
    {
      pressed = false;
      pressedonce = false;
    }
    return false;
  }
};