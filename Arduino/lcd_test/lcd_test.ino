#include "map.h"
#include "virtualport.h"
#include "encoder.h"
#include "LCD.h"

typedef LCD<16, 2, PortC5, PortC4, PortC3, PortC2, PortC1, PortC0> MyLcd;
typedef Encoder<PortD0, PortD1> Enc1;

void setup() {
  Enc1::init();
  MyLcd::init();
  MyLcd::moveTo(0, 0);
  MyLcd::print("NAV1 117.90");
  MyLcd::moveTo(0, 1);
  MyLcd::print("COM1 117.90");
}

void loop() {
  delay(1000);
}
