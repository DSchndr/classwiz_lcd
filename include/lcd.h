#ifndef lcd_h
#define lcd_h

#include <array>
#include "../include/Font.h"

#define ON 12

#define RESET 13
#define DISPLAYEN 18
#define CLOCK 14
#define SHIFT 27
#define MYST 26
#define FIRST_LINE_MARKER 25

//{2,15 ,4 ,16,19,23,32,33
#define D0 2
#define D1 15
#define D2 4
#define D3 16
#define D4 19
#define D5 23
#define D6 32
#define D7 33

//#define LOW false
//#define HIGH true

//First line (0) is not getting drawn onto lcd
//Second (1) is Icon bar (icon_pos is x position of icon)
//Rest is dot matrix area

/*
X Position of Display icon
ICONS ARE AT Y=1
*/
enum icon_pos {
  S = 7,
  A = 15,
  M = 23,
  X = 31,
  SQRT = 47,
  D = 55,
  R = 63,
  G = 71,
  FIX = 79,
  SCI = 87,
  E = 95,
  I = 103,
  DEG = 111,
  DOWN_CLEAR = 127,
  LEFT = 135,
  DOWN = 143,
  UP = 151,
  RIGHT = 159,
  PAUSE = 175,
  SUN = 183
};

class CasioLCD {
    public:
        std::array<std::array<bool, 64>, 192> framebuffer;
        CasioLCD();
        void Writeframebuffer();
        void setContrast(uint8_t contrast);
        void Init();
        void ResetPosition();
        void SetBus(uint8_t bus);
        void SetBus(int a, int b, int c, int d, int e, int f, int g, int h);
        void setReset(bool state);
        void TakeoverBus();
        void BusTristate();
        void Prepare();
        void DrawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
                              int16_t w, int16_t h, bool color);
        void ClearFB();
        void line(int x1, int y1, int x2, int y2);
        void lineLow(int x1, int y1, int x2, int y2);
        void lineHigh(int x1, int y1, int x2, int y2);
        void rect(int x, int y, int width, int height, bool _fill);
        void text(const char* str, int x, int y, bool color);
        void text(const char* str, int x, int y, bool color, bool onlyfirstline);
        void bitmap(const uint8_t* data, int x, int y, int width, int height, bool color);
        void textFont(const Font& which);
        int textFontWidth() const;
        int textFontHeight() const;


    private:
        const Font* _font;
        void FBtoBus(int x, int i);
        void WriteByte();
        void BeforeWrite();
        void BeginWrite();
        void BeginWrite(int offset); //Checkme, offset maybe not needed or needs to be implemented differently?!
        void ResetBus();
        //void SetControl(bool Clock, bool Shift, bool Myst, bool FLM);
        void WriteGPIO(int pin, bool state);
        void SetBusTristate(bool state);


    
};

extern const struct Font Font_4x6;
extern const struct Font Font_5x7;

#endif