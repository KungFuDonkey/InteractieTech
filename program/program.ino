#include "EventQueue.h"
#define airwick 13
EventQueue queue; // gebruikt default constructor
void setup() {
  //Serial.begin(115200); //uses a lot of memory
  //queue.Enqueue(new Event(AirwickFire,2000));
  pinMode(airwick,OUTPUT);
  pinMode(2,INPUT);
  attachInterrupt(0,MotionDetected,RISING);
}

void loop() {
  queue.PerformEvents();
}

void AirwickFire(){
  digitalWrite(airwick,HIGH);
  queue.Enqueue(new Event(AirwickOff,millis() + 25000));
}
void AirwickOff(){
  digitalWrite(airwick,LOW);
  queue.Enqueue(new Event(AirwickFire,millis() + 25000));
}

int status = LOW;
void MotionDetected(){
  status = !status;
  digitalWrite(13,status);
}
