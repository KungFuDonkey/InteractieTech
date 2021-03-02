#include "EventQueue.h"
#include "DallasTemperature.h"
#include "OneWire.h"


#define DEBUG

#ifdef DEBUG 
  #define LOG(x) Serial.print(x)
  #define LOGLN(x) Serial.println(x)
  #define ENABLELOGGING Serial.begin(115200)
#else
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

#define AirwickFireTime 21000
#define MinLight 600
#define MinDistance 10

OneWire oneWire(tempPin);

DallasTemperature sensors(&oneWire);


EventQueue queue; // gebruikt default constructor
bool active = false;
int distance = MinDistance;
long duration;
int light = MinLight;
float temperature;
bool poop;


void setup() {
  ENABLELOGGING;
  pinMode(airwick,OUTPUT);
  pinMode(3,INPUT);
  attachInterrupt(interruptPin,InterruptRoutine,FALLING);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(LightSensorPin, INPUT);
}

void loop() {
  
  queue.PerformEvents();
  if(active){
    
    if(distance < MinDistance && light < MinLight){
      AirwickFire();
      active = false;
    }
  }
}



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
  if(active)
  {
    queue.Enqueue(new Event(UpdateDistance,2000));
  }
  
}


void AirwickFireTwice(){
  digitalWrite(airwick,HIGH);
  LOGLN(F("FIRE TWICE"));
  queue.Enqueue(new Event(AirwickOff,AirwickFireTime));
  queue.Enqueue(new Event(AirwickFire,AirwickFireTime + 1000));
}
//Fires the airwick once
void AirwickFire(){
  digitalWrite(airwick,HIGH);
  LOGLN(F("Fire"));
  queue.Enqueue(new Event(AirwickOff,AirwickFireTime));
}

//Disables the airwick
void AirwickOff(){
  digitalWrite(airwick,LOW);
  EnableInterrupt();
  LOGLN(F("Reload"));
}

//Changes to the active status
void InterruptRoutine(){
  active = true;
  detachInterrupt(interruptPin);
  LOGLN(F("interrupt"));
  queue.Enqueue(new Event(UpdateDistance,0));
  queue.Enqueue(new Event(UpdateLight,0));
  //queue.Enquue(new Event(UpdateTemp,0));
}

//Enables interrupts if disabled
void EnableInterrupt(){
  active = false;
  light = MinLight;
  distance = MinDistance;
  EIFR = (1 << 0); // Clears the interrupt flag
  attachInterrupt(interruptPin,InterruptRoutine,FALLING);
}

//Gets the light from the light sensor
void UpdateLight(){
  light = analogRead(LightSensorPin);
  
  if(active){
    queue.Enqueue(new Event(UpdateLight, 1000));
  }
  
  LOG(F("Light: "));
  LOGLN(light);
}

//Gets the temperature from the temperature sensor
void UpdateTemp(){
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  if(temperature != DEVICE_DISCONNECTED_C){
    LOG(F("Temperature: "));
    LOGLN(temperature);
  }
  else{
    LOGLN(F("Error in temp sensor"));
  }


}
