#include "MyAT24Cxx.h"

unsigned long MyAT24Cxx::readLong(uint16_t address){
  long four = read(address);
  long three = read(address + 1);
  long two = read(address + 2);
  long one = read(address + 3);
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);  
}

void MyAT24Cxx::writeLong(uint16_t address, unsigned long value){
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  write(address, four);
  write(address + 1, three);
  write(address + 2, two);
  write(address + 3, one);
}
