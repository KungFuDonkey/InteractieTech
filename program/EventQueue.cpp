#include "EventQueue.h"

// Empty EventQueue
EventQueue::EventQueue()
{
  for(int i = 0; i < QueSize; i++){
    queue[i] = nullptr;
  }
}

/// Enqueue an event into the queue
void EventQueue::Enqueue(Event *e)
{
  queue[Count] = e;
  Count++;
  Rootify(Count);
}

/// longtime for catching the 49.71 day period
#define longtime 2147483648
/// Check if an event must be handled and handle if so
void EventQueue::PerformEvents()
{
  if(Count == 0 || millis() < queue[0]->time && queue[0]->time - millis() <= longtime) return;
  PerformEvent(queue[0]);
  Count--;
  queue[0] = queue[Count];
  Heapify(1);
}

/// Handle event e
void EventQueue::PerformEvent(Event* e){
  if(e != NULL){
    e->action();
    delete(e);
  }
}

/// From heap, orders the heap from a leaf to a root
void EventQueue::Rootify(int index){
  if(index == 1 || queue[index - 1]->time > queue[index / 2 - 1]->time) return;
  Swap(index - 1, index / 2 - 1);
  Rootify(index / 2);
}

/// From heap, orders the heap from the index to a leaf
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

/// Swaps 2 elements in an array
void EventQueue::Swap(int index1, int index2){
  Event* tmp = queue[index1];
  queue[index1] = queue[index2];
  queue[index2] = tmp;
}

/// for debugging
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
