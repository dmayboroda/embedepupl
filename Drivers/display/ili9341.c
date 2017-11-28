#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "lcd.h"
#include "glcdfont.h"
#include "stm32f4xx_hal.h"
#include "fsmc.h"
#include "dma.h"

static int16_t m_width;
static int16_t m_height;
static int16_t m_cursor_x;
static int16_t m_cursor_y;

static uint16_t m_textcolor;
static uint16_t m_textbgcolor;
static uint8_t m_textsize;
static uint8_t m_wrap;



#if defined(STM32F407_VE)
#define LCD_REG ((uint32_t)  0x60000000) // Disp Reg ADDR
#define LCD_DATA ((uint32_t) 0x60080000) // Disp Data ADDR for A18
#endif

#if defined(STM32F407_ZE)
#define LCD_REG ((uint32_t)  0x6C000000) // Disp Reg ADDR
#define LCD_DATA ((uint32_t) 0x6C000080) // Disp Data ADDR for A6
#endif

inline void LCD_WR_REG(uint16_t index) {
	*(__IO uint16_t *) (LCD_REG) = index;
}
inline void LCD_WR_CMD(uint16_t index, uint16_t val) {
	*(__IO uint16_t *) (LCD_REG) = index;
	*(__IO uint16_t *) (LCD_DATA) = val;
}
inline void LCD_WR_Data(uint16_t val) {
	*(__IO uint16_t *) (LCD_DATA) = val;
}

/**
 * \brief GPIO Initialization
 *
 * \param
 *
 * \return void
 */

/**
 * \brief LCD Initialization
 *
 * \param
 *
 * \return void
 */
void LCD_Init(void) {
	m_width = TFTWIDTH;
	m_height = TFTHEIGHT;
	m_cursor_y = m_cursor_x = 0;
	m_textsize = 4;
	m_textcolor = m_textbgcolor = 0xFFFF;
	m_wrap = 1;

	LCD_Reset();

	LCD_WR_REG(0xCF);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x81);
	LCD_WR_Data(0x30);

	LCD_WR_REG(0xED);
	LCD_WR_Data(0x64);
	LCD_WR_Data(0x03);
	LCD_WR_Data(0x12);
	LCD_WR_Data(0x81);

	LCD_WR_REG(0xE8);
	LCD_WR_Data(0x85);
	LCD_WR_Data(0x10);
	LCD_WR_Data(0x78);

	LCD_WR_REG(0xCB);
	LCD_WR_Data(0x39);
	LCD_WR_Data(0x2C);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x34);
	LCD_WR_Data(0x02);

	LCD_WR_REG(0xF7);
	LCD_WR_Data(0x20);

	LCD_WR_REG(0xEA);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x00);

	LCD_WR_REG(0xB1);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x1B);

	LCD_WR_REG(0xB6);
	LCD_WR_Data(0x0A);
	LCD_WR_Data(0xA2);

	LCD_WR_REG(0xC0);
	LCD_WR_Data(0x35);

	LCD_WR_REG(0xC1);
	LCD_WR_Data(0x11);

	LCD_WR_REG(0xC5);
	LCD_WR_Data(0x45);
	LCD_WR_Data(0x45);

	LCD_WR_REG(0xC7);
	LCD_WR_Data(0xA2);

	LCD_WR_REG(0xF2);
	LCD_WR_Data(0x00);

	LCD_WR_REG(0x26);
	LCD_WR_Data(0x01);

	LCD_WR_REG(0xE0); //Set Gamma
	LCD_WR_Data(0x0F);
	LCD_WR_Data(0x26);
	LCD_WR_Data(0x24);
	LCD_WR_Data(0x0B);
	LCD_WR_Data(0x0E);
	LCD_WR_Data(0x09);
	LCD_WR_Data(0x54);
	LCD_WR_Data(0xA8);
	LCD_WR_Data(0x46);
	LCD_WR_Data(0x0C);
	LCD_WR_Data(0x17);
	LCD_WR_Data(0x09);
	LCD_WR_Data(0x0F);
	LCD_WR_Data(0x07);
	LCD_WR_Data(0x00);
	LCD_WR_REG(0XE1); //Set Gamma
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x19);
	LCD_WR_Data(0x1B);
	LCD_WR_Data(0x04);
	LCD_WR_Data(0x10);
	LCD_WR_Data(0x07);
	LCD_WR_Data(0x2A);
	LCD_WR_Data(0x47);
	LCD_WR_Data(0x39);
	LCD_WR_Data(0x03);
	LCD_WR_Data(0x06);
	LCD_WR_Data(0x06);
	LCD_WR_Data(0x30);
	LCD_WR_Data(0x38);
	LCD_WR_Data(0x0F);

	LCD_WR_REG(0x36);
//	LCD_WR_Data(0x08);
	LCD_WR_Data(0b01111000);

	LCD_WR_REG(ILI9341_COLADDRSET);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0xEF);

	LCD_WR_REG(ILI9341_PAGEADDRSET);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x00);
	LCD_WR_Data(0x01);
	LCD_WR_Data(0x3F);

	LCD_WR_REG(0x3a); // Memory Access Control
	LCD_WR_Data(0x55);
	LCD_WR_REG(0x11); //Exit Sleep
	HAL_Delay(120);
	LCD_WR_REG(0x29); //display on
	LCD_WR_REG(ILI9341_MEMORYWRITE);

	TIM3->CCR4 = 50; // LCD Backlight
}

/**
 * \brief Calucalte 16Bit-RGB
 *
 * \param r	Red
 * \param g	Green
 * \param b	Blue
 *
 * \return uint16_t	16Bit-RGB
 */
uint16_t LCD_Color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/**
 * \brief Draws a point at the specified coordinates
 *
 * \param x		x-Coordinate
 * \param y		y-Coordinate
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color) {
	// Clip
	if ((x < 0) || (y < 0) || (x >= TFTWIDTH) || (y >= TFTHEIGHT))
		return;
	LCD_WR_REG(ILI9341_COLADDRSET);
	LCD_WR_Data(x >> 8);
	LCD_WR_Data(x & 0xFF);

	LCD_WR_REG(ILI9341_PAGEADDRSET);
	LCD_WR_Data(y >> 8);
	LCD_WR_Data(y & 0xFF);

	LCD_WR_REG(ILI9341_MEMORYWRITE); //开始写
	LCD_WR_Data(color);
}

/**
 * \brief Draws a line connecting the two points specified by the coordinate pairs
 *
 * \param x0	The x-coordinate of the first point
 * \param y0	The y-coordinate of the first point
 * \param x1	The x-coordinate of the second point
 * \param y1	The y-coordinate of the second point.
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t color) {
	// Bresenham's algorithm - thx wikpedia

	int16_t steep = abs(y2 - y1) > abs(x2 - x1);
	if (steep) {
		swap(x1, y1);
		swap(x2, y2);
	}

	if (x1 > x2) {
		swap(x1, x2);
		swap(y1, y2);
	}

	int16_t dx, dy;
	dx = x2 - x1;
	dy = abs(y2 - y1);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y1 < y2) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x1 <= x2; x1++) {
		if (steep) {
			LCD_DrawPixel(y1, x1, color);
		} else {
			LCD_DrawPixel(x1, y1, color);
		}
		err -= dy;
		if (err < 0) {
			y1 += ystep;
			err += dx;
		}
	}
}

/**
 * \brief Draws a horizontal line
 *
 * \param x			The x-coordinate of the first point
 * \param y			The y-coordinate of the first point
 * \param length	Length of the line
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color) {
	int16_t x2;
#ifdef LCD_SPLIT_FIX
	if (y >= 160) y = 1 + (y % 160) * 2;
	else y = y * 2;
#endif
	// Initial off-screen clipping
	if ((length <= 0) || (y < 0) || (y >= m_height) || (x >= m_width) || ((x2 =
			(x + length - 1)) < 0))
		return;

	if (x < 0)   // Clip left
			{
		length += x;
		x = 0;
	}

	if (x2 >= m_width)   // Clip right
			{
		x2 = m_width - 1;
		length = x2 - x + 1;
	}

	LCD_SetAddrWindow(x, y, x2, y);
	LCD_Flood(color, length);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);

}

/**
 * \brief Draws a vertical line
 *
 * \param x		The x-coordinate of the first point
 * \param y		The y-coordinate of the first point
 * \param h		High of the line
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	// Update in subclasses if desired!
	LCD_DrawLine(x, y, x, y + h - 1, color);
}

/**
 * \brief Draws a rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x			The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y			The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w			Width of the rectangle to draw
 * \param h			Height of the rectangle to draw
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	LCD_DrawFastHLine(x, y, w, color);
	LCD_DrawFastHLine(x, y + h - 1, w, color);
	LCD_DrawFastVLine(x, y, h, color);
	LCD_DrawFastVLine(x + w - 1, y, h, color);
}

/**
 * \brief Draws a rectangle with rounded corners specified by a coordinate pair, a width, and a height.
 *
 * \param x			The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y			The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w			Width of the rectangle to draw
 * \param h			Height of the rectangle to draw
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
		uint16_t color) {
	// smarter version
	LCD_DrawFastHLine(x + r, y, w - 2 * r, color); // Top
	LCD_DrawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
	LCD_DrawFastVLine(x, y + r, h - 2 * r, color); // Left
	LCD_DrawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
	// draw four corners
	LCD_DrawCircleHelper(x + r, y + r, r, 1, color);
	LCD_DrawCircleHelper(x + w - r - 1, y + r, r, 2, color);
	LCD_DrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	LCD_DrawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

/**
 * \brief Helper function drawing rounded corners
 *
 * \param x0			The x-coordinate
 * \param y0			The y-coordinate
 * \param r				Radius
 * \param cornername	Corner (1, 2, 3, 4)
 * \param color			Color
 *
 * \return void
 */
void LCD_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
		uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			LCD_DrawPixel(x0 + x, y0 + y, color);
			LCD_DrawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			LCD_DrawPixel(x0 + x, y0 - y, color);
			LCD_DrawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			LCD_DrawPixel(x0 - y, y0 + x, color);
			LCD_DrawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			LCD_DrawPixel(x0 - y, y0 - x, color);
			LCD_DrawPixel(x0 - x, y0 - y, color);
		}
	}
}

/**
 * \brief Draws an circle defined by a pair of coordinates and radius
 *
 * \param x0		The x-coordinate
 * \param y0		The y-coordinate
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_DrawPixel(x0, y0 + r, color);
	LCD_DrawPixel(x0, y0 - r, color);
	LCD_DrawPixel(x0 + r, y0, color);
	LCD_DrawPixel(x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		LCD_DrawPixel(x0 + x, y0 + y, color);
		LCD_DrawPixel(x0 - x, y0 + y, color);
		LCD_DrawPixel(x0 + x, y0 - y, color);
		LCD_DrawPixel(x0 - x, y0 - y, color);
		LCD_DrawPixel(x0 + y, y0 + x, color);
		LCD_DrawPixel(x0 - y, y0 + x, color);
		LCD_DrawPixel(x0 + y, y0 - x, color);
		LCD_DrawPixel(x0 - y, y0 - x, color);
	}
}

/**
 * \brief Draws a character at the specified coordinates
 *
 * \param x			The x-coordinate
 * \param y			The y-coordinate
 * \param c			Character
 * \param color		Character color
 * \param bg		Background color
 * \param size		Character Size
 *
 * \return void
 */
void LCD_DrawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
		uint16_t bg, uint8_t size) {
	if ((x >= m_width) || // Clip right
			(y >= m_height) || // Clip bottom
			((x + 6 * size - 1) < 0) || // Clip left
			((y + 8 * size - 1) < 0))   // Clip top
		return;

	for (int8_t i = 0; i < 6; i++) {
		uint8_t line;
		if (i == 5) {
			line = 0x0;
		} else {
			line = font[c * 5 + i];   //pgm_read_byte(font+(c*5)+i);
			for (int8_t j = 0; j < 8; j++) {
				if (line & 0x1) {
					if (size == 1)   // default size
							{
						LCD_DrawPixel(x + i, y + j, color);
					} else      // big size
					{
						LCD_FillRect(x + (i * size), y + (j * size), size, size,
								color);
					}
				} else if (bg != color) {
					if (size == 1)   // default size
							{
						LCD_DrawPixel(x + i, y + j, bg);
					} else      // big size
					{
						LCD_FillRect(x + i * size, y + j * size, size, size,
								bg);
					}
				}
				line >>= 1;
			}
		}
	}
}

/**
 * \brief Draws a filled circle defined by a pair of coordinates and radius
 *
 * \param x0		The x-coordinate
 * \param y0		The y-coordinate
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	LCD_DrawFastVLine(x0, y0 - r, 2 * r + 1, color);
	LCD_FillCircleHelper(x0, y0, r, 3, 0, color);
}

/**
 * \brief Helper function to draw a filled circle
 *
 * \param x0			The x-coordinate
 * \param y0			The y-coordinate
 * \param r				Radius
 * \param cornername	Corner (1, 2, 3, 4)
 * \param delta			Delta
 * \param color			Color
 *
 * \return void
 */
void LCD_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
		int16_t delta, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x1) {
			LCD_DrawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
			LCD_DrawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
		}
		if (cornername & 0x2) {
			LCD_DrawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
			LCD_DrawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
		}
	}
}

/**
 * \brief Draws a filled rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x				The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y				The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w				Width of the rectangle to draw
 * \param h				Height of the rectangle to draw
 * \param fillcolor		Color
 *
 * \return void
 */
void LCD_FillRect(int16_t x, int16_t y1, int16_t w, int16_t h,
		uint16_t fillcolor) {
	int16_t x2, y2;

	// Initial off-screen clipping
	if ((w <= 0) || (h <= 0) || (x >= m_width) || (y1 >= m_height)
			|| ((x2 = x + w - 1) < 0) || ((y2 = y1 + h - 1) < 0))
		return;
	if (x < 0)   // Clip left
			{
		w += x;
		x = 0;
	}
	if (y1 < 0)   // Clip top
			{
		h += y1;
		y1 = 0;
	}
	if (x2 >= m_width)   // Clip right
			{
		x2 = m_width - 1;
		w = x2 - x + 1;
	}
	if (y2 >= m_height)   // Clip bottom
			{
		y2 = m_height - 1;
		h = y2 - y1 + 1;
	}

	LCD_SetAddrWindow(x, y1, x2, y2);
	LCD_Flood(fillcolor, (uint32_t) w * (uint32_t) h);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}

/**
 * \brief Draws a filled rounded rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x				The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y				The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w				Width of the rectangle to draw
 * \param h				Height of the rectangle to draw
 * \param r				Radius
 * \param fillcolor		Color
 *
 * \return void
 */
void LCD_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
		uint16_t color) {
	// smarter version
	LCD_FillRect(x + r, y, w - 2 * r, h, color);

	// draw four corners
	LCD_FillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	LCD_FillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

/**
 * \brief Fills the screen with the specified color
 *
 * \param color	Color
 *
 * \return void
 */
void LCD_FillScreen(uint16_t color) {
//	LCD_CS_LOW();
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
	LCD_Flood(color, (long) TFTWIDTH * (long) TFTHEIGHT);
}

/**
 * \brief Flood
 *
 * \param color	Color
 * \param len	Length
 *
 * \return void
 */
void LCD_Flood(uint16_t color, uint32_t len) {
	LCD_WR_REG(0x2C);
	for (uint32_t i = 0; i < len; i++) {
		LCD_WR_Data(color);
	}

}

/**
 * \brief Print the specified Text
 *
 * \param fmt	Format text
 * \param
 *
 * \return void
 */
void LCD_Printf(const char *fmt, ...) {
	static char buf[256];
	char *p;
	va_list lst;

	va_start(lst, fmt);
	vsprintf(buf, fmt, lst);
	va_end(lst);

	p = buf;
	while (*p) {
		if (*p == '\n') {
			m_cursor_y += m_textsize * 8;
			m_cursor_x = 0;
		} else if (*p == '\r') {
			m_cursor_x = 0;
		} else if (*p == '\t') {
			m_cursor_x += m_textsize * 6 * 4;
		} else {
			LCD_DrawChar(m_cursor_x, m_cursor_y, *p, m_textcolor, m_textbgcolor,
					m_textsize);
			m_cursor_x += m_textsize * 6;
			if (m_wrap && (m_cursor_x > (m_width - m_textsize * 6))) {
				m_cursor_y += m_textsize * 8;
				m_cursor_x = 0;
			}
		}
		p++;

		if (m_cursor_y >= 320) {
			m_cursor_y = 0;
			//LCD_FillScreen(m_textbgcolor);
		}
	}
}

/**
 * \brief Clears the amount lines
 *
 * \param s 	Starting line
 * \param n 	Number of lines
 * \param r		Refresh rate in Hz
 *
 * \return void
 */
void LCD_ClearLines(uint8_t s, uint8_t n, uint8_t r) {
	if (r < 0)
		r = 1;
	if (r > 1) {
		HAL_Delay(1000 / r);
	}
	if (s < 1)
		s = 1;
	if (n < 1)
		n = 1;
	m_cursor_x = 0;
	m_cursor_y = s * m_textsize * 8;
	LCD_SetAddrWindow(0, m_cursor_y, m_width - 1,
			m_cursor_y + n * m_textsize * 8);
	LCD_Flood(m_textbgcolor, (long) TFTWIDTH * (long) n * m_textsize * 8);
}

/**
 * \brief Resets the Display
 *
 * \param
 *
 * \return void
 */
void LCD_Reset(void) {
	LCD_WR_REG(ILI9341_SOFTRESET);
	HAL_Delay(50);
}

void LCD_SetBacklight(uint8_t percentage) {
	TIM3->CCR4 = percentage;
}

/**
 * \brief Sets the cursor coordinates
 *
 * \param x		The x-coordinate
 * \param y		The y-coordinate
 *
 * \return void
 */
void LCD_SetCursor(unsigned int x, unsigned int y) {
	m_cursor_x = x;
	m_cursor_y = y;
}

/**
 * \brief Sets the text size
 *
 * \param s	Size
 *
 * \return void
 */
void LCD_SetTextSize(uint8_t s) {
	m_textsize = (s > 0) ? s : 1;
}

/**
 * \brief Sets the text color
 *
 * \param c		Text color
 * \param b		Background color
 *
 * \return void
 */
void LCD_SetTextColor(uint16_t c, uint16_t b) {
	m_textcolor = c;
	m_textbgcolor = b;
}

/**
 * \brief Set Text wrap
 *
 * \param w
 *
 * \return void
 */
void LCD_SetTextWrap(uint8_t w) {
	m_wrap = w;
}

/**
 * \brief Sets window address
 *
 * \param x1
 * \param y1
 * \param x2
 * \param y2
 *
 * \return void
 */
void LCD_SetAddrWindow(int x1, int y1, int x2, int y2) {
	LCD_WR_REG(ILI9341_COLADDRSET);
	LCD_WR_Data(x1 >> 8);
	LCD_WR_Data(x1 & 0xFF);
	LCD_WR_Data(x2 >> 8);
	LCD_WR_Data(x2 & 0xFF);
	LCD_WR_REG(ILI9341_PAGEADDRSET);
	LCD_WR_Data(y1 >> 8);
	LCD_WR_Data(y1 & 0xFF);
	LCD_WR_Data(y2 >> 8);
	LCD_WR_Data(y2 & 0xFF);
	LCD_WR_REG(0x2C);
}

void LCD_FlushBufferDMA(int16_t x, int16_t y, uint32_t *buf, uint32_t len) {

	LCD_WR_REG(0X2A);
	LCD_WR_Data(x >> 8);
	LCD_WR_Data(x & 0xFF);

	LCD_WR_REG(0X2B);
	LCD_WR_Data(y >> 8);
	LCD_WR_Data(y & 0xFF);

	LCD_WR_REG(0x2C);
	HAL_DMA_Start(&hdma_memtomem_dma2_stream0, buf, LCD_DATA, len);
}

void LCD_DrawRAWFromFile(int16_t xPos, int16_t yPos, uint16_t width,
		uint16_t height, uint8_t isTransparentBlack, FIL * pFile) {
	const uint32_t clusterSize = 4096;
	uint8_t buf[clusterSize];
	uint32_t readBytes = 0, clusterNum = 0, clusterTotal = 0;
	uint32_t xOffset = 0, yOffset = 0;

	uint32_t size = 2 * width * height;
	if (size % clusterSize) {
		clusterTotal = size / clusterSize + 1;
	} else {
		clusterTotal = size / clusterSize;
	}

	LCD_SetAddrWindow(xPos, yPos, xPos + width - 1, yPos + height - 1);

	while (clusterNum <= clusterTotal) {
		/* Read new cluster */
		f_read(pFile, buf, clusterSize, (UINT *) &readBytes);
		uint16_t * data = (uint16_t *) buf;
		while (readBytes) {
			if (isTransparentBlack) {
				if ((*data)) {
					LCD_DrawPixel(xPos + xOffset++, yPos + yOffset,
							(Swap2Bytes(*data)));
				} else {
					xOffset++;
				}

				if (xOffset >= width) {
					yOffset++;
					xOffset = 0;
				}

				data += 1;
				readBytes -= 2;
			} else {

				for (int i = 0; i < readBytes/2; ++i) {
					data[i] = Swap2Bytes(data[i]);
				}

				while(HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, data, LCD_DATA, readBytes/2)!=HAL_OK);
				readBytes = 0;
			}
		}

		clusterNum++;
	}
}

/**
 * \brief  Draws a bitmap picture from FatFs file.
 *
 * \param  xPos: Bmp X position in the LCD
 * \param  yPos: Bmp Y position in the LCD
 * \param  pFile: Pointer to FIL object with bmp picture
 * \retval None
 */
void LCD_DrawBMPFromFile(int16_t xPos, int16_t yPos, FIL * pFile) {
	const uint32_t clusterSize = 512;
	uint8_t buf[clusterSize];
	uint32_t readBytes = 0, clusterNum = 0, clusterTotal = 0;
	uint16_t* pBmp;
	uint8_t* start;
	uint8_t* end;
	uint32_t offset = 0, size = 0;
	int32_t height = 0, width = 0, xOffset = 0, yOffset = 0;
	uint16_t colordepth = 0;

	/* Read BMP header: 54 bytes = 14 bytes header + 40 bytes DIB header (assuming BITMAPINFOHEADER) */
	f_read(pFile, buf, 54, (UINT *) &readBytes);
	if (readBytes != 54) {
		return;
	}
	pBmp = buf;
	/* Read bitmap size */
	size = *(uint32_t *) (pBmp + 1);
	/* Get bitmap data address offset */
	offset = *(uint32_t *) (pBmp + 5);
	/* Calculate total number of clusters to read */
	if ((size - offset) % clusterSize) {
		clusterTotal = (size - offset) / clusterSize + 1;
	} else {
		clusterTotal = (size - offset) / clusterSize;
	}
	/* Read bitmap width */
	width = *(uint32_t *) (pBmp + 9);
	/* Read bitmap height */
	height = *(uint32_t *) (pBmp + 11);
	/* Read color depth */
	colordepth = *(pBmp + 14);

	/* Start drawing */
//	if (height < 0) {
	/* Top-bottom file */
	/* Move read pointer to beginning of pixel data */
	LCD_SetAddrWindow(xPos, yPos, xPos + width - 1, yPos + height - 1);
	f_lseek(pFile, offset);
	while (clusterNum <= clusterTotal) {
		/* Read new cluster */
		f_read(pFile, buf, clusterSize, (UINT *) &readBytes);
		start = buf;
		end = buf + readBytes;
		pBmp = start;
		/* Draw image */
		if (colordepth == 16) {
			while (pBmp < end) {
//				if (*pBmp)
				LCD_WR_Data((*pBmp));
//					LCD_DrawPixel(xPos + xOffset++, yPos + height - 1 - yOffset,
//							(Swap2Bytes(*pBmp)));
//				else
//					xOffset++;
//				if (xOffset >= width) {
//					yOffset++;
//					xOffset = 0;
//				}
				pBmp += 1;
			}
//      } else if (colordepth == 24) {
//        while (pBmp < end) {
//          LCD_Write8((*(pBmp) & 0xF8) | (*(pBmp + 1) >> 5));
//          LCD_Write8(((*(pBmp + 1) & 0x1C) << 3) | (*(pBmp + 2) >> 3));
//          pBmp += 3;
//        }
		}
		clusterNum++;
	}
//	} else {
//		/* Bottom-top file */
//		clusterNum = clusterTotal;
//		while (clusterNum > 0) {
//			f_lseek(pFile, offset + (clusterNum - 1) * clusterSize);
//			f_read(pFile, buf, clusterSize, (UINT *) &readBytes);
//			start = buf;
//			end = buf + readBytes;
//			pBmp = end - 1;
//			if (colordepth == 16) {
//				while (pBmp > start) {
//
//					if (*pBmp)
//						LCD_DrawPixel(xPos + width-1 - xOffset, yPos + yOffset, Swap2Bytes(*pBmp));
//
//					if (++xOffset >= width) {
//						yOffset++;
//						xOffset = 0;
//					}
//
//					pBmp -= 1;
//				}
////      } else if (colordepth == 24) {
////        while (pBmp >= start) {
////          LCD_Write8((*(pBmp) & 0xF8) | (*(pBmp - 1) >> 5));
////          LCD_Write8(((*(pBmp - 1) & 0x1C) << 3) | (*(pBmp - 2) >> 3));
////          pBmp -= 3;
////        }
//			}
//			clusterNum--;
//		}
//	}
}
