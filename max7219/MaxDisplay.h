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

#ifndef MaxDisplay_h
#define MaxDisplay_h

template <class DATA, class CLOCK>
struct ShiftOut {
    static void init() {
        DATA::setOut();
        CLOCK::setOut();
    }

    static void write(uint8_t data) {
        uint8_t mask = 0x80;

        while (mask)  {
            DATA::send(data & mask);
            CLOCK::set();
            CLOCK::clr();
            mask >>= 1;
        }
    }
};

template <class DI, class CK, class LD, uint8_t N>
struct MaxDisplay {
    private:
        typedef ShiftOut<DI, CK> Bus;

        static void sendToDevice(uint8_t addr, uint8_t opcode, uint8_t data) {
            LD::clr();
            for (uint8_t i = N; i > 0; i--) {
                Bus::write(addr == N - i ? opcode : 0);
                Bus::write(addr == N - i ? data : 0);
            }
            LD::set();
        }

    public:
        static void shutdown(uint8_t addr, bool b) {
            sendToDevice(addr, 0xC, b ? 0 : 1);
        }

        static void test(uint8_t addr, bool b) {
            sendToDevice(addr, 0xF, b ? 1 : 0);
        }

        static void intensity(uint8_t addr, uint8_t intensity) {
            sendToDevice(addr, 0xA, intensity);
        }

        static void scanLimit(uint8_t addr, uint8_t value) {
            sendToDevice(addr, 0xB, value);
        }

        static void decodeMode(uint8_t addr, uint8_t mode) {
            sendToDevice(addr, 0x9, mode);
        }

        static void clear(uint8_t addr) {
            for (uint8_t i = 0; i < 8; i++)
                sendToDevice(addr, i + 1, 0);
        }

        static void write(uint8_t addr, uint8_t digit, uint8_t value, bool dp) {
            sendToDevice(addr, digit + 1, value | (dp ? (1 << 7) : 0));
        }

        static void init(uint8_t decMode) {
            Bus::init();
            LD::setOut();
            LD::set();

            for (uint8_t i = 0; i < N; i++) {
                test(i, false);
                scanLimit(i, 7);
                decodeMode(i, decMode);
                clear(i);
                shutdown(i, false);
            }
        }
};

template <typename Display, uint8_t...>
struct MaxMapped {
    template <typename T>
    static void print(uint8_t addr, T value, uint8_t point) {}
};

template <typename Display, uint8_t P, uint8_t... Ps>
struct MaxMapped<Display, P, Ps...> {
    template <typename T>
    static void print(uint8_t addr, T value, uint8_t point) {
        Display::write(addr, P, value % 10, point == 0);
        MaxMapped<Display, Ps...>::print(addr, value / 10, point - 1);
    }
};

#endif
