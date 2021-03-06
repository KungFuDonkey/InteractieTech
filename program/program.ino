#define RELEASE
#define LOG(x)
#define LOGLN(x)
#define ENABLELOGGING
#define LOGMENU(x)
#define LOGLNMENU(x)
#ifdef DEBUG 
  #define LOG(x) Serial.print(x)
  #define LOGLN(x) Serial.println(x)
  #define ENABLELOGGING Serial.begin(115200)
#else
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

#define motionPin 2
#define firePin 3
#define trigPin 10
#define echoPin 11
#define airwick 13

#define LightSensorPin A0
#define tempPin A1
#define magnetPin A2
#define menuUpPin A2
#define menuConfirmPin A2
#define lcdScreenPin A3
#define fireStatePin A4
#define heartBeatPin A5

#define interruptPin digitalPinToInterrupt(motionPin)
#define fireButtonPin digitalPinToInterrupt(firePin)

#define returnToMenuTimer 10000
#define AirwickFireTime 16200
#define MinLight 600
#define MinDistance 10
#define menuCount 2

OneWire oneWire(tempPin);

DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

EventQueue queue;                                                              // Uses default constructor

Button menuUpButton;
Button menuConfirmButton;

int heartBeat = LOW;
bool active = false;

int magnet = HIGH;
int distance = MinDistance;
float temperature;
int light = MinLight;
long duration;
bool firing = false;
bool toiletButtonPress = false;
unsigned long lastMotionTime = 0;
unsigned long lastAction = returnToMenuTimer;

int drawSpeed = 250;
bool settings = false;
int menu = 0;
int menuItem = 0;
int menuDefault = 0;
bool printed = false;
int actionDone = 0;
int action = 1;

bool interruptEnabled = true;
bool fireInterruptEnabled = true;

bool pooping = false;
bool cleaning = false;
int peeing = false;

void setup() {
  ENABLELOGGING;
  attachInterrupt(interruptPin, InterruptRoutine, RISING);
  attachInterrupt(fireButtonPin, AirwickFireInterrupt, FALLING);
  pinMode(airwick, OUTPUT);
  pinMode(motionPin, INPUT);
  pinMode(firePin, INPUT);
  pinMode(trigPin, OUTPUT);                                                    // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);                                                     // Sets the echoPin as an Input
  pinMode(LightSensorPin, INPUT);
  pinMode(fireStatePin, OUTPUT);
  pinMode(heartBeatPin, OUTPUT);
  pinMode(fireButtonPin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(lcdScreenPin, OUTPUT);

  digitalWrite(airwick, LOW);
  queue.Enqueue(new Event(UpdateBeat, 0));
  queue.Enqueue(new Event(DisplayMenu, 0));
  queue.Enqueue(new Event(UpdateTemp, 0));
  menuUpButton.Init(menuUpPin, 1);
  menuConfirmButton.Init(menuConfirmPin, 2);
  lcd.begin(16, 2);
}

void loop() {
  queue.PerformEvents();
  if(active) digitalWrite(lcdScreenPin, HIGH);
  else digitalWrite(lcdScreenPin, LOW);

  if (active && !settings)
  {
    if (toiletButtonPress && !cleaning)                                        // magnet sensor is placed on the flush button in the toilet, which is the
    {                                                                                                        // indicator that someone went to the toilet
      pooping = true;
      peeing = true;
      actionDone = 1;
      LOGLN("SOMEBODY IS FLUSHING");
    }
    
    if (abs(distance - getDefaultDistance()) > 5)                              // defeault distance can be set in menu, when this is not the default, the brush is used
    {              
      if (!pooping)                                                            // if the person was not flushing before using the brush, this indicates they are just cleaning
      {                                             
        cleaning = true;
      }
      else                                                                     // if the person has flushed and used the brush, this indicates they did a number two
      {
        peeing = false;
        cleaning = false;
      }
      actionDone = 2;
    }

    if (millis() - lastMotionTime > GetDelay() * 1000)                         // when no motion is sensed for a configurable delay
    {                                                                          // the airwick will fire or not depending on the activity
      if (peeing)
      {
        AirwickFire(1);
        LOGLN("FIRE BECAUSE PEEING");
        peeing = false;
        pooping = false;
      }                                            
      else if (pooping)
      {
        AirwickFire(2);
        LOGLN("FIRE TWICE BECAUSE POOPING");
        pooping = false;
      }
      else                                                                     // this is also when someone just walks in and out and doesn't interact with the sensors
      {
        NoFire();
        LOGLN("NO FIRE BECAUSE CLEANING");
        cleaning = false;
      }
      active = false;
    }
  }

  if (!settings && menuUpButton.GetDown()){
    MenuUp();
  }
  if (!settings && menu == 0 && menuConfirmButton.GetDown())
  {
    DefaultMenuUp();
  }
  
  if (!active && !interruptEnabled){
    EnableInterrupt();
  }
  if (!firing && !fireInterruptEnabled){
    EnableFireInterrupt();
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
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration / 68;
  LOG(F("Distance: "));
  LOGLN(distance);
  if(active)
  {
    queue.Enqueue(new Event(UpdateDistance, 2000));
  }
}

// Fires the airwick via an interrupt button and disables the interrupt for
// 20 seconsds
void AirwickFireInterrupt(){
  if(digitalRead(3) == LOW && fireInterruptEnabled){
    LOGLN(F("INTERRUPTFIRE"));
    detachInterrupt(fireButtonPin);
    AirwickFire(1);
    fireInterruptEnabled = false;
  }
}

// Enables the fire interrupt again
void EnableFireInterrupt(){
  attachInterrupt(fireButtonPin, AirwickFireInterrupt, FALLING);
  fireInterruptEnabled = true;
}

//Fires the airwick once
void AirwickFire(int times){
  if(!firing){
    for(int i = 0; i < times; i++){
      queue.Enqueue(new Event(FireRoutine, i * (AirwickFireTime + 3000)));
      queue.Enqueue(new Event(AirwickOff, i * (AirwickFireTime + 3000) + AirwickFireTime));
    }
  }
}

// The standard routine for firing
void FireRoutine(){
  LOGLN(F("Fire"));
  active = false;  
  firing = true;
  actionDone = 4;
  digitalWrite(airwick, HIGH);
  digitalWrite(fireStatePin, HIGH);
  queue.Enqueue(new Event(AirwickOff, AirwickFireTime));
}

// Disable variables to let the system know it's done
void NoFire(){
  active = false;
  firing = false;
  actionDone = 0;
}

//Disables the airwick
void AirwickOff(){
  int shots = GetShots();
  shots = shots - 1 < 0 ? 0 : shots - 1;
  WriteShots(shots);
  firing = false;
  digitalWrite(airwick, LOW);
  digitalWrite(fireStatePin, LOW);
  LOGLN(F("Reload"));
  actionDone = 0;
}

//Changes to the active status via the motion sensor
void InterruptRoutine(){
  active = true;
  detachInterrupt(interruptPin);
  LOGLN(F("interrupt"));
  queue.Enqueue(new Event(UpdateDistance, 0));
  queue.Enqueue(new Event(UpdateLight, 0));
  queue.Enqueue(new Event(CheckMagnet, 0));
  queue.Enqueue(new Event(CheckMotion, 0));
  lastMotionTime = millis();
  interruptEnabled = false;
  actionDone = 3;
}

//Enables enables the motion sensor interruupt
void EnableInterrupt(){
  light = MinLight;
  distance = MinDistance;
  interruptEnabled = true;
  EIFR = (1 << 0);                                                             // Clears the interrupt flag
  attachInterrupt(interruptPin, InterruptRoutine, FALLING);
}

//Gets the light from the light sensor
void UpdateLight(){
  light = analogRead(LightSensorPin);
  LOG(F("Light: "));
  LOGLN(light);

  if(active) queue.Enqueue(new Event(UpdateLight, 1000));
}

//Turns the beat led on or off
void UpdateBeat(){
  heartBeat = !heartBeat;
  digitalWrite(heartBeatPin, heartBeat);
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
    //LOGLN(F("Error in temp sensor"));
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

// Cycles through viewing temp en shots or action on the default menu
void DefaultMenuUp(){
  menuDefault = menuDefault + 1 > 1 ? 0 : menuDefault + 1;
}

// Prints the current menu to the lcd screen
void DisplayMenu(){

  queue.Enqueue(new Event(DisplayMenu, drawSpeed));
  lcd.setCursor(0, 0);
  if (menu == 1){
    previewSettingsMenu();
    drawSpeed = 50;
  }
  else if (menu == 2){
    settingsMenu();
    drawSpeed = 50;
  }
  else {
    defaultMenu(menuDefault);
    drawSpeed = 250;
  }
}

// The default menu template
void defaultMenu(int mode){
  if (mode == 0)
  {
    LOGMENU("Menu: ");
    LOGLNMENU(menu);
    LOGLNMENU("DEFAULT MENU");
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print("C         ");
    lcd.setCursor(0, 1);
    lcd.print("Shots: ");
    lcd.print(GetShots());
    if (GetShots() < 100)
    {
      lcd.print("  !!     ");
    }
    else
    {
      lcd.print("         ");
    }
  }
  else
  {
    if (actionDone == 1) lcd.print("Flushed         ");
    else if (actionDone == 2) lcd.print("Brush used      ");
    else if (actionDone == 3) lcd.print("Motion detected ");
    else if (actionDone == 4) lcd.print("Spray imminent  ");
    else lcd.print("                ");
    
    lcd.setCursor(0, 1);
    lcd.print("Action: ");
    if (peeing) lcd.print("peeing  ");
    else if (pooping) lcd.print("pooping ");
    else if (cleaning) lcd.print("cleaning");
    else lcd.print("        ");
    LOGLNMENU("DEFAULT MENU 2")
  }
}

// The menu before the settings template
void previewSettingsMenu(){
  if (!printed)
  {
    LOGMENU("Menu: ");
    LOGLNMENU(menu);
    LOGLNMENU("PREVIEW SETTINGS");
    lcd.print("Settings        ");
    lcd.setCursor(0, 1);
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
  int firstValue;
  int secondValue;
  if (shots <= 0)                                                              // No negative shots
  {
    firstValue = 0;
    secondValue = 0;
  }
  else
  {
    firstValue = shots >> 8;
    secondValue = (shots << 8) >> 8;
  }

  EEPROM.write(0, firstValue);
  EEPROM.write(1, secondValue);
}

// Prints the current settings menu to the lcd screen
void settingsMenu(){
  lcd.setCursor(0, 0);                                                          // Instead of clearing, text is just printed over the old text, this is faster
  if(menuItem == 0){                                                           // Cycling through the menu is done with the menuItem variable
    if(!printed){                                                              // Only print if it hasn't already been printed, to make sure it doesn't print over itself
      LOGLNMENU("DELAY");
      lcd.print("Change delay    ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      printed = true;
    }
    CheckTimer();                                                              // Check if there hasn't been activity in the menu, if not, return to default menu
    if (menuConfirmButton.GetDown())                                           // When confirm button is pressed, go into the selected settings panel
    {
      menuItem = 1;
      ResetMenuTimer();                                                        // Reset the timer for returning to default menu
    }
    else if (menuUpButton.GetDown()){                                          // When up button is pressed, go to the next setting
      menuItem = 2;
      ResetMenuTimer();
    }
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
      delay = delay + 5 > 60 ? 15 : delay + 5;                                 // Change delay with intervals of 5 and a minimum of 15 seconds
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
      lcd.setCursor(0, 1);
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
      WriteShots(2400);                                                        // On average a can of air freshener contains 2400 shots, so when it is replaced
      menuItem = 4;                                                                                                                   // the value can be reset
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
      lcd.setCursor(0, 1);
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
      lcd.setCursor(0, 1);
      lcd.print("                ");
      printed = true;
    }
    CheckTimer();
    if (menuConfirmButton.GetDown())
    {
      setDefaultDistance();                                                    // Set the default distance that is used to check if the brush is being used,
      menuItem = 7;                                                                                                        // this is measured automatically
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
      lcd.setCursor(0, 1);
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
  else if (menuItem == 8)
  {
    if (!printed)
    {
      LOGLNMENU("SET SPRAYS");
      lcd.print("Set number of   ");
      lcd.setCursor(0, 1);
      lcd.print("sprays          ");
      printed = true;
    }
    CheckTimer();
    if (menuUpButton.GetDown())
    {
      menuItem = 11;
      ResetMenuTimer();
    }
    if (menuConfirmButton.GetDown())
    {
      menuItem = 9;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 9)
  {
    if (!printed)
    {
      LOGLNMENU("CHOOSE NUMBER 1 OR 2");
      lcd.print(" Num 1 || Num 2 ");
      lcd.setCursor(0, 1);
      if (action == 1)
      {
        lcd.print("   ^            ");
      }
      else
      {
        lcd.print("            ^   ");
      }
      printed = true;
    }
    CheckTimer();
    if (menuUpButton.GetDown())
    {
      action = action + 1 > 2 ? 1 : action + 1;                                // action means doing a number 1 or number 2
      ResetMenuTimer();
    }
    if (menuConfirmButton.GetDown())
    {
      menuItem = 10;
      ResetMenuTimer();
    }
  }
  else if (menuItem == 10)
  {
    int sprays = GetNumberOfSprays(action);
    if (!printed)
    {
      LOGLNMENU("CHOOSING NEW AMOUNT SPRAYS");
      lcd.print("Number ");
      lcd.print(action);
      lcd.print("        ");
      printed = true;
      lcd.setCursor(0, 1);
      lcd.print(sprays);
      lcd.print(" sprays        ");
    }
    CheckTimer();
    if (menuUpButton.GetDown())
    {
      sprays = sprays + 1 > 5 ? 0 : sprays + 1;                                // Sprays can be set manually, ranging from 0 to 5. This maximum is chosen to
      SetNumberOfSprays(sprays, action);                                                                             // prevent over using the air freshener
      ResetMenuTimer();
    }
    else if (menuConfirmButton.GetDown())
    {
      menuItem = 8;
      ResetMenuTimer();
    }
  }
  else
  {
    if(!printed){
      LOGLNMENU("BACK");
      lcd.print("Back            ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      printed = true;
    }
    CheckTimer();
    if(menuConfirmButton.GetDown()){
      settings = false;                                                        // Go back to preview settings
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

// Check if there is motion
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

// Check if the toilet is flushed
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

// Set the number of sprays for a number 1 or number 2
void SetNumberOfSprays(int amount, int action){
  if (action == 1)
  {
    EEPROM.write(4, amount);
  }
  else
  {
    EEPROM.write(5, amount);
  }
}
// Gets number of spray for a number 1 or number 2
int GetNumberOfSprays(int action){
  if (action == 1)
  {
    return (int)EEPROM.read(4);
  }
  else
  {
    return (int)EEPROM.read(5);
  }
}