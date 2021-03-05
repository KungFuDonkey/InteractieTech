#include "Arduino.h"

#define DEBOUNCE_DELAY 50
class Button {
public:
  int previousState = LOW;
  int currentState = LOW;
  int lastFlickerableState = LOW;
  long lastDebounceTime = LOW;
  int lastSteadyState = LOW;
  int pin;
  int buttonID; // on multiple button pins this is the order in which the buttons are connected, 1-based

  int lastSteadyValue = 1023;
  int lowerBound;
  int upperBound;
  bool pressed = false;

  Button() {
    pin = 0;
  };
  void Init(int pin, int buttonID){
    pinMode(pin, INPUT);
    this->pin = pin;
    this->buttonID = buttonID;
    int estimateVolt = 1024 - (1024 / buttonID);
    lowerBound = estimateVolt - 10;
    upperBound = estimateVolt + 10;
  }

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