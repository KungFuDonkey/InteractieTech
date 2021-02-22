#ifndef EVENTQUEUEHEADER
#define EVENTQUEUEHEADER
#define QueSize 10

#include "Event.h"

class EventQueue
{
  public:
    EventQueue();
    void Add(Event *e);
    Event* Get(unsigned long millis);
    int Count = 0;
  private:
    Event* queue[QueSize];
    void Rootify(int index);
    void Heapify(int index);
    void Swap(int index1, int index2);
};

#endif