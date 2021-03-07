#include "EventQueue.h"
#include "DallasTemperature.h"
#include "OneWire.h"
#include "Button.h"
#include <LiquidCrystal.h>
#include "EEPROM.h"
#include "stdint.h"

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




#define returnToMenuTimer 10000

#define airwick 13
#define trigPin 10
#define echoPin 11
#define LightSensorPin A0

#define tempPin A1

#define magnetPin 12

#define interruptPin digitalPinToInterrupt(2)

#define AirwickFireTime 16000
#define MinLight 600
#define MinDistance 10

#define fireStatePin A4
#define heartBeatPin A5

#define fireButtonPin 3 

#define menuUpPin A2
#define menuConfirmPin A3

#define menuCount 2

OneWire oneWire(tempPin);

DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

EventQueue queue; // gebruikt default constructor

Button menuUpButton;
Button menuConfirmButton;
int drawSpeed = 1000;
int magnet = HIGH;
bool settings = false;
bool active = false;
int distance = MinDistance;
long duration;
int light = MinLight;
float temperature;
bool poopm = false;
bool firing = false;
int menu = 0;
int heartBeat = LOW;
int menuItem = 0;

void setup() {
  ENABLELOGGING;
  pinMode(airwick,OUTPUT);
  pinMode(3,INPUT);
  attachInterrupt(interruptPin,InterruptRoutine,FALLING);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(LightSensorPin, INPUT);
  pinMode(fireStatePin,OUTPUT);
  pinMode(heartBeatPin,OUTPUT);
  pinMode(fireButtonPin, INPUT);
  pinMode(magnetPin,INPUT);
  pinMode(tempPin,INPUT);
  queue.Enqueue(new Event(UpdateBeat,0));
  queue.Enqueue(new Event(DisplayMenu,0));
  queue.Enqueue(new Event(UpdateTemp,0));
  menuConfirmButton.Init(menuConfirmPin);
  menuUpButton.Init(menuUpPin);

  lcd.begin(16, 2);
}

void loop() {
  queue.PerformEvents();
  if(active && !settings){
    
    if(distance < MinDistance && light < MinLight){
      unsigned long sprayDelay = GetDelay();
      //queue.Enqueue(new Event(AirwickFire,sprayDelay * 1000 - AirwickFireTime));
      active = false;
    }
    
    if(digitalRead(fireButtonPin) == LOW){
      //AirwickFireTwice();
    }
  }
  if(!settings && menuUpButton.GetDown()){
    MenuUp();
  }
#ifdef DEBUG
  if(active){
    digitalWrite(fireStatePin,HIGH);
  }
  else{
    digitalWrite(fireStatePin,LOW);
  }
#endif



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
  if(!firing){
    FireRoutine();
    LOGLN(F("FIRE TWICE"));
    queue.Enqueue(new Event(AirwickFire,AirwickFireTime + 1000));
    active = false;
  }

}

//Fires the airwick once
void AirwickFire(){
  if(!firing){
    FireRoutine();
    LOGLN(F("Fire"));
  }
}

void FireRoutine(){
  active = false;  
  firing = true;
  digitalWrite(airwick,HIGH);
#ifndef DEBUG
  digitalWrite(fireStatePin,HIGH);
#endif
  queue.Enqueue(new Event(AirwickOff,AirwickFireTime));
}

//Disables the airwick
void AirwickOff(){
  int shots = GetShots();
  shots = shots - 1 < 0 ? 0 : shots - 1;
  WriteShots(shots);
  firing = false;
  digitalWrite(airwick,LOW);
#ifndef DEBUG
  digitalWrite(fireStatePin,LOW);
#endif
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
}

//Enables interrupts if disabled
void EnableInterrupt(){
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

void UpdateBeat(){
  heartBeat = !heartBeat;
  digitalWrite(heartBeatPin,heartBeat);
  queue.Enqueue(new Event(UpdateBeat, 1000));
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
  queue.Enqueue(new Event(UpdateTemp, 1000));
}

void UpdateMagnet(){
  magnet = digitalRead(magnetPin);
  LOG(F("Magnet: "));
  LOGLN(magnet);
  if(active) queue.Enqueue(new Event(UpdateMagnet, 200));
}

void MenuUp(){
  menu = menu + 1 >= menuCount ? 0 : menu + 1;
}

unsigned long lastAction = returnToMenuTimer;
void DisplayMenu(){
  queue.Enqueue(new Event(DisplayMenu,drawSpeed));
  lcd.setCursor(0,0);
  if (menu == 1){
    previewSettingsMenu();
    drawSpeed = 50;
    CheckTimer();
  }
  else if (menu == 2){
    settingsMenu();
    drawSpeed = 50;
  }
  else {
    defaultMenu();
    drawSpeed = 250;
    lastAction = millis() + returnToMenuTimer;
  }

}

void defaultMenu(){
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C      ");
  lcd.setCursor(0, 1);
  lcd.print("Shots: ");
  lcd.print(GetShots());
  lcd.print("       ");
}

bool printed = false;

void previewSettingsMenu(){
  lcd.print("Settings       ");
  lcd.setCursor(0,1);
  lcd.print("               ");

  if(menuConfirmButton.GetDown()){
    menu = 2;
    settings = true;
    ResetMenuTimer();
    menuItem = 0;
  }
}

int GetDelay(){
  return EEPROM.read(2);
}
void WriteDelay(int delay){
  EEPROM.write(2, delay);
}

int GetShots(){
  byte firstValue = EEPROM.read(0);
  byte secondValue = EEPROM.read(1);
  return (firstValue << 8) + secondValue;
}
void WriteShots(int shots){
  int firstValue = shots >> 8;
  int secondValue = (shots << 8) >> 8;
  EEPROM.write(0,firstValue);
  EEPROM.write(1,secondValue);
}

void settingsMenu(){
  lcd.setCursor(0,0);
  if(menuItem == 0){
    if(!printed){
      LOGLN("DELAY");
      lcd.print("Change delay   ");
      lcd.setCursor(0,1);
      lcd.print("               ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown())
    {
      menuItem = 1;
      ResetMenuTimer();
    }
    if (menuUpButton.GetDown()){
      menuItem = 2;
      ResetMenuTimer();
    }
  }
  else if(menuItem == 1){
    LOGLN("NEWDELAY");
    if(!printed){
      lcd.print("New delay:     ");
      printed = true;
    }
    CheckTimer();
    lcd.setCursor(0, 1);
    int delay = GetDelay();
    if (menuUpButton.GetDown())
    {
      delay = delay + 5 > 60 ? 15 : delay + 5;
      WriteDelay(delay);
      ResetMenuTimer();
    }
    lcd.print(delay);
    lcd.print("s            ");

    if (menuConfirmButton.GetDown())
    {
      menuItem = 0;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 2){
    if(!printed){
      LOGLN("RESET SPRAYS");
      lcd.print("Reset sprays   ");
      lcd.setCursor(0,1);
      lcd.print("               ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown())
    {
      menuItem = 3;
      ResetMenuTimer();
    }
    if (menuUpButton.GetDown()){
      menuItem = 5;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 3){
    if(!printed){
      LOGLN("CONFIRM");
      lcd.print("Confirm?        ");
      lcd.setCursor(0,1);
      lcd.print("               ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown())
    {
      WriteShots(2400);
      menuItem = 2;
      ResetMenuTimer();
    }
    if (menuUpButton.GetDown())
    {
      menuItem = 2;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 4){
    if(!printed){
      LOGLN("RESETTED");
      lcd.print("Sprays have been reset");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown() || menuUpButton.GetDown()){
      menuItem = 2;
      ResetMenuTimer();
    }
  }
  else{
    if(!printed){
      LOGLN("BACK");
      lcd.print("Back           ");
      lcd.setCursor(0,1);
      lcd.print("               ");
    }
    CheckTimer();
    if(menuConfirmButton.GetDown()){
      settings = false;
      menu = 1;
      ResetMenuTimer();
    }
    if(menuUpButton.GetDown()){
      menuItem = 0;
      ResetMenuTimer();
    }
  }
}
void ResetMenuTimer(){
  printed = false;
  lastAction = millis();
}

void CheckTimer(){
  if(millis() - lastAction > returnToMenuTimer){ 
    settings = false;
    menu = 0;
    printed = false;
  }
}
