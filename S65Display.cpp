#ifdef __cplusplus
extern "C" {
#endif
  #include <inttypes.h>
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  #include <util/delay.h>
  #include "fonts.h"
#ifdef __cplusplus
}
#endif
#include "stdint.h"
#include "Arduino.h" 
#include "digitalWriteFast.h"
#include "S65Display.h"
#include "ls020.h"


//-------------------- Constructor --------------------


S65Display::S65Display(void)
{
  return;
}


//-------------------- Public --------------------


void S65Display::init(uint8_t clock_div)
{
  //init pins
  pinMode(S65_RST_PIN, OUTPUT);
  digitalWrite(S65_RST_PIN, LOW);
  pinMode(S65_CS_PIN, OUTPUT);
  digitalWrite(S65_CS_PIN, HIGH);
  pinMode(S65_RS_PIN, OUTPUT);
  pinMode(S65_CLK_PIN, OUTPUT);
  pinMode(S65_DAT_PIN, OUTPUT);

# if (defined(__AVR_ATmega1280__) || \
      defined(__AVR_ATmega1281__) || \
      defined(__AVR_ATmega2560__) || \
      defined(__AVR_ATmega2561__))     //--- Arduino Mega ---
  DDRF |= (1<<2)|(1<<3); //A0-A15 currently not working
#endif

#if !defined(S65_SOFTWARE_SPI)
  //SS has to be output or input with pull-up
# if (defined(__AVR_ATmega1280__) || \
      defined(__AVR_ATmega1281__) || \
      defined(__AVR_ATmega2560__) || \
      defined(__AVR_ATmega2561__))     //--- Arduino Mega ---
#  define S65_SS_PORTBIT (0) //PB0
# else                                 //--- Arduino Uno ---
#  define S65_SS_PORTBIT (2) //PB2
# endif
  if(!(DDRB & (1<<S65_SS_PORTBIT))) //SS is input
  {
      PORTB |= (1<<S65_SS_PORTBIT); //pull-up on
  }

  //init hardware spi
  switch(clock_div)
  {
    case 2:
      SPCR = (1<<SPE)|(1<<MSTR); //enable SPI, Master, clk=Fcpu/4
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/2
      break;
    case 4:
      SPCR = (1<<SPE)|(1<<MSTR); //enable SPI, Master, clk=Fcpu/4
      SPSR = (0<<SPI2X); //clk*2 = off
      break;
    case 8:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //enable SPI, Master, clk=Fcpu/16
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/8
      break;
    case 16:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); //enable SPI, Master, clk=Fcpu/16
      SPSR = (0<<SPI2X); //clk*2 = off
      break;
    case 32:
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1); //enable SPI, Master, clk=Fcpu/64
      SPSR = (1<<SPI2X); //clk*2 = Fcpu/32
      break;
  }
#endif

  s65_init();

  return;
}

void S65Display::scroll(uint8_t value)
{
  s65_scroll(value);
  return;
}

void S65Display::scrollP(uint8_t start, uint8_t width, uint8_t value)
{
  s65_scrollP(start,width,value);
  return;
}

void S65Display::drawStart(void)
{
  s65_drawStart();

  return;
}


void S65Display::draw(uint16_t color)
{
  s65_draw(color);

  return;
}


void S65Display::drawStop(void)
{
  s65_drawStop();

  return;
}


void S65Display::setArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  s65_setArea(x0, y0, x1, y1);

  return;
}


void S65Display::setCursor(uint8_t x, uint8_t y)
{
  s65_setCursor(x, y);

  return;
}


void S65Display::clear(uint16_t color)
{
  uint16_t size;

  s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

  s65_drawStart();
  for(size=(S65_WIDTH*S65_HEIGHT); size!=0; size--)
  {
    s65_draw(color);
  }
  s65_drawStop();

  return;
}


void S65Display::drawPixel(uint8_t x0, uint8_t y0, uint16_t color)
{
  if((x0 >= S65_WIDTH) ||
     (y0 >= S65_HEIGHT))
  {
    return;
  }

  s65_setCursor(x0, y0);

  s65_drawStart();
  s65_draw(color);
  s65_drawStop();

  return;
}


void S65Display::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
	int16_t dx, dy, dx2, dy2, err, stepx, stepy;

  if((x0 == x1) ||
     (y0 == y1)) //horizontal or vertical line
  {
    fillRect(x0, y0, x1, y1, color);
  }
  else
  {
    //calculate direction
    dx = x1 - x0;
    dy = y1 - y0;
    if(dx < 0) { dx = -dx; stepx = -1; } else { stepx = +1; }
    if(dy < 0) { dy = -dy; stepy = -1; } else { stepy = +1; }
    dx2 = dx << 1;
    dy2 = dy << 1;
    //draw line
    s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));
    drawPixel(x0, y0, color);
    if(dx > dy)
    {
      err = dy2 - dx;
      while(x0 != x1)
      {
        if(err >= 0)
        {
          y0  += stepy;
          err -= dx2;
        }
        x0  += stepx;
        err += dy2;
        drawPixel(x0, y0, color);
      }
    }
    else
    {
      err = dx2 - dy;
      while(y0 != y1)
      {
        if(err >= 0)
        {
          x0  += stepx;
          err -= dy2;
        }
        y0  += stepy;
        err += dx2;
        drawPixel(x0, y0, color);
      }
    }
  }

  return;
}


void S65Display::drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
  fillRect(x0, y0, x0, y1, color);
  fillRect(x0, y1, x1, y1, color);
  fillRect(x1, y0, x1, y1, color);
  fillRect(x0, y0, x1, y0, color);

  return;
}


void S65Display::fillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
  uint16_t size;
  uint8_t tmp;

  if(x0 > x1)
  {
    tmp = x0;
    x0  = x1;
    x1  = tmp;
  }
  if(y0 > y1)
  {
    tmp = y0;
    y0  = y1;
    y1  = tmp;
  }

  if((x1 >= S65_WIDTH) ||
     (y1 >= S65_HEIGHT))
  {
    return;
  }

  s65_setArea(x0, y0, x1, y1);

  s65_drawStart();
  for(size=((1+(x1-x0))*(1+(y1-y0))); size!=0; size--)
  {
    s65_draw(color);
  }
  s65_drawStop();

  return;
}


void S65Display::drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color)
{
  int16_t err, x, y;
  
  err = -radius;
  x   = radius;
  y   = 0;

  s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

  while(x >= y)
  {
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);

    err += y;
    y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err -= x;
      err -= x;
    }
  }

  return;
}


void S65Display::fillCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color)
{
  int16_t err, x, y;
  
  err = -radius;
  x   = radius;
  y   = 0;

  s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

  while(x >= y)
  {
    drawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
    drawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
    drawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
    drawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);

    err += y;
    y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err -= x;
      err -= x;
    }
  }

  return;
}


uint8_t S65Display::drawChar(uint8_t x, uint8_t y, char c, uint8_t size, uint16_t color, uint16_t bg_color)
{
  uint8_t ret;
#if FONT_WIDTH <= 8
  uint8_t data, mask;
#elif FONT_WIDTH <= 16
  uint16_t data, mask;
#elif FONT_WIDTH <= 32
  uint32_t data, mask;
#endif
  uint8_t i, j, width, height;
  const prog_uint8_t *ptr;

  i      = (uint8_t)c;
#if FONT_WIDTH <= 8
  ptr    = &font_PGM[(i-FONT_START)*(8*FONT_HEIGHT/8)];
#elif FONT_WIDTH <= 16
  ptr    = &font_PGM[(i-FONT_START)*(16*FONT_HEIGHT/8)];
#elif FONT_WIDTH <= 32
  ptr    = &font_PGM[(i-FONT_START)*(32*FONT_HEIGHT/8)];
#endif
  width  = FONT_WIDTH;
  height = FONT_HEIGHT;

  if(size <= 1)
  {
    ret = x+width;
    if(ret > S65_WIDTH)
    {
      return S65_WIDTH+1;
    }

    setArea(x, y, (x+width-1), (y+height-1));

    s65_drawStart();
    for(; height!=0; height--)
    {
#if FONT_WIDTH <= 8
      data = pgm_read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
      data = pgm_read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
      data = pgm_read_dword(ptr); ptr+=4;
#endif
      for(mask=(1<<(width-1)); mask!=0; mask>>=1)
      {
        if(data & mask)
        {
          s65_draw(color);
        }
        else
        {
          s65_draw(bg_color);
        }
      }
    }
    s65_drawStop();
  }
  else
  {
    ret = x+(width*size);
    if(ret > S65_WIDTH)
    {
      return S65_WIDTH+1;
    }

    s65_setArea(x, y, (x+(width*size)-1), (y+(height*size)-1));

    s65_drawStart();
    for(; height!=0; height--)
    {
#if FONT_WIDTH <= 8
      data = pgm_read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
      data = pgm_read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
      data = pgm_read_dword(ptr); ptr+=4;
#endif
      for(i=size; i!=0; i--)
      {
        for(mask=(1<<(width-1)); mask!=0; mask>>=1)
        {
          if(data & mask)
          {
            for(j=size; j!=0; j--)
            {
              s65_draw(color);
            }
          }
          else
          {
            for(j=size; j!=0; j--)
            {
              s65_draw(bg_color);
            }
          }
        }
      }
    }
    s65_drawStop();
  }

  return ret;
}


uint8_t S65Display::drawText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  while(*s != 0)
  {
    x = drawChar(x, y, *s++, size, color, bg_color);
    if(x > S65_WIDTH)
    {
      break;
    }
  }

  return x;
}


uint8_t S65Display::drawTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char c;

  c = pgm_read_byte(s++);
  while(c != 0)
  {
    x = drawChar(x, y, c, size, color, bg_color);
    if(x > S65_WIDTH)
    {
      break;
    }
    c = pgm_read_byte(s++);
  }

  return x;
}


uint8_t S65Display::drawMLText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  uint8_t i, start_x=x, wlen, llen;
  char c;
  char *wstart;

  fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text

  llen   = (S65_WIDTH-x)/8;
  wstart = s;
  while(*s)
  {
    c = *s++;
    if(c == '\n') //new line
    {
      fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
      x  = start_x;
      y += (FONT_HEIGHT*size)+2;
      fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
      continue;
    }

    if(c == ' ') //start of a new word
    {
      wstart = s;
    }

    if((c == ' ') && (x == start_x))
    {
      //do nothing
    }
    else if(c >= FONT_START)
    {
      i = drawChar(x, y, c, size, color, bg_color);
      if(i > S65_WIDTH) //new line
      {
        if(c == ' ') //do not start with space
        {
          fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
          x  = start_x;
          y += (FONT_HEIGHT*size)+2;
          fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
        }
        else
        {
          wlen = (s-wstart);
          if(wlen > llen) //word too long
          {
            fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
            x  = start_x;
            y += (FONT_HEIGHT*size)+2;
            fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
            x = drawChar(x, y, c, size, color, bg_color);
          }
          else
          {
            fillRect(x-(wlen*8), y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
            x  = start_x;
            y += (FONT_HEIGHT*size)+2;
            fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
            s = wstart;
          }
        }
      }
      else
      {
        x = i;
      }
    }
  }

  fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text

  return x;
}


uint8_t S65Display::drawMLTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  uint8_t i, start_x=x, wlen, llen;
  char c;
  PGM_P wstart;

  fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text

  llen   = (S65_WIDTH-x)/8;
  wstart = s;

  c = pgm_read_byte(s++);
  while(c != 0)
  {
    if(c == '\n') //new line
    {
      fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
      x  = start_x;
      y += (FONT_HEIGHT*size)+2;
      fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
      c = pgm_read_byte(s++);
      continue;
    }

    if(c == ' ') //start of a new word
    {
      wstart = s;
    }

    if((c == ' ') && (x == start_x))
    {
      //do nothing
    }
    else if(c >= FONT_START)
    {
      i = drawChar(x, y, c, size, color, bg_color);
      if(i > S65_WIDTH) //new line
      {
        if(c == ' ') //do not start with space
        {
          fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
          x  = start_x;
          y += (FONT_HEIGHT*size)+2;
          fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
        }
        else
        {
          wlen = (s-wstart);
          if(wlen > llen) //word too long
          {
            fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
            x  = start_x;
            y += (FONT_HEIGHT*size)+2;
            fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
            x = drawChar(x, y, c, size, color, bg_color);
          }
          else
          {
            fillRect(x-(wlen*8), y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
            x  = start_x;
            y += (FONT_HEIGHT*size)+2;
            fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
            s = wstart;
          }
        }
      }
      else
      {
        x = i;
      }
    }
    c = pgm_read_byte(s++);
  }

  fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text

  return x;
}
