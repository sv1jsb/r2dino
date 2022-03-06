#include "r2dino.h"

void setup() {
  analogReference(INTERNAL);
  pinMode(FirePin, INPUT_PULLUP);
  pinMode(Backlight, OUTPUT);
  digitalWrite(Backlight, LOW);
  randomSeed(analogRead(RandomSampling));
  serial.begin(9600);
  delay(1000);
  SetupLcd();
  state = INFO;
};

void SetupLcd(){
  CF_restore_display();
  CF_contrast(CONTRAST);  
  CF_backlight(BACKLIGHT);
  CF_hide_cursor();
  CF_scroll_off();
  /////////////// Setup custom chars  //////////////
  CF_custom_char(R2D2::lcd, R2D2::graph);
  CF_custom_char(TIE::lcd, TIE::graph);
  CF_custom_char(R2Laser::lcd, R2Laser::graph);
  CF_custom_char(TieLaser::lcd, TieLaser::graph);
  CF_custom_char(Boom::lcd, Boom::graph);
  CF_custom_char(bellIcon.lcd, bellIcon.graph);
};

void loop() {
  switch(state){
    case MENU:
      Menu();
      break;
    case INFO:
      Info();
      break;
    case CHOOSE_ALARM_OR_TIME:
      ChooseAlarmOrTime();
      break;
    case SETUP_ALARM:
      SetUpAlarm();
      break;
    case ALARM_FIRED:
      AlarmFired();
      break;
    case SETUP_TIME:
      SetUpTime();
      break;
    case TIME_TEMP_LOOP:
      TimeTempLoop();
      break;
    case RESET_GAME:
      ResetGame();
      break;
    case GAME_LOOP:
      GameLoop();
      break;
    case TIE_WON:
      Results("GAME OVER", false);
      break;
    case R2D2_WON:
      Results(" VICTORY", true);
      break;
    case SHOW_HIGH_SCORES:
      ShowHighScores();
      break;
  }
};

void Menu(){
  CF_backlight(BACKLIGHT);
  CF_cls();
  CF_print_at(7, 0, "R2-Dino");
  CF_print_at(0, 1, "1 Date Time Temp");
  CF_print_at(0, 2, "2 Setup alarm/time");
  CF_print_at(0, 3, "3 Game TIE Invaders");
  CF_pos(0,0);
  for(byte i = 0; i < 4; i++)
    serial.write(R2D2::lcd);
  CF_pos(16,0);
  for(byte i = 0; i < 4; i++)
    serial.write(R2D2::lcd);
  byte choice = 1;
  CF_inverted_cursor();
  do {
    delay(200);
    CF_pos(0, choice);
    choiceY(choice, 1 , 3);
  } while(digitalRead(FirePin));
  CF_hide_cursor();
  beep(NOTE_A5, OK_TONE_DURATION);
  delay(OK_TONE_DURATION * 2);
  switch(choice){
    case 1:
      state = INFO;
      break;
    case 2:
      state = CHOOSE_ALARM_OR_TIME;
      break;
    case 3:
      state = RESET_GAME;
      break;
  }
};

void Info(){
  ResetBacklight();
  CF_cls();
  if (! rtc.begin()) {
    serial.println("Couldn't find RTC");
    delay(3000);
    state = MENU;
    return;
  }
  if (! rtc.isrunning()) {
    state = SETUP_TIME;
    return;
  }
  CF_print_at(7, 0, "R2-Dino");
  CF_print_at(2, 1, "DATE:");
  byte _alarmEnabled = eeprom.read(ALARM_ENABLED_ADDRESS);
  if(_alarmEnabled == 255){
    alarmEnabled = false;
    _alarmEnabled = 0;
    eeprom.write(ALARM_ENABLED_ADDRESS, 0);
  } else {
    alarmEnabled = (bool)_alarmEnabled;
  }

  if(alarmEnabled){
    readAlarmFromEeprom();
    CF_pos(2,2);
    serial.write(bellIcon.lcd);
    
  }
  CF_print_at(3, 2, "TIME:");
  CF_print_at(4, 3, "TEMP:");
  for(int i=0;i<4;i++){
    CF_pos(0, i);
    serial.write(R2D2::lcd);
    CF_pos(19, i);
    serial.write(R2D2::lcd);
  }

  state = TIME_TEMP_LOOP;
};

void ResetBacklight(){
  CF_backlight(BACKLIGHT);
  isBacklightOn = true;
  backlightCounter = millis();
};

void GetAndUpdateDateTime(String &_date, String &_time){
  DateTime now = rtc.now();
  char format[] = "DD/MM/YYYY hh:mm:ss";
  s = String(now.toString(format));
  _date = s.substring(0, 10);
  _time = s.substring(11);  
}

void CheckForAlarm(String &_time){
  char _temp[ALARM_LENGTH + 1];
  _time.toCharArray(_temp, ALARM_LENGTH + 1);
  if(strncmp(_temp, alarm, 5) == 0){
    if(!alarmStopped){
      CF_restore_display();
      ResetBacklight();
      attachInterrupt(digitalPinToInterrupt(FirePin), alarmInterrupt, LOW);
      state = ALARM_FIRED;
    }
  } else if(alarmStopped){
    alarmStopped = false;
  }
}

void TimeTempLoop(){
  if(millis() % 1000 == 0) {  // Get readings and display evey second
    GetAndUpdateDateTime(_date, _time);
    tempC = (analogRead(TempReading) / 9.31) - 1.50;
    CF_print_at(8, 1, _date);
    CF_print_at(9, 2, _time);
    CF_print_at(10, 3, tempC);
    serial.print("c");
  }

  if(alarmEnabled){
    CheckForAlarm(_time);
  }

  if(!digitalRead(FirePin)){
    if(!isBacklightOn){
      ResetBacklight();
      delay(200);
    } else {
      beep(NOTE_A5, OK_TONE_DURATION);
      delay(OK_TONE_DURATION * 2);
      state = MENU;
    }
  }

  if(isBacklightOn && millis() >= backlightCounter + BACK_LIGHT_MILLIS){
    CF_backlight(BACKLIGHT_OFF);
    isBacklightOn = false;
    sleepCounter = millis();
  } 

  if(!isBacklightOn && millis() >= sleepCounter + SLEEP_MILLIS){
    CF_hide_display();
    delay(200);
    r2_fire = HIGH;
    do {
      attachInterrupt(digitalPinToInterrupt(FirePin), exitPowerDown, LOW);
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      detachInterrupt(digitalPinToInterrupt(FirePin));
      GetAndUpdateDateTime(_date, _time);
      if(alarmEnabled){
        CheckForAlarm(_time);
      }
      if(r2_fire == LOW){
        sleepCounter = millis();
        CF_restore_display();
      }
    } while(r2_fire != LOW && state == TIME_TEMP_LOOP);
  }
};

void ChooseAlarmOrTime(){
  CF_cls();
  CF_print_at(2, 0, "Setup alarm/time");
  CF_print_at(0, 1, "1 Alarm");
  CF_print_at(0, 2, "2 Date / time");
  CF_print_at(0, 3, "3 Back");
  byte choice = 1;
  CF_inverted_cursor();

  do {
    delay(200);
    CF_pos(0, choice);
    choiceY(choice, 1, 3);
  } while(digitalRead(FirePin));

  CF_hide_cursor();

  switch(choice){
    case 1:
      state = SETUP_ALARM;
      beep(NOTE_A5, OK_TONE_DURATION);
      delay(OK_TONE_DURATION * 2);
      break;
    case 2:
      state = SETUP_TIME;
      beep(NOTE_A5, OK_TONE_DURATION);
      delay(OK_TONE_DURATION * 2);
      break;
    case 3:
      state = MENU;
      beep(NOTE_C4, CANCEL_TONE_DURATION);
      delay(CANCEL_TONE_DURATION * 2);
      break;
  }
};

void SetUpAlarm(){
  int dateTime[6] = {01, 01, 2020, 00, 00, 00};
  int index = 3;
  byte column[5] = {0,0,0, 8, 11};
  CF_cls();
  CF_inverted_cursor();
  CF_print_at(4, 0, "Setup alarm");
  readAlarmFromEeprom();
  if(alarm[0] != 255){
    byte _inc = 3;
    for(byte i=0; i < ALARM_LENGTH; i+=3){
      char _temp[] = {alarm[i], alarm[i+1]};
      dateTime[_inc] = atoi(_temp);
      _inc++;
    }
  }
  do {
    CF_pos(7, 1);
    sprintf(alarm, "%02d:%02d", dateTime[3], dateTime[4]);
    serial.print(alarm);
    CF_pos(column[index], 1);
    GetDateTimeAndValidate(index, dateTime);
    choiceX(index, 3, 4);
    r2_fire = digitalRead(FirePin);
    delay(200);
  } while(r2_fire);

  CF_print_at(1, 2, "CANCEL");
  CF_print_at(9, 2, "UNSET");
  CF_print_at(16, 2, "SET");
  CF_pos(1,2);
  index = 0;
  byte columnSetCancel[3] = {1, 9, 16};
  do {
    CF_pos(columnSetCancel[index], 2);
    choiceX(index, 0, 2);
    r2_fire = digitalRead(FirePin);
    delay(200);
  } while(r2_fire);

  switch(index){
    case 0:
      beep(NOTE_C4, CANCEL_TONE_DURATION);
      delay(CANCEL_TONE_DURATION * 2);
      break;
    case 1:
      beep(NOTE_A5, OK_TONE_DURATION);
      delay(OK_TONE_DURATION * 2);
      setAlarmState(false);
      break;
    case 2:
      beep(NOTE_A5, OK_TONE_DURATION);
      delay(OK_TONE_DURATION * 2);
      setAlarmState(true);
      writeAlarmToEeprom();
      break;
  }
  state = MENU;
};

void alarmInterrupt(){
  alarmStopped = true;
  noTone(Buzzer);
}

void AlarmFired(){
  if(millis() % 3000 == 0){
    CF_cls();
    CF_print_at(7, 0, "Alarm");
    CF_print_at(6, 1, "Wake up");
    R2R2_speeak();
  }
  if(alarmStopped){
    detachInterrupt(digitalPinToInterrupt(FirePin)); 
    state = INFO;
    delay(200);
  }

};

void SetUpTime(){
  int dateTime[6] = {01, 01, 2020, 00, 00, 00};
  int index = 0;
  byte column[6] = {1, 4, 9, 12, 15, 18};
  char editLine[20];
  CF_cls();
  CF_inverted_cursor();
  if(!rtc.begin()) {
    ErrorSettingTime("RTC is not present");
    state = MENU;
    return;
  }
  if(rtc.isrunning()){
    DateTime _dt = rtc.now();
    dateTime[0] = _dt.day();
    dateTime[1] = _dt.month();
    dateTime[2] = _dt.year();
    dateTime[3] = _dt.hour();
    dateTime[4] = _dt.minute();
    dateTime[5] = _dt.second();
  }
  CF_print_at(3, 0, "Setup date/time");

  do {
    CF_pos(0, 1);
    sprintf(editLine, "%02d/%02d/%02d %02d:%02d:%02d",
            dateTime[0], dateTime[1], dateTime[2], dateTime[3], dateTime[4], dateTime[5]);
    serial.print(editLine);
    CF_pos(column[index], 1);
    GetDateTimeAndValidate(index, dateTime);
    choiceX(index, 0, 5);
    r2_fire = digitalRead(FirePin);
    delay(200);
  } while(r2_fire);

  CF_print_at(1, 2, "CANCEL");
  CF_print_at(11, 2, "SET");
  index = 0;
  byte columnSetCancel[2] = {1, 11};
  do {
    CF_pos(columnSetCancel[index], 2);
    choiceX(index, 0, 1);
    r2_fire = digitalRead(FirePin);
  } while(r2_fire);

  if(index == 0){
      beep(NOTE_C4, CANCEL_TONE_DURATION);
      delay(CANCEL_TONE_DURATION * 2);    
  } else {
    DateTime _datetime = DateTime(dateTime[2], dateTime[1], dateTime[0], dateTime[3], dateTime[4], dateTime[5]);
    if(_datetime.isValid()){
      CF_hide_cursor();
      beep(NOTE_A5, OK_TONE_DURATION);
      delay(OK_TONE_DURATION * 2);
      rtc.adjust(_datetime);
    } else {
      ErrorSettingTime("Wrong date/time!");
    }
  }
  state = MENU;
};

void ErrorSettingTime(String s){
  CF_hide_cursor();
  CF_print_at(2, 3, s);
  beep(NOTE_C4, CANCEL_TONE_DURATION);
  delay(2000);
};

void GetDateTimeAndValidate(int index, int dateTime[6]){
  int _joyY = map(analogRead(JoystickY), 0, 1023, 1, -1);
  switch(index){
    case 0:
      if(dateTime[index] + _joyY < 1){
        dateTime[index] = 31;
      } else if(dateTime[index] + _joyY > 31){
        dateTime[index] = 1;
      } else{
        dateTime[index] += _joyY;
      }
      break;
    case 1:
      if(dateTime[index] + _joyY < 1){
        dateTime[index] = 12;
      } else if(dateTime[index] + _joyY > 12){
        dateTime[index] = 1;
      } else{
        dateTime[index] += _joyY;
      }
      break;
    case 2:
      if(dateTime[index] + _joyY < 2000){
        dateTime[index] = 2099;
      } else if(dateTime[index] + _joyY > 2099){
        dateTime[index] = 2000;
      } else{
        dateTime[index] += _joyY;
      }
      break;
    case 3:
      if(dateTime[index] + _joyY < 0){
        dateTime[index] = 23;
      } else if(dateTime[index] + _joyY > 23){
        dateTime[index] = 0;
      } else{
        dateTime[index] += _joyY;
      }
      break;
    default:
      if(dateTime[index] + _joyY < 0){
        dateTime[index] = 59;
      } else if(dateTime[index] + _joyY > 59){
        dateTime[index] = 0;
      } else{
        dateTime[index] += _joyY;
      }
      break;
  }
};

void GameLoop(){
  MovePlayers();
  ResetScreen();
  UpdateScreen();
  DisplayScreen();
  counter++;
};

void UpdateScreen(){
  for(byte i = 0; i < Squadron::membersNum; i++){
    if(!squadron.members[i].destroyed)
      screen[squadron.members[i].y][squadron.members[i].x] = TIE::lcd;
  };
  if(r2d2.visible)
    screen[r2d2.y][r2d2.x] = R2D2::lcd;
  if(r2laser.visible)
    screen[r2laser.y][r2laser.x] = R2Laser::lcd;
  if(boom.visible)
    screen[boom.y][boom.x] = Boom::lcd;
  if(tielaser.visible)
    screen[tielaser.y][tielaser.x] = TieLaser::lcd;;  
  if(c3boom.visible)
    screen[c3boom.y][c3boom.x] = Boom::lcd;;  
};

void MovePlayers(){
  if(!squadron.won || !r2d2.won){  // Don't let TIE fighters or R2 to fire
    if(!r2laser.visible){          // if we have a winner
      r2_fire = !digitalRead(FirePin);
      if(r2_fire){
        r2laser.y = r2d2.y;
        r2laser.visible = true;        
      }
    }
    if(!squadron.fired){
      byte _x = 19;
      byte _y = 0;
      bool _b = false;
      squadron.randomFire(_x, _y, _b);
      if(_b){
        tielaser.x = _x;
        tielaser.y = _y;
        tielaser.visible = true;        
      }
    }
    if(counter % (LOOPS_NUM / 2) == 0) {  // Move R2 every 2 circles
      char _joy = map(analogRead(JoystickY), 0, 1023, 1, -1);
      if(r2d2.y + _joy < 0)
        r2d2.y = 0;
      else if(r2d2.y + _joy > 3)
        r2d2.y = 3;
      else
        r2d2.y += _joy;
    }
  }
  if(r2_fire){
    r2laser.x++;
    if(r2laser.x < 20) {
      if(screen[r2laser.y][r2laser.x] == TIE::lcd){
        squadron.destroy_at(r2laser.x, r2laser.y);
        if(!squadron.hasMore()){
          r2d2.won = true;
        }
        boom.x = r2laser.x;
        boom.y = r2laser.y;
        boom.visible = true;
        boom.counter = 1;
        r2laser.Reset();
      }
    } else {
      r2laser.Reset();
    }
  }
  if(boom.visible){
    if(boom.counter == 2)
      beep(NOTE_C4, BOOM_DURATION);
    if(boom.counter % BOOM_LOOPS == 0){  // Boom graphic is visible for 5 circles
      if(r2d2.won){
        gameScore = millis() - gameScore;
        state = R2D2_WON;
      } else {
        boom.Reset();
      }
    } else {
      boom.counter++;
    }
  }

  if(tielaser.visible){
    tielaser.x--;
    if(tielaser.x == 0 && screen[tielaser.y][tielaser.x] == R2D2::lcd) {
      r2d2.visible = false;
      squadron.won = true;
      c3boom.x = 0;
      c3boom.y = tielaser.y;
      c3boom.visible = true;
      c3boom.counter = 1;
      tielaser.Reset();
    } else if(tielaser.x == 0) {
      squadron.fired = false;
      tielaser.Reset();      
    }
  }

  if(c3boom.visible){
    if(c3boom.counter == 2){
      beep(NOTE_A4, BOOM_DURATION);
      beep(NOTE_G4, BOOM_DURATION);
      beep(NOTE_E4, BOOM_DURATION);
    }
    if(c3boom.counter % BOOM_LOOPS == 0){  // Boom graphic is visible for 5 circles
      if(squadron.won){
        gameScore = millis() - gameScore;
        state = TIE_WON;
      }
    } else {
      c3boom.counter++;
    }
  }
  
  if(!squadron.won)              // Dont't move TIE if they have won
    if(counter % LOOPS_NUM == 0)  // Move TIE fighters every 9 circles
      squadron.move_members();
};

void ResetScreen(){
  for(byte i=0;i<4;i++)
    for(byte j=0;j<20;j++)
      screen[i][j] = 32;
};

void DisplayScreen(){
  CF_pos(0, 0);
  for(byte i=0; i<4; i++){
    serial.write(screen[i], 20);
  }
};

void Results(String s, bool displayScore){
  CF_cls();
  CF_print_at(5, 0, s);
  if(displayScore){
    R2R2_speeak();
    CF_print_at(4, 1, "Score: ");
    serial.println(gameScore, DEC);
    if(IsHighScore(gameScore)){
      CF_print_at(3, 2, "New high score");
    }
  }

  CF_print_at(1, 3, "MENU");
  CF_print_at(7, 3, "PLAY");
  CF_print_at(13, 3, "SCORES");
  CF_pos(1,3);
  CF_inverted_cursor();
  int index = 0;
  byte columnSetCancel[3] = {1, 7, 13};

  do {
    CF_pos(columnSetCancel[index], 3);
    choiceX(index, 0, 2);
    r2_fire = digitalRead(FirePin);
    delay(200);
  } while(r2_fire);
  CF_hide_cursor();
  beep(NOTE_A5, OK_TONE_DURATION);
  delay(OK_TONE_DURATION * 2);
  switch(index){
    case 0:
      state = MENU;
      break;
    case 1:
      state = RESET_GAME;
      break;
    case 2:
      state = SHOW_HIGH_SCORES;
      break;
  }
};

bool IsHighScore(unsigned long score){
  readScoresFromEeprom();
  bool isHighScore = false;
  for(byte i=0; i< HIGH_SCORES; i++){
    if(score < highScores[i]){
      isHighScore = true;
      SaveNewHighScore(score, i);
      break;
    }
  }
  return isHighScore;
};

void SaveNewHighScore(unsigned long score, byte index){
  unsigned long prevValue = score;
  for(byte i = index; i< HIGH_SCORES; i++){
    unsigned long tmp = prevValue;
    prevValue = highScores[i];
    highScores[i] = tmp;
  }
  saveScoresToEeprom();
};

void ShowHighScores(){
  CF_cls();
  CF_print_at(5, 0, "HIGH SCORES");
  readScoresFromEeprom();
  for(byte i=0; i < HIGH_SCORES; i++){
    if(highScores[i] < 60000)
      CF_print_at(3, 1+i, highScores[i]);
  }

  CF_print_at(13, 1, "1 Back");
  CF_print_at(13, 2, "2 Clear");

  byte choice = 1;
  CF_inverted_cursor();

  do {
    delay(200);
    CF_pos(13, choice);
    choiceY(choice, 1, 2);
  } while(digitalRead(FirePin));

  CF_hide_cursor();

  switch(choice){
    case 1:
      beep(NOTE_A5, OK_TONE_DURATION);
      delay(OK_TONE_DURATION * 2);
      break;
    case 2:
      clearScroresFromEeprom();
      beep(NOTE_C4, CANCEL_TONE_DURATION);
      delay(CANCEL_TONE_DURATION * 2);
      
      break;
  }
  state = MENU;
};

void ResetGame(){
  GameIntro();
  squadron.set_members();
  counter = 1;
  r2_fire = 0;
  boom.Reset();
  c3boom.Reset();
  r2d2.Reset();
  r2laser.Reset();
  tielaser.Reset();
  gameScore = millis();
  state = GAME_LOOP;
};

void GameIntro(){
  CF_cls();
  CF_print_at(3, 1, "TIE Invaders");
  playMelody();
  CF_cls();
};

void R2R2_speeak(){
  phrase2();
  phrase1();
  phrase2();
  phrase3();
}
