#ifndef ls020_h
#define ls020_h


#ifdef S65_LS020

void s65_drawStart(void);
void s65_draw(uint16_t color);
void s65_drawStop(void);
void s65_setArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void s65_setCursor(uint8_t x, uint8_t y);
void s65_scrollP(uint8_t start, uint8_t width, uint8_t value);
void s65_scroll(uint8_t value);
void s65_on(void);
void s65_off(uint8_t bw);
void s65_invert(uint8_t inv);
void s65_contrast(uint8_t value);
void s65_init(void);
void s65_writeData(uint16_t data);
void s65_writeCmd(uint8_t reg, uint8_t param);
void s65_writeSPI(uint8_t data);

#endif //S65_LS020


#endif //ls020_h
