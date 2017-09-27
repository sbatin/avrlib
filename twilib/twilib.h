#define TWI_DIRECTION_WRITE 0
#define TWI_DIRECTION_READ  1

#define TWCR_START (1 << TWSTA | 0 << TWSTO | 0 << TWEA)
#define TWCR_STOP  (0 << TWSTA | 1 << TWSTO | 0 << TWEA)
#define TWCR_ACK   (0 << TWSTA | 0 << TWSTO | 1 << TWEA)
#define TWCR_NACK  (0 << TWSTA | 0 << TWSTO | 0 << TWEA)
#define TWCR_MASTER_CONT  (TWCR_NACK  | 1 << TWINT | 1 << TWEN | 1 << TWIE)
#define TWCR_MASTER_STOP  (TWCR_STOP  | 1 << TWINT | 1 << TWEN | 0 << TWIE)
#define TWCR_MASTER_ACK   (TWCR_ACK   | 1 << TWINT | 1 << TWEN | 1 << TWIE)
#define TWCR_MASTER_START (TWCR_START | 1 << TWINT | 1 << TWEN | 1 << TWIE)

#define TWI_BUSY          1
#define TWI_LAST_TRANS_OK 2
#define TWI_MSG_RECEIVED  4

#define TWI_BUFFER_SIZE 20

uint8_t TWI_slaveAddress = 0x22;
uint8_t TWI_writeBit = TWI_DIRECTION_WRITE;
uint8_t TWI_status = 0;
uint8_t TWI_bufPtr = 0;
uint8_t TWI_msgSize = 0;
uint8_t TWI_buf[TWI_BUFFER_SIZE];
uint8_t TWI_inMsgSize = 0;
uint8_t TWI_inBuf[TWI_BUFFER_SIZE];
uint8_t TWI_outMsgSize = 0;
uint8_t TWI_outBuf[TWI_BUFFER_SIZE];

static inline void TWI_startWrite(uint8_t *msg, uint8_t msgSize) {
    TWI_msgSize = msgSize;
    TWI_writeBit = TWI_DIRECTION_WRITE;
    memcpy(TWI_buf, msg, msgSize);
    TWCR = TWCR_MASTER_START;
}

static inline void TWI_startRead(uint8_t msgSize) {
    TWI_msgSize = msgSize;
    TWI_writeBit = TWI_DIRECTION_READ;
    TWCR = TWCR_MASTER_START;
}

ISR(TWI_vect) {
    switch (TWSR) {
        case 0x08: // START has been transmitted
        case 0x10: // Repeated START has been transmitted
            TWI_status = TWI_BUSY;
            // send address + direction bit
            TWDR = TWI_slaveAddress << 1 | TWI_writeBit;
            TWCR = TWCR_MASTER_CONT;
            TWI_bufPtr = 0;
            break;

        case 0x18: // SLA+W has been tramsmitted and ACK received
        case 0x28: // Data byte has been tramsmitted and ACK received
            if (TWI_bufPtr < TWI_msgSize) {
                TWDR = TWI_buf[TWI_bufPtr++];
                TWCR = TWCR_MASTER_CONT;
            } else {
                //TWI_startRead();
                TWCR = TWCR_MASTER_STOP;
                TWI_status = TWI_LAST_TRANS_OK;
            }
            break;

        case 0x50: // Data byte has been received and ACK tramsmitted
            TWI_buf[TWI_bufPtr++] = TWDR;
        case 0x40: // SLA+R has been tramsmitted and ACK received
            if (TWI_bufPtr < (TWI_msgSize - 1)) {
                TWCR = TWCR_MASTER_ACK;
            } else {
                TWCR = TWCR_MASTER_CONT;
            }
            break;

        case 0x58: // Data byte has been received and NACK tramsmitted
            TWI_buf[TWI_bufPtr] = TWDR;
            TWCR = TWCR_MASTER_STOP;
            TWI_status = TWI_MSG_RECEIVED;
            break;

        case 0x38: // Arbitration lost
            TWCR = TWCR_MASTER_START;
            break;

        case 0x00: // Bus error due to an illegal START or STOP condition
        case 0x20: // SLA+W has been tramsmitted and NACK received
        case 0x30: // Data byte has been tramsmitted and NACK received
        case 0x48: // SLA+R has been tramsmitted and NACK received
            TWCR = 0 | (1 << TWEN);
            TWI_status = 0;
    }
}

void TWI_sendMessage(uint8_t *msg, uint8_t msgSize) {
    memcpy(TWI_inBuf, msg, msgSize);
    TWI_inMsgSize = msgSize;
}

uint8_t TWI_readMessage(uint8_t *dest, uint8_t msgSize) {
    if (msgSize != TWI_outMsgSize) return 0;

    memcpy(dest, TWI_outBuf, TWI_outMsgSize);
    TWI_outMsgSize = 0;
    return 1;
}

void TWI_work() {
    if (TWI_status & TWI_BUSY) return;

    if (TWI_inMsgSize) {
        TWI_startWrite(TWI_inBuf, TWI_inMsgSize);
        TWI_inMsgSize = 0;
    }

    if (TWI_status == TWI_MSG_RECEIVED) {
        memcpy(TWI_outBuf, TWI_buf, TWI_msgSize);
        TWI_outMsgSize = TWI_msgSize;
        TWI_status = 0;
    }
}

void TWI_initSlave(uint8_t address) {
    TWAR = (address << 1);
    TWCR = (1 << TWEN);
}

ISR(TWI_vect) {
    switch (TWSR) {
        // receiving data from master
        case 0x60: // Own SLA+W has been received ACK has been returned
        case 0x70: // Broadcast address has been received; ACK has been returned
            TWI_bufPtr = 0;
            TWCR = TWCR_MASTER_ACK;
            TWI_status = TWI_BUSY;
            break;

        case 0x80: // Previously addressed with own SLA+W; data has been received; ACK has been returned
        case 0x90: // Previously addressed with broadcast call; data has been received; ACK has been returned
            //TODO: check that we have enough space in buffer
            TWI_buf[TWI_bufPtr++] = TWDR;
            TWCR = TWCR_MASTER_ACK;
            break;

        case 0xA0: // A STOP condition or repeated START condition has been received while still addressed as Slave
            TWCR = TWCR_MASTER_ACK;
            TWI_status = TWI_MSG_RECEIVED;
            TWI_msgSize = TWI_bufPtr;
            break;

        // transmitting data to master
        case 0xA8: // Own SLA+R has been received; ACK has been returned
            TWI_bufPtr = 0;
        case 0xB8: // Data byte in TWDR has been transmitted; ACK has been received
            //TODO: send byte from TX buffer
            TWDR = TWI_buf[TWI_bufPtr++];
            TWCR = TWCR_MASTER_ACK;
            break;

        case 0xC0: // Data byte in TWDR has been transmitted; NOT ACK has been received
            //TODO: check that we transmitted all the message
            TWCR = TWCR_MASTER_ACK;
            break;

        case 0x88: // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
        case 0x98: // Previously addressed with broadcast call; data has been received; NOT ACK has been returned
        case 0xC8: // Last data byte in TWDR has been transmitted; ACK has been received
        case 0x00: // Bus error due to an illegal START or STOP condition
            TWCR = (1 << TWSTO) | (1 << TWINT);
            break;

        default:
            TWCR = TWCR_MASTER_ACK;
    }
}
