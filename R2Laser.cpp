#include "R2Laser.h"

void R2Laser::Reset(){
  x = 0;
  y = 0;
  visible = false;
};

char R2Laser::graph[8] = {
  B000000,
  B000000,
  B000000,
  B111111,
  B111111,
  B000000,
  B000000,
  B000000
};
