#include "R2D2.h"

void R2D2::Reset(){
  x = 0;
  y = 1;
  visible = true;
  won = false;
};

char R2D2::graph[8] = {
  B000000,
  B011100,
  B111110,
  B101110,
  B111110,
  B111110,
  B010100,
  B010100
};
