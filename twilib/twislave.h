//  Copyright (c) 2017 Sergey Batin. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef TWISlave_h
#define TWISlave_h

#include <string.h>

#define TWI_TWCR_ACK (1 << TWEA | 1 << TWINT | 1 << TWEN | 1 << TWIE)
#define TWI_BUFFER_SIZE 20

static volatile uint8_t TWI_bufPtr = 0;
static volatile uint8_t TWI_buf[TWI_BUFFER_SIZE];

static volatile uint8_t TWI_rxMsgSize = 0;
static volatile uint8_t TWI_rxBuf[TWI_BUFFER_SIZE];

static volatile uint8_t TWI_txMsgSize = 0;
static volatile uint8_t TWI_txBuf[TWI_BUFFER_SIZE];

ISR(TWI_vect) {
    switch (TWSR) {
        // receiving data from master
        case 0x60: // Own SLA+W has been received ACK has been returned
        case 0x70: // Broadcast address has been received; ACK has been returned
            TWI_bufPtr = 0;
            TWCR = TWI_TWCR_ACK;
            break;

        case 0x80: // Previously addressed with own SLA+W; data has been received; ACK has been returned
        case 0x90: // Previously addressed with broadcast call; data has been received; ACK has been returned
            TWI_buf[TWI_bufPtr++] = TWDR;
            TWCR = TWI_TWCR_ACK;
            break;

        case 0xA0: // A STOP condition or repeated START condition has been received while still addressed as Slave
            TWCR = TWI_TWCR_ACK;
            // we have received all the message from master
            // so we have to copy received data into RX buffer
            if (TWI_rxMsgSize == 0) {
                memcpy(TWI_rxBuf, TWI_buf, TWI_bufPtr);
                TWI_rxMsgSize = TWI_bufPtr;
            }
            if (TWI_txMsgSize > 0) {
                memcpy(TWI_buf, TWI_txBuf, TWI_txMsgSize);
                TWI_txMsgSize = 0;
            }
            break;

        // transmitting data to master
        case 0xA8: // Own SLA+R has been received; ACK has been returned
            TWI_bufPtr = 0;
        case 0xB8: // Data byte in TWDR has been transmitted; ACK has been received
            TWDR = TWI_buf[TWI_bufPtr++];
            TWCR = TWI_TWCR_ACK;
            break;

        case 0xC0: // Data byte in TWDR has been transmitted; NOT ACK has been received
            TWCR = TWI_TWCR_ACK;
            break;

        case 0x88: // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
        case 0x98: // Previously addressed with broadcast call; data has been received; NOT ACK has been returned
        case 0xC8: // Last data byte in TWDR has been transmitted; ACK has been received
        case 0x00: // Bus error due to an illegal START or STOP condition
            TWCR = (1 << TWSTO) | (1 << TWINT);
            break;

        default:
            TWCR = TWI_TWCR_ACK;
    }
}

void TWI_sendMessage(const uint8_t *msg, uint8_t msgSize) {
    while (TWI_txMsgSize) {}; // wait until TX buffer is ready to write
    memcpy(TWI_txBuf, msg, msgSize);
    TWI_txMsgSize = msgSize;
}

uint8_t TWI_readMessage(uint8_t *dest, uint8_t msgSize) {
    if (msgSize == TWI_rxMsgSize) {
        memcpy(dest, TWI_rxBuf, msgSize);
        TWI_rxMsgSize = 0;
        return 1;
    }

    return 0;
}

void TWI_initSlave(uint8_t address) {
    TWAR = (address << 1);
    TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
}

#endif
