#define DEBUG_MENU

#ifdef DEBUG 
  #define LOG(x) Serial.print(x)
  #define LOGLN(x) Serial.println(x)
  #define ENABLELOGGING Serial.begin(115200)
#else
  #define LOG(x)
  #define LOGLN(x)
  #define ENABLELOGGING
#ifdef DEBUG_MENU
  #define LOGMENU(x) Serial.print(x)
  #define LOGLNMENU(x) Serial.println(x)
  #define ENABLELOGGING Serial.begin(115200)
#endif
#endif
#include "EventQueue.h"
#include "DallasTemperature.h"
#include "OneWire.h"
#include "Button.h"
#include <LiquidCrystal.h>
#include "EEPROM.h"
#include "stdint.h"



#define returnToMenuTimer 10000

#define airwick 13
#define trigPin 10
#define echoPin 11
#define LightSensorPin A0
#define motionPin 2

#define tempPin A1


#define interruptPin digitalPinToInterrupt(motionPin)


#define AirwickFireTime 16000
#define MinLight 600
#define MinDistance 10

#define fireStatePin A4
#define heartBeatPin A5

#define lcdScreenPin A3


#define fireButtonPin digitalPinToInterrupt(3) 

#define magnetPin A2
#define menuUpPin A2
#define menuConfirmPin A2

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
bool toiletButtonPress = false;

bool pooping = false;
bool cleaning = false;
int peeing = false;
unsigned long lastMotionTime = 0;
unsigned long lastAction = returnToMenuTimer;
bool printed = false;

void setup() {
  ENABLELOGGING;
  pinMode(airwick,OUTPUT);
  pinMode(2,INPUT); //Motion sensor interrupt pin
  pinMode(3,INPUT); //Button interrupt pin
  attachInterrupt(interruptPin,InterruptRoutine,FALLING);
  attachInterrupt(fireButtonPin,AirwickFireInterrupt,FALLING);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(LightSensorPin, INPUT);
  pinMode(fireStatePin, OUTPUT);
  pinMode(heartBeatPin, OUTPUT);
  pinMode(fireButtonPin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(lcdScreenPin, OUTPUT);
  queue.Enqueue(new Event(UpdateBeat,0));
  queue.Enqueue(new Event(DisplayMenu,0));
  queue.Enqueue(new Event(UpdateTemp,0));
  menuUpButton.Init(menuUpPin, 1);
  menuConfirmButton.Init(menuConfirmPin, 2);
  lcd.begin(16, 2);
}

void loop() {
  queue.PerformEvents();
  if(active){
    digitalWrite(lcdScreenPin,HIGH);
  }
  else{
    digitalWrite(lcdScreenPin,LOW);
  }
  if (active && !settings)
  {
    if (toiletButtonPress && !cleaning) // magnet sensor is placed on the flush button in the toilet, which is the indicator that someone went to the toilet
    {
      pooping = true;
      peeing = true;
      LOGLN("SOMEBODY IS FLUSHING");
    }
    
    if (abs(distance - getDefaultDistance()) > 5)   // defeault distance can be set in menu, when this is not the default, the brush is used
    {              
      if (!pooping)   // if the person was not flushing before using the brush, this indicates they are just cleaning
      {                                             
        cleaning = true;
      }
      else            // if the person has flushed and used the brush, this indicates they did a number two
      {
        peeing = false;
        cleaning = false;
      }
    }

    if (millis() - lastMotionTime > GetDelay() * 1000)  // when no motion is sensed for a configurable delay
    {                                                   // the airwick will fire or not depending on the activity
      if (peeing)
      {
        AirwickFire();
        LOGLN("FIRE BECAUSE PEEING");
        peeing = false;
        pooping = false;
      }                                            
      else if (pooping)
      {
        AirwickFireTwice();
        LOGLN("FIRE BECAUSE POOPING");
        pooping = false;
      }
      else            // this is also when someone just walks in and out and doesn't interact with the sensors
      {
        NoFire();
        LOGLN("NO FIRE BECAUSE CLEANING");
        cleaning = false;
      }
      active = false;
    }
  }
  if(!settings && menuUpButton.GetDown()){
    MenuUp();

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


// Fires the airwick twice
void AirwickFireTwice(){
  if(!firing){
    FireRoutine();
    LOGLN(F("FIRE TWICE"));
    queue.Enqueue(new Event(AirwickFire,AirwickFireTime + 1000));
    active = false;
  }
}

// Fires the airwick via an interrupt button and disables the interrupt for
// 20 seconsds
void AirwickFireInterrupt(){
  detachInterrupt(fireButtonPin);
  queue.Enqueue(new Event(EnableFireInterrupt,20000));
  AirwickFire();
}

// Enables the fire interrupt again
void EnableFireInterrupt(){
  EIFR = (1 << 0); // Clears the interrupt flag
  attachInterrupt(interruptPin,AirwickFireInterrupt,FALLING);
}

//Fires the airwick once
void AirwickFire(){
  if(!firing){
    FireRoutine();
    LOGLN(F("Fire"));
  }
}

// The standard routine for firing
void FireRoutine(){
  active = false;  
  firing = true;
  digitalWrite(airwick,HIGH);
  digitalWrite(fireStatePin,HIGH);
  queue.Enqueue(new Event(AirwickOff,AirwickFireTime));
}

void NoFire(){
  active = false;
  firing = false;
  EnableInterrupt();
}

//Disables the airwick
void AirwickOff(){
  int shots = GetShots();
  shots = shots - 1 < 0 ? 0 : shots - 1;
  WriteShots(shots);
  firing = false;
  digitalWrite(airwick,LOW);
  digitalWrite(fireStatePin,LOW);
  EnableInterrupt();
  LOGLN(F("Reload"));
}

//Changes to the active status via the motion sensor
void InterruptRoutine(){
  active = true;
  detachInterrupt(interruptPin);
  LOGLN(F("interrupt"));
  queue.Enqueue(new Event(UpdateDistance,0));
  queue.Enqueue(new Event(UpdateLight,0));
  queue.Enqueue(new Event(CheckMagnet,0));
  queue.Enqueue(new Event(CheckMotion,0));
  lastMotionTime = millis();
}

//Enables enables the motion sensor interruupt
void EnableInterrupt(){
  light = MinLight;
  distance = MinDistance;
  EIFR = (1 << 0); // Clears the interrupt flag
  attachInterrupt(interruptPin, InterruptRoutine, FALLING);
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

//Turns the beat led on or off
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

// Switches the menu status
void MenuUp(){
  menu = menu + 1 >= menuCount ? 0 : menu + 1;
  lastAction = millis();
  printed = false;
  LOGMENU("MENUUP: ");
  LOGLNMENU(menu);
  LOGLNMENU(printed);
}


// Prints the current menu to the lcd screen
void DisplayMenu(){

  queue.Enqueue(new Event(DisplayMenu,drawSpeed));
  lcd.setCursor(0,0);
  if (menu == 1){
    previewSettingsMenu();
    drawSpeed = 50;
  }
  else if (menu == 2){
    settingsMenu();
    drawSpeed = 50;
  }
  else {
    defaultMenu();
    drawSpeed = 250;
  }
}

// The default menu template
void defaultMenu(){
  LOGMENU("Menu: ");
  LOGLNMENU(menu);
  LOGLNMENU("DEFAULT MENU");
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C         ");
  lcd.setCursor(0, 1);
  lcd.print("Shots: ");
  lcd.print(GetShots());
  lcd.print("          ");
}



// The menu before the settings template
void previewSettingsMenu(){
  if (!printed)
  {
    LOGMENU("Menu: ");
    LOGLNMENU(menu);
    LOGLNMENU("PREVIEW SETTINGS");
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

// Gets the current spray delay from EEPROM
unsigned long GetDelay(){
  unsigned long value = (unsigned long)EEPROM.read(2);
  return value;
}
// Writes the current spray delay to EEPROM
void WriteDelay(unsigned int delay){
  EEPROM.write(2, delay);
}

// Gets the shots from the EEPROM
int GetShots(){
  byte firstValue = EEPROM.read(0);
  byte secondValue = EEPROM.read(1);
  return (firstValue << 8) + secondValue;
}
// Writes the shots to the EEPROM
void WriteShots(int shots){
  int firstValue = shots >> 8;
  int secondValue = (shots << 8) >> 8;
  EEPROM.write(0,firstValue);
  EEPROM.write(1,secondValue);
}

// Prints the current settings menu to the lcd screen
void settingsMenu(){
  lcd.setCursor(0,0);
  if(menuItem == 0){
    if(!printed){
      LOGLNMENU("DELAY");
      lcd.print("Change delay    ");
      lcd.setCursor(0,1);
      lcd.print("                ");
      printed = true;
    }
    bool confirm = menuConfirmButton.GetDown();
    bool up = menuUpButton.GetDown(); 
    if (confirm)
    {
      menuItem = 1;
      ResetMenuTimer();
    }
    else if (up){
      menuItem = 2;
      ResetMenuTimer();
    }
    CheckTimer();
  }
  else if(menuItem == 1){
    LOGLNMENU("NEWDELAY");
    if(!printed){
      lcd.print("New delay:      ");
      printed = true;
    }
    CheckTimer();
    lcd.setCursor(0, 1);
    unsigned long delay = GetDelay();
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
      LOGLNMENU("RESET SPRAYS");
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
      LOGLNMENU("CONFIRM");
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
      LOGLNMENU("RESETTED");
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
      LOGLNMENU("Set default distance");
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
      LOGLNMENU("CONFIRM DISTANCE");
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
      LOGLNMENU("DISTANCE RESET");
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
      LOGLNMENU("BACK");
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

// Resets the menu timer to return to the default menu
void ResetMenuTimer(){
  printed = false;
  lastAction = millis();
}

// Checks the menu timer to return to the default menu
void CheckTimer(){
  if(millis() - lastAction > returnToMenuTimer){ 
    settings = false;
    menu = 0;
    printed = false;
  }
}

// Sets the default distance
void setDefaultDistance(){
  UpdateDistance();
  EEPROM.write(3, distance);
}

// Gets the default distance
int getDefaultDistance(){
  return EEPROM.read(3);
}

void CheckMotion(){
  int motion = digitalRead(motionPin);
  LOG(F("Motion: "));
  LOGLN(motion);
  if (motion == HIGH)
  {
    lastMotionTime = millis();
  }

  if(active) queue.Enqueue(new Event(CheckMotion, 500));
}

void CheckMagnet(){
  int value = analogRead(magnetPin);

#ifdef DEBUG
  if(toiletButtonPress != value >= 700){
    LOG(F("Magnet: "));
    LOGLN(toiletButtonPress);
  }
#endif

  toiletButtonPress = value >= 700;
  if(active) queue.Enqueue(new Event(CheckMagnet, 200));
}
