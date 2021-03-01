#include "EventQueue.h"
#include "DallasTemperature.h"
#include "OneWire.h"


#ifndef RELEASE 
#define LOG(x) Serial.print(x)
#define LOGLN(x) Serial.println(x)
#define ENABLELOGGING Serial.begin(115200)
#elif
#define LOG(x)
#define LOGLN(x)
#define ENABLELOGGING
#endif

#define airwick 13

#define trigPin 9
#define echoPin 10
#define LightSensorPin A0

#define tempPin 4

#define interruptPin digitalPinToInterrupt(2)

OneWire oneWire(tempPin);

DallasTemperature sensors(&oneWire);

int status = LOW;
EventQueue queue; // gebruikt default constructor
void setup() {
  ENABLELOGGING;
  queue.Enqueue(new Event(UpdateLight,3000));
  queue.Enqueue(new Event(UpdateDistance,1500));
  pinMode(airwick,OUTPUT);
  pinMode(3,INPUT);
  attachInterrupt(interruptPin,InterruptRoutine,FALLING);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(LightSensorPin, INPUT);
}




void loop() {
  queue.PerformEvents();
  if(status == HIGH){
    AirwickFire();
    status = LOW;
  }
}

// defines variables
int distance;
long duration;
int light;

// delayMicroseconds was used here
// this was done for the implementation of the sonar
// this code only blocks for 12 microseconds while the library
// blocks for 24 microseconds, furthermore the library uses an
// interrupt pin which is a scarce resource, besides generally
// you want to run as little interrupts as possible for better
// code flow.
void UpdateDistance(){
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  duration = pulseIn(echoPin,HIGH);
  distance = duration / 66;
  LOG(F("Distance: "));
  LOGLN(distance);
  queue.Enqueue(new Event(UpdateDistance,2000));
}

void AirwickFire(){
  digitalWrite(airwick,HIGH);
  LOGLN(F("Fire"));
  queue.Enqueue(new Event(AirwickOff,25000));
}
void AirwickOff(){
  digitalWrite(airwick,LOW);
  LOGLN(F("Reload"));
}

void InterruptRoutine(){
  status = !status;
  detachInterrupt(interruptPin);
  LOGLN(F("interrupt"));
  queue.Enqueue(new Event(EnableInterrupt,5000));
}

void EnableInterrupt(){
  EIFR = (1 << 0); // Clears the interrupt flag
  attachInterrupt(interruptPin,InterruptRoutine,FALLING);
}

void UpdateLight(){
  light = analogRead(LightSensorPin);
  queue.Enqueue(new Event(UpdateLight, 1000));
  LOG(F("Light: "));
  LOGLN(light);
}

void UpdateTemp(){
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if(tempC != DEVICE_DISCONNECTED_C){
    Serial.print(F("Temperature: "));
    Serial.println(tempC);
  }
  else{
    Serial.print(F("Error in temp sensor"));
  }

}
