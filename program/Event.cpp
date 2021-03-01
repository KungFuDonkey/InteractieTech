#include "Event.h"

Event::Event(void (*action)(), unsigned long time){
  this->action = action;
  this->time = time + millis();
}
