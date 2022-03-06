#include "TIE.h"

char TIE::graph[8] = {
  B000000,
  B100010,
  B100010,
  B101010,
  B111110,
  B101010,
  B100010,
  B100010
};

void Squadron::move_all_left(){
  for(int i=MEMBERSNUM-1;i>=0;i--)
    if(!members[i].destroyed)
      members[i].x--;
};

void Squadron::move_all_down(){
  for(int i=MEMBERSNUM-1;i>=0;i--)
    if(!members[i].destroyed)
      members[i].y++;
};

void Squadron::move_all_up(){
  for(int i=0;i<MEMBERSNUM;i++)
    if(!members[i].destroyed)
      members[i].y--;
};

void Squadron::set_members(){
  // Reset attrs
  fired = false;
  won = false;
  down = true;
  // Add TIE fighters to members
  byte line = 0;
  byte column = 19;
  for(byte i=0;i<MEMBERSNUM;i++){
    members[i] = TIE();
    members[i].y = line % 3;
    members[i].x = column;
    line++;
    if(line % 3 == 0)
      column--;
  }
};

void Squadron::move_members(){
  if(down){
    bool over = false;
    for(byte i=0;i<MEMBERSNUM;i++){
      if(!members[i].destroyed && members[i].y == 3){
        over = true;
        break;
      }
    }
    if(over){
      move_all_left();
      down = false;
    } else {
      move_all_down();
    }
  } else {
    bool over = false;
    for(byte i=0;i<MEMBERSNUM;i++){
      if(!members[i].destroyed && members[i].y == 0){
        over = true;
        break;
      }
    }
    if(over){
      move_all_left();
      down = true;
    } else {
      move_all_up();
    }        
  }
};

void Squadron::destroy_at(byte x, byte y){
  for(byte i=0;i<MEMBERSNUM;i++){
    if(members[i].y == y && members[i].x == x){
      members[i].destroyed = true;
      break;
    }
  }
};

void Squadron::randomFire(byte &_x, byte &_y, bool &_b){
  _y = (byte)random(4);
  for(byte i=0;i<MEMBERSNUM;i++){
    if(!members[i].destroyed && members[i].y == _y && members[i].x < _x){
      _x = members[i].x;
    }
  }
  if(_x < 19){
    _b = true;
    fired = true;
  }
};

void Squadron::Reset(){
  fired = false;
  won = false;
};

bool Squadron::hasMore(){
  bool has_more = false;
  for(byte i=0;i<MEMBERSNUM;i++){
    if(!members[i].destroyed){
        has_more = true;
        break;
    }
  }
  return has_more;
};
