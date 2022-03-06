#include "TieLaser.h"

void TieLaser::Reset(){
  x = 0;
  y = 0;
  visible = false;
};

char TieLaser::graph[8] = {
  B000000,
  B000000,
  B111110,
  B111110,
  B000000,
  B111110,
  B111110,
  B000000
};
