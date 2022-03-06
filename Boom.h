#ifndef Boom_h
#define Boom_h

#include "Arduino.h"

class Boom {
  public:
    static char graph[8];
    static const byte lcd = 132;
    byte x = 0;
    byte y = 0;
    byte counter = 1;
    bool visible = false;
    void Reset();
};
#endif
