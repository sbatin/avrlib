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

#define MAKE_AVR_PORT(portName, dirName, pinName, className) \
    class className {\
        public:\
            typedef uint8_t DataT;\
            static DataT read() {\
                return pinName;\
            }\
            static void dirWrite(DataT value) {\
                dirName = value;\
            }\
            static void dirClear(DataT mask) {\
                dirName &= ~mask;\
            }\
            static void dirSet(DataT value) {\
                dirName |= value;\
            }\
            static void write(DataT value) {\
                portName = value;\
            }\
            static void clear(DataT mask) {\
                portName &= ~mask;\
            }\
            static void set(DataT value) {\
                portName |= value;\
            }\
            static void toggle(DataT value) {\
                portName ^= value;\
            }\
            static void clearSet(DataT mask, DataT value) {\
                portName = (portName & ~mask) | value;\
            }\
            enum { width = sizeof(DataT) * 8 };\
    };\
    typedef TPin<className, 0> className ## 0;\
    typedef TPin<className, 1> className ## 1;\
    typedef TPin<className, 2> className ## 2;\
    typedef TPin<className, 3> className ## 3;\
    typedef TPin<className, 4> className ## 4;\
    typedef TPin<className, 5> className ## 5;\
    typedef TPin<className, 6> className ## 6;\
    typedef TPin<className, 7> className ## 7;\

template<class PORT, int PinNumber>
class TPin {
    public:
        static void set() {
            PORT::set(1 << PinNumber);
        }

        static void clr() {
            PORT::clear(1 << PinNumber);
        }

        static void tgl() {
            PORT::toggle(1 << PinNumber);
        }

        static void setOut() {
            PORT::dirSet(1 << PinNumber);
        }

        static void pullUp() {
            PORT::dirClear(1 << PinNumber);
            PORT::set(1 << PinNumber);
        }

        template<typename T>
        static void send(const T value) {
            if (value) set();
            else clr();
        }

        typedef PORT Port;
        enum { number = PinNumber };
};

//MAKE_AVR_PORT(PORTA, DDRA, PINA, PortA)
MAKE_AVR_PORT(PORTB, DDRB, PINB, PortB)
MAKE_AVR_PORT(PORTC, DDRC, PINC, PortC)
MAKE_AVR_PORT(PORTD, DDRD, PIND, PortD)

