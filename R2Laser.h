#ifndef R2Laser_h
#define R2Laser_h

#include "Arduino.h"

class R2Laser {
  public:
    static char graph[8];
    static const byte lcd = 130;
    byte x = 0;
    byte y = 0;
    bool visible = false;
    void Reset();
};

#endif
