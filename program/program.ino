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

#define airwick 13

#define trigPin 10
#define echoPin 11
#define LightSensorPin A0

#define tempPin 4

#define interruptPin digitalPinToInterrupt(2)

#define AirwickFireTime 21000
#define MinLight 600
#define MinDistance 10

#define fireStatePin A4
#define heartBeatPin A5

#define fireButtonPin A1 

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
bool settings = false;
bool active = false;
int distance = MinDistance;
long duration;
int light = MinLight;
float temperature;
bool poop;
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
  queue.Enqueue(new Event(UpdateBeat,0));
  queue.Enqueue(new Event(DisplayMenu,0));
  menuConfirmButton.Init(menuConfirmPin);
  menuUpButton.Init(menuUpPin);

  lcd.begin(16, 2);
}

void loop() {
  queue.PerformEvents();
  if(active && !settings){
    
    if(distance < MinDistance && light < MinLight){
      queue.Enqueue(new Event(AirwickFire,EEPROM.read(2) * 1000 - 10000));
      active = false;
    }
    if(digitalRead(fireButtonPin) == LOW){
      AirwickFire();
    }
    if(menuUpButton.GetDown()){
      MenuUp();
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
  digitalWrite(fireStatePin,HIGH);
  LOGLN(F("FIRE TWICE"));
  queue.Enqueue(new Event(AirwickOff,AirwickFireTime));
  queue.Enqueue(new Event(AirwickFire,AirwickFireTime + 1000));
}
//Fires the airwick once
void AirwickFire(){
  if(!firing){
    firing = true;
    digitalWrite(airwick,HIGH);
    digitalWrite(fireStatePin,HIGH);
    LOGLN(F("Fire"));
    queue.Enqueue(new Event(AirwickOff,AirwickFireTime));
  }

}

//Disables the airwick
void AirwickOff(){
  firing = false;
  digitalWrite(airwick,LOW);
  digitalWrite(fireStatePin,LOW);
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

void MenuUp(){
  menu = menu + 1 >= menuCount ? 0 : menu + 1;
}

void DisplayMenu(){
  queue.Enqueue(new Event(DisplayMenu,drawSpeed));
  lcd.setCursor(0,0);
  switch (menu)
  {
    case 0:
      defaultMenu();
      drawSpeed = 500;
      break;
    case 1:
      previewSettingsMenu();
      drawSpeed = 500;
      break;
    case 2:
      settingsMenu();
      drawSpeed = 50;
      break;
    default:
      lcd.print("NO MENU ");
      lcd.print(menu);
      LOGLN("OTHER MENU");
      break;
  }

}

void defaultMenu(){
  LOGLN("default menu");
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C      ");
  lcd.setCursor(0, 1);
  lcd.print("Shots: ");
  byte firstValue = EEPROM.read(0);
  byte secondValue = EEPROM.read(1);
  int value = (firstValue << 8) + secondValue;
  lcd.print(value);
  lcd.print("       ");
}

void previewSettingsMenu(){
  lcd.print("Settings       ");
  lcd.setCursor(0,1);
  lcd.print("               ");
  if(menuConfirmButton.GetDown()){
    menu = 2;
    settings = true;
    menuItem = 0;
  }
}

void settingsMenu(){
  lcd.setCursor(0,0);
  switch (menuItem)
  {
  case 0:
    LOGLN("DELAY");
    lcd.print("Change delay   ");
    lcd.setCursor(0,1);
    lcd.print("               ");
    if (menuConfirmButton.GetDown())
    {
      menuItem = 1;
    }
    if (menuUpButton.GetDown()){
      menuItem = 2;
    }
    break;
  case 1:
    LOGLN("NEWDELAY");
    lcd.print("New delay:     ");
    lcd.setCursor(0, 1);
    int delay = EEPROM.read(2);
    if (menuUpButton.GetDown())
    {
      delay = delay + 5 > 60 ? 15 : delay + 5;
      EEPROM.write(2, delay);
    }
    lcd.print(delay);
    lcd.print("s            ");

    if (menuConfirmButton.GetDown())
    {
      menuItem = 0;
    }
    break;
  case 2:
    LOGLN("RESET SPRAYS");
    lcd.print("Reset sprays   ");
    lcd.setCursor(0,1);
    lcd.print("               ");
    if (menuConfirmButton.GetDown())
    {
      menuItem = 3;
    }
    if (menuUpButton.GetDown()){
      menuItem = 4;
    }
    break;
  case 3:
    LOGLN("CONFIRM");
    lcd.print("Confirm?        ");
    lcd.setCursor(0,1);
    lcd.print("               ");
    if (menuConfirmButton.GetDown())
    {
      EEPROM.write(0, 5);
      EEPROM.write(1, 96);
    }
    if (menuUpButton.GetDown())
    {
      menuItem = 2;
    }
    break;
  default:
    LOGLN("BACK");
    lcd.print("Back           ");
    lcd.setCursor(0,1);
    lcd.print("               ");
    if(menuConfirmButton.GetDown()){
      settings = false;
      menu = 1;
    }
    if(menuUpButton.GetDown()){
      menuItem = 0;
    }
    break;
  }
  
}

