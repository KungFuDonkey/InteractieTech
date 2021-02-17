#include "EventQueue.h"

EventQueue::EventQueue()
{
  for(int i = 0; i < QueSize; i++){
    queue[i] = nullptr;
  }
}
void EventQueue::Add(Event *e)
{
  Count++;
  queue[Count] = e;
  Rootify(Count);
}

Event* EventQueue::Get(unsigned long millis)
{
  if(Count == 0 || millis < queue[1]->time) return nullptr;
  Event* result = queue[1];
  queue[1] = queue[Count];
  
  queue[Count] = nullptr; //can be removed
  
  Count--;
  Heapify(1);
  return result;
}

void EventQueue::Rootify(int index){
  if(index == 1 || queue[index]->time > queue[index / 2]->time) return;
  Swap(index, index / 2);
  Rootify(index / 2);
}

void EventQueue::Heapify(int index){
  if(index * 2 > Count) return;
  Event* e = queue[index];
  int otherindex = index;
  if(e->time > queue[index << 1]->time){
    otherindex += otherindex; //index * 2
    e = queue[otherindex];
  }
  if((index * 2) + 1 <= Count && e->time > queue[(index * 2) + 1]->time)
    otherindex = (index << 1) + 1;
  
  if (otherindex != index){
    Swap(index, otherindex);
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
