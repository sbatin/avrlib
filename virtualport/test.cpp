#include <stdio.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

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

#include "map.h"
#include "virtualport.h"

//typedef VirtualPort<PortA1, PortA2, PortA3, PortB3, PortB4> PortX;
//typedef VirtualPort<PortA2, PortA1, PortA3, PortB3, PortB4> PortX;
//typedef VirtualPort<PortA0, PortA1, PortB0, PortB1, PortA2, PortA3, PortC0, PortC1> PortX;
typedef VirtualPort<PortA1, PortA2, PortA3, PortB3, PortB4, PortD0, PortD1, PortD3, PortC0, PortC1, PortC2, PortC3, PortC4, PortC5, PortC6, PortC7> PortX;

typedef TList<PinWrapper<PortA0, 0>, PinWrapper<PortA3, 1> > TestPins;

int main() {
    PortX::asOutput<uint16_t>();
    PortX::write(0x1255);
    printf("PortX=0x%X\n", PortX::read<uint16_t>());
    printf("PORTA=0x%X, PORTB=0x%X, PORTC=0x%X, PORTD=0x%X\n", PORTA, PORTB, PORTC, PORTD);
    printf("DDRA=0x%X, DDRB=0x%X, DDRC=0x%X, DDRD=0x%X\n", DDRA, DDRB, DDRC, DDRD);
    printf("bitmask = %d\n", BitMask<TestPins>::value);
    printf("length = %d\n", Length<TestPins>::value);
    printf("done\n");
    return 0;
}
