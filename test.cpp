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

#include <stdio.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

uint8_t PORTA = 0;
uint8_t PORTB = 0;
uint8_t PORTC = 0;
uint8_t PORTD = 0;
uint8_t DDRA = 0;
uint8_t DDRB = 0;
uint8_t DDRC = 0;
uint8_t DDRD = 0;
uint8_t PINA = 0;
uint8_t PINB = 0;
uint8_t PINC = 0;
uint8_t PIND = 0;

void sync() {
    PINA = PORTA;
    PINB = PORTB;
    PINC = PORTC;
    PIND = PORTD;
}

#include "virtualport/map.h"
#include "virtualport/virtualport.h"
#include "encoder/encoder.h"

MAKE_AVR_PORT(PORTA, DDRA, PINA, PortA)

//typedef VirtualPort<PortA1, PortA2, PortA3, PortB3, PortB4> PortX;
//typedef VirtualPort<PortA2, PortA1, PortA3, PortB3, PortB4> PortX;
//typedef VirtualPort<PortA0, PortA1, PortB0, PortB1, PortA2, PortA3, PortC0, PortC1> PortX;
typedef VirtualPort<PortA1, PortA2, PortA3, PortB3, PortB4, PortD0, PortD1, PortD3, PortC0, PortC1, PortC2, PortC3, PortC4, PortC5, PortC6, PortC7> PortX;

typedef TList<PinWrapper<PortA0, 0>, PinWrapper<PortA3, 1> > TestPins;
typedef Encoder<PortA5, PortA6> Enc1;

int main() {
    Enc1::init();
    Enc1::capture();
    PortX::toOutput(0xFFFF);
    PortX::write(0x1255);
    sync();
    printf("PortX=0x%X\n", PortX::read<uint16_t>());
    printf("PORTA=0x%X, PORTB=0x%X, PORTC=0x%X, PORTD=0x%X\n", PORTA, PORTB, PORTC, PORTD);
    printf("DDRA=0x%X, DDRB=0x%X, DDRC=0x%X, DDRD=0x%X\n", DDRA, DDRB, DDRC, DDRD);
    printf("done\n");
    return 0;
}
