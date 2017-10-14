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

#ifndef TWI_H_
#define TWI_H_

#define TWCR_START (1 << TWSTA | 0 << TWSTO | 0 << TWEA)
#define TWCR_STOP  (0 << TWSTA | 1 << TWSTO | 0 << TWEA)
#define TWCR_ACK   (0 << TWSTA | 0 << TWSTO | 1 << TWEA)
#define TWCR_NACK  (0 << TWSTA | 0 << TWSTO | 0 << TWEA)
#define TWCR_CONTINUE   (TWCR_NACK  | 1 << TWINT | 1 << TWEN | 1 << TWIE)
#define TWCR_SEND_STOP  (TWCR_STOP  | 1 << TWINT | 1 << TWEN | 0 << TWIE)
#define TWCR_SEND_ACK   (TWCR_ACK   | 1 << TWINT | 1 << TWEN | 1 << TWIE)
#define TWCR_SEND_START (TWCR_START | 1 << TWINT | 1 << TWEN | 1 << TWIE)

#define TWI_BUFFER_SIZE     20
#define TWI_DIRECTION_WRITE 0
#define TWI_DIRECTION_READ  1
#define TWI_BUSY            1
#define TWI_MSG_RECEIVED    2

static uint8_t TWI_slaveAddress = 0;
static uint8_t TWI_txMsgSize = 0;
static uint8_t TWI_rxMsgSize = 0;

volatile static uint8_t TWI_writeBit = TWI_DIRECTION_WRITE;
volatile static uint8_t TWI_status = 0;
volatile static uint8_t TWI_bufPtr = 0;
volatile static uint8_t TWI_buf[TWI_BUFFER_SIZE];

ISR(TWI_vect) {
    switch (TWSR & 0xF8) { // get rid of prescaler bits
        case 0x08: // START has been transmitted
        case 0x10: // Repeated START has been transmitted
            TWI_status = TWI_BUSY;
            TWI_bufPtr = 0;
            TWDR = (TWI_slaveAddress << 1) | TWI_writeBit;
            TWCR = TWCR_CONTINUE;
            break;

        case 0x18: // SLA+W has been transmitted and ACK received
        case 0x28: // Data byte has been transmitted and ACK received
            if (TWI_bufPtr < TWI_txMsgSize) {
                TWDR = TWI_buf[TWI_bufPtr++];
                TWCR = TWCR_CONTINUE;
            } else {
                TWI_writeBit = TWI_DIRECTION_READ;
                TWCR = TWCR_SEND_START;
                //TWCR = TWCR_SEND_STOP;
                //TWI_status = 0;
            }
            break;

        case 0x50: // Data byte has been received and ACK transmitted
            TWI_buf[TWI_bufPtr++] = TWDR;
        case 0x40: // SLA+R has been transmitted and ACK received
            if (TWI_bufPtr < (TWI_rxMsgSize - 1)) {
                TWCR = TWCR_SEND_ACK;
            } else {
                TWCR = TWCR_CONTINUE;
            }
            break;

        case 0x58: // Data byte has been received and NACK transmitted
            TWI_buf[TWI_bufPtr] = TWDR;
            TWCR = TWCR_SEND_STOP;
            TWI_status = TWI_MSG_RECEIVED;
            break;

        case 0x38: // Arbitration lost
            TWCR = TWCR_SEND_START;
            break;

        case 0x00: // Bus error due to an illegal START or STOP condition
        case 0x20: // SLA+W has been transmitted and NACK received
        case 0x30: // Data byte has been transmitted and NACK received
        case 0x48: // SLA+R has been transmitted and NACK received
            TWCR = (1 << TWEN);
            TWI_status = 0;
    }
}

// blocking function to send message to a slave
// this function runs TWI transmission
void TWI_sendMessage(const uint8_t *msg, uint8_t msgSize) {
    while (TWI_status) {};

    memcpy(TWI_buf, msg, msgSize);
    TWI_txMsgSize = msgSize;
    TWI_writeBit = TWI_DIRECTION_WRITE;
    TWCR = TWCR_SEND_START;
}

// non-blocking read message from buffer (if received)
// should be used prior to TWI_sendMessage
uint8_t TWI_readMessage(uint8_t *dest, uint8_t msgSize) {
    if (TWI_status == TWI_MSG_RECEIVED) {
        memcpy(dest, TWI_buf, msgSize);
        TWI_rxMsgSize = msgSize;
        TWI_status = 0;
        return 1;
    }

    return 0;
}

void TWI_init(uint8_t slaveAddress, uint32_t baudRate) {
    TWI_slaveAddress = slaveAddress;

    // I2C speed = F_CPU/(16 + 2*TWBR*4^TWPS)
    uint32_t prescaler = F_CPU / (2 * baudRate) - 8;

    if (prescaler > 4080) {
        TWBR = prescaler / 64;
        TWSR |= (1 << TWPS1) | (1 << TWPS0);
    } else if (prescaler > 1020) {
        TWBR = prescaler / 16;
        TWSR |= (1 << TWPS1) | (0 << TWPS0);
    } else if (prescaler > 255) {
        TWBR = prescaler / 4;
        TWSR |= (0 << TWPS1) | (1 << TWPS0);
    } else {
        TWBR = prescaler;
        TWSR |= (0 << TWPS1) | (0 << TWPS0);
    }
}

#endif /* TWI_H_ */
