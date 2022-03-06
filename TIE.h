#ifndef TIE_h
#define TIE_h
#include "Arduino.h"
#define MEMBERSNUM 9

class TIE {
  public:
    static char graph[8];
    static const byte lcd = 129;
    byte x = 0;
    byte y = 0;
    bool destroyed = false;
};

class Squadron {
  private:
    bool down = true;
    void move_all_left();
    void move_all_down();
    void move_all_up();
  public:
    TIE members[MEMBERSNUM];
    static const byte membersNum = MEMBERSNUM;
    bool won = false;
    bool fired = false;
   
    void set_members();
    void move_members();
    void destroy_at(byte x, byte y);
    void randomFire(byte &_x, byte &_y, bool &_b);
    void Reset();
    bool hasMore();
};

#endif
