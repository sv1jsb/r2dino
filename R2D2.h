#ifndef R2D2_h
#define R2D2_h

#include "Arduino.h"

class R2D2
{
  public:
    static char graph[8];
    static const byte lcd = 128;
    byte x = 0;
    byte y = 1;
    bool visible = false;
    bool won = false;
    void Reset();
};

#endif
