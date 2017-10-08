#include "MaxDisplay.h"
#include "map.h"

/*
 * PB5: Pin 13
 * PB4: Pin 12
 * PB3: Pin 11
 */
typedef MaxDisplay<PortB5, PortB4, PortB3, 1> LD1;
typedef MaxMapped<LD1, 1, 2, 6, 0, 5> LM1;

void setup() {
    // decode mode B for all digits
    LD1::init(0xFF);
    LD1::intensity(0, 15);
    //LD1::write(0, 5, 0xF, true);
    LM1::print(0, 11234, 4);
    delay(5000);
}

uint16_t counter = 0;

void loop() {
    LM1::print(0, counter++, 1);
    delay(100);
}

