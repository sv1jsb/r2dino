#ifndef TieLaser_h
#define TieLaser_h

#include "Arduino.h"

class TieLaser {
  public:
    static char graph[8];
    static const byte lcd = 131;
    byte x = 0;
    byte y = 0;
    bool visible = false;
    void Reset();
};

#endif
