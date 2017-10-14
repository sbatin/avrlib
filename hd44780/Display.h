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

#ifndef LCD_h
#define LCD_h

// commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT     0x00
#define LCD_ENTRYLEFT      0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON      0x04
#define LCD_CURSORON       0x02
#define LCD_BLINKON        0x01

// flags for display/cursor shift
#define LCD_DISPLAYMOVE    0x08
#define LCD_CURSORMOVE     0x00
#define LCD_MOVERIGHT      0x04
#define LCD_MOVELEFT       0x00

// flags for function set
#define LCD_DL 4
#define LCD_N  3
#define LCD_F  2

#define LCD_8BITMODE  (1 << LCD_DL)
#define LCD_4BITMODE  (0 << LCD_DL)
#define LCD_2LINE     (1 << LCD_N)
#define LCD_1LINE     (0 << LCD_N)
#define LCD_5x10DOTS  (1 << LCD_F)
#define LCD_5x8DOTS   (0 << LCD_F)

template <uint8_t Cols, uint8_t Rows, typename RS, typename EN, typename D0, typename D1, typename D2, typename D3>
struct LCD {
private:
    typedef VirtualPort<D0, D1, D2, D3> Bus;

    static void writeToBus(uint8_t value) {
        Bus::write(value);
        EN::clr();
        _delay_us(1);
        EN::set();
        _delay_us(1);   // enable pulse must be >450ns
        EN::clr();
        _delay_us(100); // commands need > 37us to settle
    }
public:
    static void command(uint8_t value) {
        RS::clr();
        writeToBus(value >> 4);
        writeToBus(value);
    }

    static void write(char value) {
        RS::set();
        writeToBus(value >> 4);
        writeToBus(value);
    }

    static void print(const char *st) {
        size_t n = strlen(st);
        for (size_t i = 0; i < n; i++)
            write(*st++);
    }

    // this method clears display and sets cursor position to zero
    static void clear() {
        command(LCD_CLEARDISPLAY);
        _delay_us(2000);
    }

    // this method sets cursor position to zero
    static void home() {
        command(LCD_RETURNHOME);
        _delay_us(2000);
    }

    static void moveTo(uint8_t col, uint8_t row) {
        uint8_t offset = (row % 2) * 0x40 + (row > 1 ? Cols : 0);
        command(LCD_SETDDRAMADDR | (col + offset));
    }

    static void init() {
        // set D0-D3, RS, E pins as OUTPUT
        RS::setOut();
        EN::setOut();
        Bus::toOutput(0xFF);

        _delay_ms(50);
        // Now we pull both RS and R/W low to begin commands
        RS::clr();
        EN::clr();

        // we start in 8bit mode, try to set 4 bit mode
        writeToBus(0x03);
        _delay_us(4500); // wait min 4.1ms
        // second try
        writeToBus(0x03);
        _delay_us(4500); // wait min 4.1ms
        // third go!
        writeToBus(0x03);
        _delay_us(150);
        // finally, set to 4-bit interface
        writeToBus(0x02);

        // finally, set # lines, font size, etc.
        command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);

        // turn the display on with no cursor or blinking default
        command(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
        clear();

        // initialize to default text direction (for romance languages)
        command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    }
};

#endif
