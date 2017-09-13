#include "twilib.h"

uint8_t dataReceived[5];
uint8_t dataToSent[5];
uint8_t needUpdate = 1;

int main() {
    // init TWI
    sei();

    while (true) {
        if (TWI_readMessage(dataReceived, 5)) {
            // do with data in dataReceived
        }

        if (needUpdate) {
            dataToSent[0]++;
            TWI_sendMessage(dataToSent, 5);
        }

        TWI_work();
        _delay_ms(10);
    }
}
