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

#define magnetPin A2

#define interruptPin digitalPinToInterrupt(2)

#define AirwickFireTime 16000
#define MinLight 600
#define MinDistance 10

#define fireStatePin A4
#define heartBeatPin A5

#define fireButtonPin 3 

#define menuUpPin A2
#define menuConfirmPin A2

#define menuCount 2

OneWire oneWire(tempPin);

DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

EventQueue queue; // gebruikt default constructor

Button menuUpButton;
Button menuConfirmButton;
Button magnetSensor;
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

bool pooping = false;
bool peeing = false;
bool cleaning = false;
int lastMotionTime = 0;

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
  menuUpButton.Init(menuUpPin, 1);
  menuConfirmButton.Init(menuConfirmPin, 2);
  magnetSensor.Init(magnetPin, 3);
  lcd.begin(16, 2);
}

void loop() {
  queue.PerformEvents();
  /*if(active && !settings){
    
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
  }*/
#ifdef DEBUG
  if(active){
    digitalWrite(fireStatePin,HIGH);
  }
  else{
    digitalWrite(fireStatePin,LOW);
  }
#endif

  if (active && !settings)
  {
    if (magnetSensor.GetDown()) // magnet sensor is placed on the big flush button in the toilet, which is the indicator that someone did a number two
    {
      cleaning = false;
      pooping = true;
      LOGLN("SOMEBODY IS POOPING");
    }
    
    if (abs(distance - getDefaultDistance()) > 5)   // defeault distance can be set in menu, when this is not the default, the brush is used
    {                                               // if the person was not pooping this indicates they are just cleaning
      if (!pooping)
        cleaning = true;
    }

    if (millis() - lastMotionTime > GetDelay() * 1000)  // when no motion is sensed for a configurable delay
    {                                                   // the airwick will fire or not depending on the activity
      if (pooping)
      {
        AirwickFireTwice();
        LOGLN("FIRE BECAUSE POOPING");
        pooping = false;
      }
      else if (cleaning)
      {
        LOGLN("NO FIRE BECAUSE CLEANING");
        cleaning = false;
      }
      else
      {
        AirwickFire();
        LOGLN("FIRE BECAUSE PEEING");
        peeing = false;
      }
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
  distance = duration / 68;
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
<<<<<<< HEAD
=======
  queue.Enqueue(new Event(UpdateTemp,0));

  lastMotionTime = millis();
>>>>>>> 4f76fc7b79f58461c1f0a52aacc5bf26b099a014
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
    lastAction = millis();// + returnToMenuTimer;
  }
}

void defaultMenu(){
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C         ");
  lcd.setCursor(0, 1);
  lcd.print("Shots: ");
  lcd.print(GetShots());
  lcd.print("          ");
}

bool printed = false;

void previewSettingsMenu(){
  if (!printed)
  {
    LOGLN("PREVIEW SETTINGS");
    lcd.print("Settings        ");
    lcd.setCursor(0,1);
    lcd.print("                ");
    printed = true;
  }
  CheckTimer();
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
      lcd.print("Change delay    ");
      lcd.setCursor(0,1);
      lcd.print("                ");
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
      lcd.print("New delay:      ");
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
    lcd.print("s             ");

    if (menuConfirmButton.GetDown())
    {
      menuItem = 0;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 2){
    if(!printed){
      LOGLN("RESET SPRAYS");
      lcd.print("Reset sprays    ");
      lcd.setCursor(0,1);
      lcd.print("                ");
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
      lcd.print("                ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown())
    {
      WriteShots(2400);
      menuItem = 4;
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
      lcd.print("Sprays have     ");
      lcd.setCursor(0,1);
      lcd.print("been reset      ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown() || menuUpButton.GetDown()){
      menuItem = 2;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 5)
  {
    if (!printed)
    {
      LOGLN("Set default distance");
      lcd.print("Reset distance  ");
      lcd.setCursor(0,1);
      lcd.print("                ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown())
    {
      menuItem = 6;
      ResetMenuTimer();
    }
    if (menuUpButton.GetDown())
    {
      menuItem = 8;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 6)
  {
    if (!printed)
    {
      LOGLN("CONFIRM DISTANCE");
      lcd.print("Confirm?        ");
      lcd.setCursor(0,1);
      lcd.print("                ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown())
    {
      setDefaultDistance();
      menuItem = 7;
      ResetMenuTimer();
    }
    if (menuUpButton.GetDown())
    {
      menuItem = 5;
      ResetMenuTimer();
    }
    
  }
  else if (menuItem == 7)
  {
    if (!printed)
    {
      LOGLN("DISTANCE RESET");
      lcd.print("Default distance");
      lcd.setCursor(0,1);
      lcd.print("set             ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown() || menuUpButton.GetDown())
    {
      menuItem = 5;
      ResetMenuTimer();
    }
  }
  else{
    if(!printed){
      LOGLN("BACK");
      lcd.print("Back            ");
      lcd.setCursor(0,1);
      lcd.print("                ");
      printed = true;
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

void setDefaultDistance(){
  UpdateDistance();
  EEPROM.write(3, distance);
}

int getDefaultDistance(){
  return EEPROM.read(3);
}