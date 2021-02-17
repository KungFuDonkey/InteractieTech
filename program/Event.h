#ifndef EVENTHEADER
#define EVENTHEADER
#include "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\cores\\arduino\\Arduino.h"
class Event {
  public:
    Event(void (*action)(), unsigned long time);
    unsigned long time;
    void (*action)();
};

#endif