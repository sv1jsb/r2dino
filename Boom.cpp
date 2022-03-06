#include "Boom.h"

void Boom::Reset(){
  x = 0;
  y = 0;
  counter = 1;
  visible = false;
};

char Boom::graph[8] = {
  B100010,
  B000000,
  B010100,
  B101010,
  B010100,
  B000000,
  B100010,
  B000000
};
