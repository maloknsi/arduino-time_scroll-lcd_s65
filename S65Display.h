#ifndef S65Display_h
#define S65Display_h


#ifdef __cplusplus
extern "C" {
#endif
  #include <inttypes.h>
  #include <avr/pgmspace.h>
#ifdef __cplusplus
}
#endif


//Pins
#if (defined(__AVR_ATmega1280__) || \
     defined(__AVR_ATmega1281__) || \
     defined(__AVR_ATmega2560__) || \
     defined(__AVR_ATmega2561__))      //--- Arduino Mega ---
# define S65_RST_PIN         (17) //A3=17
# define S65_CS_PIN          (16) //A2=16
# define S65_RS_PIN          (4)
# define S65_SOFTWARE_SPI              //!!! use Software-SPI (default for Arduino Mega)
# if defined(S65_SOFTWARE_SPI)
#  define S65_CLK_PIN        (13)
#  define S65_DAT_PIN        (11)
# else
#  define S65_CLK_PIN        (52)
#  define S65_DAT_PIN        (51)
# endif

#else                                  //--- Arduino Uno ---
# define S65_RST_PIN         (9) //A3=17
# define S65_CS_PIN          (8) //A2=16
# define S65_RS_PIN          (10)
# define S65_CLK_PIN         (13)
# define S65_DAT_PIN         (11)
#endif


#define S65_RST_DISABLE()    digitalWriteFast(S65_RST_PIN, HIGH)
#define S65_RST_ENABLE()     digitalWriteFast(S65_RST_PIN, LOW)

#define S65_CS_DISABLE()     digitalWriteFast(S65_CS_PIN, HIGH)
#define S65_CS_ENABLE()      digitalWriteFast(S65_CS_PIN, LOW)

#define S65_RS_DISABLE()     digitalWriteFast(S65_RS_PIN, HIGH)
#define S65_RS_ENABLE()      digitalWriteFast(S65_RS_PIN, LOW)

#define S65_CLK_HIGH()       digitalWriteFast(S65_CLK_PIN, HIGH)
#define S65_CLK_LOW()        digitalWriteFast(S65_CLK_PIN, LOW)

#define S65_DAT_HIGH()       digitalWriteFast(S65_DAT_PIN, HIGH)
#define S65_DAT_LOW()        digitalWriteFast(S65_DAT_PIN, LOW)


//Display settings
#define S65_LS020   //LS020 type (default)
//#define S65_LPH88 //LPH88 type
//#define S65_L2F50 //L2F50 type

#define S65_MIRROR
//#define S65_ROTATE

#if defined(S65_ROTATE)
# define S65_WIDTH            (132)
# define S65_HEIGHT           (176)
#else
# define S65_WIDTH            (176)
# define S65_HEIGHT           (132)
#endif
#define RGB(r,g,b)           (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue


class S65Display
{
  public:
    S65Display();
    void init(uint8_t clock_div);

    void drawStart(void);
    void draw(uint16_t color);
    void drawStop(void);
    void setArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
    void setCursor(uint8_t x, uint8_t y);
    void clear(uint16_t color);

    void drawPixel(uint8_t x0, uint8_t y0, uint16_t color);
    void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
    void drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
    void fillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
    void drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color);
    void fillCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color);

    uint8_t drawChar(uint8_t x, uint8_t y, char c, uint8_t size, uint16_t color, uint16_t bg_color);
    uint8_t drawText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color);
    uint8_t drawTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color);

    uint8_t drawMLText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color);
    uint8_t drawMLTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color);

    void scroll(uint8_t value);
    void scrollP(uint8_t start, uint8_t width, uint8_t value);
};


#endif //S65Display_h
