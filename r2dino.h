#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>
#include <SoftwareSerial.h>

#include "MyAT24Cxx.h"
#include "R2D2.h"
#include "TIE.h"
#include "R2Laser.h"
#include "Boom.h"
#include "TieLaser.h"

#define LOOPS_NUM             9
#define BOOM_LOOPS            5
#define BOOM_DURATION         90
#define OK_TONE_DURATION      90
#define CANCEL_TONE_DURATION  110
#define CONTRAST              54
#define BACKLIGHT_MAX         100
#define BACKLIGHT             32
#define BACKLIGHT_OFF         0
#define BACK_LIGHT_MILLIS     30000
#define SLEEP_MILLIS          60000

#define EEPROM_ADDRESS        0x50
#define ALARM_ENABLED_ADDRESS 0
#define ALARM_ADDRESS         1
#define ALARM_LENGTH          5
#define HIGH_SCORES           3
#define HIGH1_ADDRESS         6
#define HIGH2_ADDRESS         10
#define HIGH3_ADDRESS         14

// Notes
#define NOTE_C4               261
#define NOTE_D4               294
#define NOTE_E4               329
#define NOTE_F4               349
#define NOTE_G4               391
#define NOTE_A4               440
#define NOTE_C5               523
#define NOTE_A5               880

// Digital pins configuration
const byte FirePin = 3;
const byte Buzzer = 10;
const byte Backlight = 13;
// Analog pins configuration
const byte JoystickX = 1;
const byte JoystickY = 0;
const byte TempReading = 2;
const byte RandomSampling = 3;

// Variables
unsigned long backlightCounter;
unsigned long sleepCounter;
unsigned long gameScore;
unsigned long highScores[HIGH_SCORES];
uint16_t highScoreAddresses[HIGH_SCORES] = {HIGH1_ADDRESS, HIGH2_ADDRESS, HIGH3_ADDRESS};
byte counter;
volatile byte r2_fire = 0;
byte screen[4][20] = {
  {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32}
};
float tempC;
String s;
String _date;
String _time;
bool alarmEnabled = false;
bool isBacklightOn = true;
volatile bool alarmStopped = false;
char alarm[6];

enum State {MENU, INFO, CHOOSE_ALARM_OR_TIME, SETUP_TIME, SETUP_ALARM, ALARM_FIRED, TIME_TEMP_LOOP, RESET_GAME, GAME_LOOP, TIE_WON, R2D2_WON, SHOW_HIGH_SCORES};
State state;

// RTC object
RTC_DS1307 rtc;

// EEPROM object
MyAT24Cxx eeprom(EEPROM_ADDRESS, 32);

//SoftwareSerial ports
SoftwareSerial serial(12,11);

//  Main game objects
R2D2 r2d2;
R2Laser r2laser;
TieLaser tielaser;
Boom boom;
Boom c3boom;
Squadron squadron;

// Non game icons
struct Icon {
  byte lcd;
  char graph[8];
};

Icon bellIcon = {
  133,
  {
    B001000,
    B011100,
    B011100,
    B011100,
    B111110,
    B111110,
    B001000,
    B000000
  }
};

// Utility functions
void choiceY(byte &choice, byte lowBoundary, byte highBoundary){
  char _joy = map(analogRead(JoystickY), 0, 1023, 1, -1);
  if(choice + _joy < lowBoundary)
    choice = lowBoundary;
  else if(choice + _joy > highBoundary)
    choice = highBoundary;
  else
    choice += _joy;
}

void choiceX(int &choice, int lowBoundary, int highBoundary){
  int _joyX = map(analogRead(JoystickX), 0, 1023, -1, 1);
  if(choice + _joyX < lowBoundary)
    choice = lowBoundary;
  else if(choice + _joyX > highBoundary)
    choice = highBoundary;
  else
    choice += _joyX; 
}

void readAlarmFromEeprom(){
  for(byte i=0; i < ALARM_LENGTH; i++)
    alarm[i] = eeprom.read(ALARM_ADDRESS + i);
}

void writeAlarmToEeprom(){
  for(byte i=0; i < ALARM_LENGTH; i++)
    eeprom.write(ALARM_ADDRESS + i, alarm[i]);
}

void setAlarmState(bool active){
  alarmEnabled = active;
  eeprom.write(ALARM_ENABLED_ADDRESS, active);
}

void readScoresFromEeprom(){
  for(byte i=0; i< HIGH_SCORES; i++)
    highScores[i] = eeprom.readLong(highScoreAddresses[i]);
}

void saveScoresToEeprom(){
  for(byte i =0; i < HIGH_SCORES; i++)
    eeprom.writeLong(highScoreAddresses[i], highScores[i]);
}

void clearScroresFromEeprom(){
  for(byte i =0; i < HIGH_SCORES; i++)
    eeprom.writeLong(highScoreAddresses[i], 99999L);  
}

void beep(int note, int duration){
  tone(Buzzer, note, duration);
  delay(duration);
  noTone(Buzzer);
}

void playMelody(){
  beep(NOTE_A4, 500);
  beep(NOTE_A4, 500);    
  beep(NOTE_A4, 500);
  beep(NOTE_F4, 350);
  beep(NOTE_C5, 150);  
  beep(NOTE_A4, 500);
  beep(NOTE_F4, 350);
  beep(NOTE_C5, 150);
  beep(NOTE_A4, 650);  
}

void phrase1() {
  int k = random(1000,2000);
  int r1 = 301;
  int r2 = 700;
  if(!alarmStopped){
    digitalWrite(Backlight, HIGH);
    for (int i = 0; i <=  r1; i++){
        tone(Buzzer, k+(-i*2));          
        delay(random(.9, 2));
    } 
    digitalWrite(Backlight, LOW);   
    for (int i = 0; i <= r2; i++){
        tone(Buzzer, k + (i * 10));          
        delay(random(.9, 2));
    }
  }
}

void phrase2() {
  int k = random(1000,2000);
  int r1 = random(100,2000);
  int r2 = random(100,1000);
  if(!alarmStopped){
    digitalWrite(Backlight, HIGH);
    for (int i = 0; i <= r1; i++){
        tone(Buzzer, k+(i*2));          
        delay(random(.9,2));
    } 
    digitalWrite(Backlight, LOW);   
    for (int i = 0; i <= r2; i++){
        tone(Buzzer, k + (-i * 10));          
        delay(random(.9,2));
    } 
  }
}

void phrase3() {
  int K = 2000;
  int r1 = random(3, 9);
  int r2 = random(300, 900);
  if(!alarmStopped){
    for (int i = 0; i <= r1; i++){
      digitalWrite(Backlight, HIGH);
      tone(Buzzer, K - r2);
      delay(random(70, 170));
      digitalWrite(Backlight, LOW);
      noTone(Buzzer);
      delay(random(0, 30));    
    }
  }
}

void exitPowerDown(){
  r2_fire = LOW;
}

void CF_cls(){
  serial.write(12);
};

void CF_pos(int column, int row){
  serial.write(17);
  serial.write(column);
  serial.write(row);
};

void CF_print_at(int column, int row, String s){
  CF_pos(column, row);
  serial.print(s);
};

void CF_print_at(int column, int row, float f){
  CF_pos(column, row);
  serial.print(f);
};

void CF_print_at(int column, int row, unsigned long l){
  CF_pos(column, row);
  serial.print(l, DEC);  
};

void CF_contrast(int value){
  serial.write(15);
  serial.write(value);
};

void CF_backlight(int value){
  serial.write(14);
  serial.write(value);
};

void CF_hide_cursor(){
  serial.write(4);
};

void CF_underline_cursor(){
  serial.write(5);
};

void CF_block_cursor(){
  serial.write(6);
};

void CF_inverted_cursor(){
  serial.write(7);
};

void CF_scroll_off(){
  serial.write(20);
};

void CF_scroll_on(){
  serial.write(19);
};

void CF_wrap_off(){
  serial.write(24);
};

void CF_wrap_on(){
  serial.write(23);
};

void CF_custom_char(int charNum, char *buf){
  serial.write(25);
  serial.write(charNum - 128);
  serial.write(buf, 8);
};

void CF_hide_display(){
  serial.write(2);
};

void CF_restore_display(){
  serial.write(3);
};
