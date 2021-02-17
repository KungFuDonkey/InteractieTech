#include "EventQueue.h"

EventQueue::EventQueue()
{
  for(int i = 0; i < QueSize; i++){
    queue[i] = nullptr;
  }
}
void EventQueue::Add(Event *e)
{
  queue[Count] = e;
  Count++;
  Rootify(Count);
}

Event* EventQueue::Get(unsigned long millis)
{
  if(Count == 0 || millis < queue[0]->time) return nullptr;
  Event* result = queue[0];
  Count--;
  queue[0] = queue[Count];

  queue[Count] = nullptr; //can be removed
  
  
  Heapify(1);
  return result;
}

void EventQueue::Rootify(int index){
  if(index == 1 || queue[index - 1]->time > queue[index / 2 - 1]->time) return;
  Swap(index-1, index / 2 -1);
  Rootify(index / 2);
}

void EventQueue::Heapify(int index){
  if(index * 2 > Count) return;
  Event* e = queue[index - 1];
  int otherindex = index;
  if(e->time > queue[index * 2 - 1]->time){
    otherindex = index * 2;
    e = queue[otherindex - 1];
  }
  if(index * 2 + 1 <= Count && e->time > queue[index * 2]->time)
    otherindex = (index * 2) + 1;
  
  if (otherindex != index){
    Swap(index - 1, otherindex - 1);
    Heapify(otherindex);
  }
}

void EventQueue::Swap(int index1, int index2){
  Event* tmp = queue[index1];
  queue[index1] = queue[index2];
  queue[index2] = tmp;
}

/*
void EventQueue::Debug(){
  Serial.print("Count: ");
  Serial.print(Count);
  Serial.println(" Queue:");
  for(int i = 0; i < QueSize; i++){
    if(queue[i] != nullptr){
      Serial.print(queue[i]->time);
      Serial.print(" ");
    }
    else{
      Serial.print("NULL ");
    }
  }
  Serial.println("");
}
*/
