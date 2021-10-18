/*
 * Adapted from http://stefanfrings.de/arduino_oled/index.html
 */

#include "oled.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_PAGES ((OLED_HEIGHT + 7) / 8)
#define OLED_BUFSIZE (OLED_PAGES * OLED_WIDTH)

uint8_t oled_buf[OLED_BUFSIZE];

void oled_clear() {
  for(int i=0; i<OLED_BUFSIZE; i++) {
    oled_buf[i] = 0x00;
  }
}

void oled_draw_byte(int x, int y, uint8_t b) {
  if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
  size_t buffer_index = y / 8 * OLED_WIDTH + x;
  if (y % 8 == 0) {
    if (buffer_index < OLED_BUFSIZE) {
      oled_buf[buffer_index] |= b;
    }
  } else {
    uint16_t w = (b << (y % 8));
    if (buffer_index < OLED_BUFSIZE) {
      oled_buf[buffer_index] |= (w & 0xFF);
    }
    size_t buffer_index2 = buffer_index + OLED_WIDTH;
    if (buffer_index2 < OLED_BUFSIZE) {
      oled_buf[buffer_index2] |= (w >> 8);
    }
  }
}

void oled_draw_bytes(int x, int y, uint8_t *data, int size, int scaling) {
  if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
  for (int column=0; column < size; column++) {
    uint8_t b = *data;
    data++;
    if(scaling == 2) {
      uint16_t w = 0;
      for (int bit=0; bit<8; bit++) {
        if (b & (1 << bit)) {
          uint8_t pos = (bit << 1) & 0xFF;
          w |= ((1 << pos) | (1 << (pos + 1)));
        }
      }
      oled_draw_byte(x, y, w & 0xFF);
      oled_draw_byte(x, y + 8, (w >> 8));
      x++;
      oled_draw_byte(x, y, w & 0xFF);
      oled_draw_byte(x, y + 8, (w >> 8));
      x++;
    } else {
      oled_draw_byte(x++, y, b);
    }
  }
}

void oled_draw_character(int x, int y, char c, int scaling) {
  if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
  size_t font_index = ((c - 32)*6) & 0xFFFF;
  if (font_index >= sizeof(oled_font6x8)) return;
  oled_draw_bytes(x, y, &oled_font6x8[font_index], 6, scaling);
}

void oled_draw_string(int origx, int y, char* s) {
  int x = origx;
  while(*s) {
    if (*s == '\n') {
      y += 11;
      x = origx;
      s++;
      continue;
    }
    oled_draw_character(x, y, *s, 1);
    x+= 12;
    s++;
  }
}

void oled_draw_pixel(int x, int y) {
  if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
  oled_buf[x + (y / 8) * OLED_WIDTH] |= (1 << (y & 7));
}

void oled_draw_line(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  int e2;

  while(1) {
    oled_draw_pixel(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 > dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void oled_init() {
  uint8_t buf[50];
  int i=0;
  buf[i++] = 0x00;
  buf[i++] = 0xAF; // off: AE, on: AF
  buf[i++] = 0xD5; // clock divider
  buf[i++] = 0x80;
  buf[i++] = 0xA8; // multiplex ratio
  buf[i++] = OLED_HEIGHT - 1; // height-1
  buf[i++] = 0xD3; // no display offset
  buf[i++] = 0x00;
  buf[i++] = 0x40; // start line address=0
  buf[i++] = 0x8D; // enable charge pump
  buf[i++] = 0x14;
  buf[i++] = 0x20; // memory adressing mode=horizontal
  buf[i++] = 0x00;
  buf[i++] = 0xA1; // segment remapping mode
  buf[i++] = 0xC8; // COM output scan direction
  buf[i++] = 0xDA; // com pins hardware configuration
  buf[i++] = (OLED_WIDTH == 64) ? 0x12 : 0x02; // height: 0x02 = 32px, 0x12 = 64px
  buf[i++] = 0x81; // contrast control
  buf[i++] = 0x40;
  buf[i++] = 0xD9; // pre-charge period
  buf[i++] = 0x22;
  buf[i++] = 0xDB;
  buf[i++] = 0x20;
  buf[i++] = 0xA4; // output RAM to display
  buf[i++] = 0xA6; // display mode: A6=normal, A7=inverse
  buf[i++] = 0x2E; // stop scrolling
  i2c_write_blocking(i2c0, 0x3C, buf, i, false);
}

void oled_display() {
  int i = 0;
  int bufindex = 0;
  uint8_t buf[50];
  for (int page=0; page<OLED_PAGES; page++) {
    i = 0;
    buf[i++] = 0x00;
    buf[i++] = 0xB0 + page;
    buf[i++] = 0x21; // column address
    buf[i++] = 0x00; // first column
    buf[i++] = OLED_WIDTH - 1; // last column
    i2c_write_blocking(i2c0, 0x3C, buf, i, false);

    i = 0;
    buf[i++] = 0x40;
    for (int column=0; column < OLED_WIDTH; column++) {
      if (column>0 && (column % 31 == 0)) {
        i2c_write_blocking(i2c0, 0x3C, buf, i, false);
        i = 0;
        buf[i++] = 0x40;
      }
      buf[i++] = oled_buf[bufindex++];
    }
    i2c_write_blocking(i2c0, 0x3C, buf, i, false);
  }
}



