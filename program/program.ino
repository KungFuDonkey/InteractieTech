#include "EventQueue.h"

#define airwick 3

EventQueue* queue;
void setup() {
  //Serial.begin(115200); //uses a lot of memory
  queue = new EventQueue();
  queue->Add(new Event(AirwickFire,2000));
  pinMode(airwick,OUTPUT);
  digitalWrite(airwick,LOW);
}

void loop() {
  digitalWrite(airwick,HIGH);
}

void PerformEvent(Event* e){
  if(e != NULL){
    e->action();
    delete(e);
  }
}

void AirwickFire(){
  digitalWrite(airwick,HIGH);
  digitalWrite(LED_BUILTIN,HIGH);
  queue->Add(new Event(AirwickOff,millis() + 25000));
}
void AirwickOff(){
  digitalWrite(airwick,LOW);
  digitalWrite(LED_BUILTIN,LOW);
  queue->Add(new Event(AirwickFire,millis() + 100000));
}
