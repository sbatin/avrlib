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

#ifndef Encoder_h
#define Encoder_h

template<typename A, typename B>
struct Encoder {
private:
    typedef VirtualPort<A, B> Bus;

    volatile static uint8_t prevState;
    volatile static uint8_t prevValue;
    volatile static uint8_t currValue;

public:
    static void init() {
        Bus::pullUp(0xFF);
    }

    static void capture() {
        auto currState = Bus::template read<uint8_t>();

        switch (currState | prevState << 2) {
            case 0x2: case 0x4: case 0xB: case 0xD:
                currValue--;
                break;
            case 0x1: case 0x7: case 0x8: case 0xE:
                currValue++;
                break;
        }

        prevState = currState;
    }

    static inline uint8_t read() {
        return currValue >> 2;
    }

    static inline uint8_t readDiff() {
        auto curr = read();
        auto diff = curr - prevValue;
        prevValue = curr;
        return diff;
    }
};

template<typename A, typename B>
volatile uint8_t Encoder<A, B>::prevState = 0;

template<typename A, typename B>
volatile uint8_t Encoder<A, B>::prevValue = 0;

template<typename A, typename B>
volatile uint8_t Encoder<A, B>::currValue = 0;

#endif
