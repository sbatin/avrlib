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

#ifndef ILIDRIVER_H_
#define ILIDRIVER_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>

// VGA color palette
#define VGA_BLACK	0x0000
#define VGA_WHITE	0xFFFF
#define VGA_RED		0xF800
#define VGA_GREEN	0x0400
#define VGA_BLUE	0x001F
#define VGA_SILVER	0xC618
#define VGA_GRAY	0x8410
#define VGA_MAROON	0x8000
#define VGA_YELLOW	0xFFE0
#define VGA_OLIVE	0x8400
#define VGA_LIME	0x07E0
#define VGA_AQUA	0x07FF
#define VGA_TEAL	0x0410
#define VGA_NAVY	0x0010
#define VGA_FUCHSIA	0xF81F
#define VGA_PURPLE	0x8010

#define fontbyte(x) pgm_read_byte(&cfont.font[x])
#define from_rgb(r, g, b) ((r & 248) << 8 | (g & 252) << 3 | (b & 248) >> 3)

typedef uint16_t color_t;

struct font_t {
	uint8_t* font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
};

template <typename RS, typename WR, typename CS, typename RST, typename PortHi, typename PortLo, uint16_t W, uint16_t H>
class IliDriver {
public:
	IliDriver(color_t fontColor, color_t backColor) {
		PortHi::dirSet(0xFF);
		PortLo::dirSet(0xFF);
		RS::setOut();
		WR::setOut();
		CS::setOut();
		RST::setOut();
		
		setColor(fontColor);
		setBackColor(backColor);
		cfont.font = 0;
	}
	
	void InitLCD() {
		RST::set();
		_delay_ms(5);
		RST::clr();
		_delay_ms(15);
		RST::set();
		_delay_ms(15);

		CS::clr();

		// exit sleep mode
		writeCom(0x11);
		_delay_ms(20);

		// power setting
		writeCom(0xD0);
		writeData(0x07);
		writeData(0x42);
		writeData(0x18);

		// VCOM control
		writeCom(0xD1);
		writeData(0x00);
		writeData(0x07);
		writeData(0x10);

		// power setting for normal mode
		writeCom(0xD2);
		writeData(0x01);
		writeData(0x02);

		// panel driving setting
		writeCom(0xC0);
		writeData(0x10);
		writeData(0x3B);
		writeData(0x00);
		writeData(0x02);
		writeData(0x11);

		// frame rate and inversion control
		// set 72Hz frame rate
		writeCom(0xC5);
		writeData(0x00);

		// gamma setting
		writeCom(0xC8);
		writeData(0x00);
		writeData(0x32);
		writeData(0x36);
		writeData(0x45);
		writeData(0x06);
		writeData(0x16);
		writeData(0x37);
		writeData(0x75);
		writeData(0x77);
		writeData(0x54);
		writeData(0x0C);
		writeData(0x00);

		// set address mode
		writeCom(0x36);
		writeData(0x0A);

		// set pixel format
		writeCom(0x3A);
		writeData(0x55);

		// set column address
		setColumnAddress(0, W - 1);
		setPageAddress(0, H - 1);
		_delay_ms(120);

		// set display on
		writeCom(0x29);
		CS::set();
	}
	
	void clrScr() {
		fillScr(VGA_BLACK);
	}
	
	void fillScr(uint8_t r, uint8_t g, uint8_t b) {
		color_t color = from_rgb(r, g, b);
		fillScr(color);
	}

	void fillScr(color_t color) {
		CS::clr();
		clrXY();
		fastFill16(color, (uint32_t)W * (uint32_t)H);
		CS::set();
	}
	
	void fillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color) {
		CS::clr();
		setXY(x1, y1, x2, y2);
		fastFill16(color, (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1));
		CS::set();
	}
	
	void setColor(uint8_t r, uint8_t g, uint8_t b) {
		fore_color = from_rgb(r, g, b);
	}

	void setColor(color_t color) {
		fore_color = color;
	}

	void setBackColor(uint8_t r, uint8_t g, uint8_t b) {
		back_color = from_rgb(r, g, b);
	}

	void setBackColor(color_t color) {
		back_color = color;
	}
	
	void drawPixel(uint16_t x, uint16_t y) {
		CS::clr();
		setXY(x, y, x, y);
		setPixel(fore_color);
		CS::set();
		clrXY();
	}

	void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
		if (y1 == y2)
			drawHLine(x1, y1, x2 - x1);
		else if (x1 == x2)
			drawVLine(x1, y1, y2 - y1);
	}
	
	void printChar(char c, uint16_t x, uint16_t y) {
		uint8_t i, ch;

		CS::clr();
		setXY(x, y, x + cfont.x_size - 1, y + cfont.y_size - 1);

		uint16_t n = cfont.x_size * cfont.y_size / 8;
		uint32_t pos = 4 + (c - cfont.offset) * n;

		for (uint16_t j = 0; j < n; j++) {
			ch = fontbyte(pos);

			for (i = 0; i < 8; i++) {
				if ((ch & (1 << (7 - i))) != 0) {
					setPixel(fore_color);
				} else {
					setPixel(back_color);
				}
			}
			pos++;
		}

		CS::set();
		clrXY();
	}

	void print(const char *st, uint16_t x, uint16_t y) {
		size_t stl, i;

		stl = strlen(st);

		for (i = 0; i < stl; i++) {
			printChar(*st++, x + i*(cfont.x_size), y);
		}
	}
	
	void printNumber(uint32_t value, uint8_t numDigits, uint8_t pointPos, uint16_t x, uint16_t y) {
		const uint8_t pointSize = cfont.x_size - 8;
		
		printChar('.', x + pointPos * cfont.x_size - 4, y);
		x+= cfont.x_size * numDigits + pointSize;

		while (numDigits) {
			if (numDigits == pointPos) {
				x-= pointSize;
			}

			char ch = value % 10;
			x-= cfont.x_size;
			printChar(ch + '0', x, y);
			value = value / 10;
			numDigits--;
		}
	}

	void setFont(uint8_t* font) {
		cfont.font = font;
		cfont.x_size = fontbyte(0);
		cfont.y_size = fontbyte(1);
		cfont.offset = fontbyte(2);
		cfont.numchars = fontbyte(3);
	}

private:
	color_t fore_color, back_color;
	font_t cfont;

	static inline void writeBus(uint8_t high, uint8_t low) {
		PortHi::write(high);
		PortLo::write(low);
		WR::pulseLow();
	}
	
	static inline void writeCom(uint8_t low) {
		RS::clr();
		writeBus(0x00, low);
	}

	static inline void writeData(uint8_t high, uint8_t low) {
		RS::set();
		writeBus(high, low);
	}

	static inline void writeData(uint8_t low) {
		RS::set();
		writeBus(0x00, low);
	}

	static inline void fastFill16(color_t color, uint32_t pix) {
		uint16_t m = pix / 16;
		uint8_t k = pix % 16;

		RS::set();
		PortHi::write(color >> 8);
		PortLo::write(color & 0xFF);

		while (m--) {
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
			WR::pulseLow();
		}
		
		while (k--) {
			WR::pulseLow();
		}
	}
	
	static inline void setColumnAddress(uint16_t x1, uint16_t x2) {
		writeCom(0x2A);
		writeData(x1 >> 8);
		writeData(x1);
		writeData(x2 >> 8);
		writeData(x2);
	}

	static inline void setPageAddress(uint16_t y1, uint16_t y2) {
		writeCom(0x2B);
		writeData(y1 >> 8);
		writeData(y1);
		writeData(y2 >> 8);
		writeData(y2);
	}
	
	static inline void setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
		setColumnAddress(x1, x2);
		setPageAddress(y1, y2);
		writeCom(0x2C);
	}
	
	static inline void clrXY() {
		setXY(0, 0, W - 1, H - 1);
	}
	
	static inline void setPixel(color_t color) {
		writeData(color >> 8, color & 0xFF); // rrrrrggggggbbbbb
	}
	
	void drawHLine(uint16_t x, uint16_t y, uint16_t len) {
		CS::clr();
		setXY(x, y, x + len, y);
		fastFill16(fore_color, len);
		CS::set();
		clrXY();
	}

	void drawVLine(uint16_t x, uint16_t y, uint16_t len) {
		CS::clr();
		setXY(x, y, x, y + len);
		fastFill16(fore_color, len);
		CS::set();
		clrXY();
	}
};

#endif /* ILIDRIVER_H_ */