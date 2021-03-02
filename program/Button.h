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
  Button() {
    pin = 0;
  };
  void Init(int pin){
    pinMode(pin, INPUT);
    this->pin = pin;
  }
  bool GetDown(){
      // read the state of the switch/button:
    int currentState = digitalRead(pin);
    bool returnValue = false;
    

    if(currentState == LOW && previousState == LOW){
      return returnValue;
    }
    else if (currentState == HIGH && previousState == LOW){
      previousState = HIGH;
    }

    
    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch/button changed, due to noise or pressing:
    if (currentState != lastFlickerableState) {
      // reset the debouncing timer
      lastDebounceTime = millis();
      // save the the last flickerable state
      lastFlickerableState = currentState;
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:

      if(lastSteadyState == HIGH && currentState == LOW){
        returnValue = true;
        previousState = LOW;
      }
        
        

      // save the the last steady state
      lastSteadyState = currentState;
    }
    return returnValue;
  }
};