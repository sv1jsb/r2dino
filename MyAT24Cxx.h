#include <AT24Cxx.h>

class MyAT24Cxx: public AT24Cxx {
  public:
    MyAT24Cxx(uint8_t i2c_address): AT24Cxx(i2c_address){};
    MyAT24Cxx(uint8_t i2c_address, uint32_t eeprom_size): AT24Cxx(i2c_address, eeprom_size){};
    unsigned long readLong(uint16_t address);
    void writeLong(uint16_t address, unsigned long value);
};
