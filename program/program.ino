#include "EventQueue.h"

#define airwick 3

EventQueue* queue;
void setup() {
  //Serial.begin(115200); //uses a lot of memory
  queue = new EventQueue();
  queue->Enqueue(new Event(AirwickFire,25000));
  pinMode(airwick,OUTPUT);
  digitalWrite(airwick,LOW);
}

void loop() {
  queue->PerformEvents(millis());
}

void AirwickFire(){
  digitalWrite(airwick,HIGH);
  digitalWrite(LED_BUILTIN,HIGH);
  queue->Enqueue(new Event(AirwickOff,millis() + 25000));
}
void AirwickOff(){
  digitalWrite(airwick,LOW);
  digitalWrite(LED_BUILTIN,LOW);
  queue->Enqueue(new Event(AirwickFire,millis() + 25000));
}
