#include "Arduino.h"

class Button {
public:
  int pin;
  int buttonID; 

  int lastSteadyValue = 1023;
  int lowerBound;
  int upperBound;
  bool pressed = false;

  Button() {
    pin = 0;
  };
  void Init(int pin, int buttonID){ // on multiple button pins the buttonID is the order in which the buttons are connected, 1-based
    pinMode(pin, INPUT);
    this->pin = pin;
    
    int estimateVolt = 1024 - (1024 / buttonID);
    lowerBound = estimateVolt - 10;
    upperBound = estimateVolt + 10;
  }

  // Gets if a button is down
  bool GetDown(){
    int value = analogRead(pin);
    if (value > lowerBound && value < upperBound && !pressed)
    {
      pressed = true;
      return true;
    }
    else if (pressed && (value < lowerBound || value > upperBound))
    {
      pressed = false;
    }
    
    return false;
  }
};