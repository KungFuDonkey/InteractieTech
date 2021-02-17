#include "EventQueue.h"
#define dled1 7
#define dled2 13
#define aled 9

EventQueue* queue;
void setup() {
  queue = new EventQueue();
  queue->Add(new Event(On,1000));
  queue->Add(new Event(Off,2000));
  queue->Add(new Event(On2,500));
  queue->Add(new Event(Off2,1500));
  queue->Add(new Event(ChangeBrightness, 0));
  pinMode(dled1,OUTPUT);
  pinMode(dled2,OUTPUT);
  pinMode(aled,OUTPUT);
  //Serial.begin(115200); //uses a lot of memory
}

void loop() {
  PerformEvent(queue->Get(millis()));
}

void PerformEvent(Event* e){
  if(e != NULL){
    e->action();
    delete(e);
  }
}

#define changespeed 2
#define recallspeed 25
short direction = 1;
int brightness = 0;
void ChangeBrightness(){
  brightness += direction * changespeed;
  if(brightness > 255){
    direction = -1;
    brightness = 255;
  }
  else if (brightness < 0){
    direction = 1;
    brightness = 0;
  }
  analogWrite(aled, brightness);
  queue->Add(new Event(ChangeBrightness,millis() + 5));
}

void On(){
  digitalWrite(dled1,1);
  queue->Add(new Event(Off,millis() + 1000));
}

void Off(){
  digitalWrite(dled1,0);
  queue->Add(new Event(On,millis() + 1000));
}

void On2(){
  digitalWrite(dled2,1);
  queue->Add(new Event(Off2,millis() + 1000));
}

void Off2(){
  digitalWrite(dled2,0);
  queue->Add(new Event(On2,millis() + 1000));
}
