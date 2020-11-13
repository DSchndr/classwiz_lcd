//ESP32 code
//TODO: Use i2s for blazing fast parallel write

#include <Arduino.h>
#include <array>
#include "bitmaps.h"

//Reset pin (P151)
int _resetcalc = 13; //channel 0

//Solder on wires onto testpads from left side to right side
int _displayen = 15; //channel 1
int _clock = 25; // channel 2
int _shift = 26;  //channel 3 //CS line?! Low->Instruction Register(write) | High->Data Register
int _myst =  27; //channel 4
int _begin = 14; //channel 5

int _pins[8] = {4,16 ,17 ,18,19,21,22,23}; //D0-D7

void disableProc();
void enableProc();
void restartProc();
void setTristate();
void initlcd();
void setbus(int a, int b, int c, int d, int e, int f, int g, int h);
void setbus(uint8_t* arr);
void beforewrite();
void write();
void write_dip();

void test();
void setOutput();
void beginwrite(int offset);
void gpioMode(uint8_t gpio, uint8_t mode);
void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, bool color);

// bool framebuffer[192][64];
std::array<std::array<bool, 64>, 192> framebuffer; //Luca, plz stop this is a poc
int xwert = 0;

void setup() {
  drawBitmap(0,0,BMP,192,64,true);
  framebuffer[61][191] = true;
  framebuffer[61][189] = true;
  framebuffer[61][186] = true;
  framebuffer[61][184] = true;

  restartProc();
  delay(2000);
  digitalWrite(_displayen, HIGH);
  digitalWrite(_myst, HIGH);
  pinMode(_myst, OUTPUT);
  pinMode(_displayen, OUTPUT);
  setOutput();
  disableProc();
  delay(2000);
  test();
}

void loop() {
  delay(1000);
  for(int x = 0; x<192; x++) {
    for(int y = 0; y<64;y++) {
      framebuffer[x][y] = 0;
    }
  }
  drawBitmap(0,0,hacked,192,64,true);
  test();
  delay(1000);
    for(int x = 0; x<192; x++) {
    for(int y = 0; y<64;y++) {
      framebuffer[x][y] = 0;
    }
  }
  drawBitmap(0,0,BMP,192,64,true);
  test();
}

void test() {
  beforewrite();
  ets_delay_us(500);
  beginwrite(0);
  ets_delay_us(5);
    for(int i = 1; i <=64; i++) {
      for(int j = 0; j < 8; j++) {
        ets_delay_us(1);
        //setbus(framebuffer[i] + xwert);
        setbus(
          framebuffer[xwert][i],
          framebuffer[xwert + 1][i],
          framebuffer[xwert + 2][i],
          framebuffer[xwert + 3][i],
          framebuffer[xwert + 4][i],
          framebuffer[xwert + 5][i],
          framebuffer[xwert + 6][i],
          framebuffer[xwert + 7][i]
          );
        write();
        ets_delay_us(1);
        setbus(
          framebuffer[xwert + 8][i],
          framebuffer[xwert + 1 + 8][i],
          framebuffer[xwert + 2 + 8][i],
          framebuffer[xwert + 3 + 8][i],
          framebuffer[xwert + 4 + 8][i],
          framebuffer[xwert + 5 + 8][i],
          framebuffer[xwert + 6 + 8][i],
          framebuffer[xwert + 7 + 8][i]
          );
        write();
        ets_delay_us(1);
        setbus(
          framebuffer[xwert + 0 + 16][i],
          framebuffer[xwert + 1 + 16][i],
          framebuffer[xwert + 2 + 16][i],
          framebuffer[xwert + 3 + 16][i],
          framebuffer[xwert + 4 + 16][i],
          framebuffer[xwert + 5 + 16][i],
          framebuffer[xwert + 6 + 16][i],
          framebuffer[xwert + 7 + 16][i]
          );
        write();
        xwert = xwert + 24;
      }
      xwert = 0;
      ets_delay_us(5);
  }
}

void write() {
  digitalWrite(_clock, 0);
  digitalWrite(_shift, 0);
  ets_delay_us(2);
  setbus(1,1,1,1,1,1,1,1);
  digitalWrite(_clock, 1);
  digitalWrite(_shift, 1);
}

void write_dip() {
  setbus(0,0,0,0,0,0,0,0);
  digitalWrite(_clock, 0);
  digitalWrite(_shift, 0);
  digitalWrite(_myst, 0);
  ets_delay_us(1);
  digitalWrite(_myst, 1);
  ets_delay_us(2);
  setbus(1,1,1,1,1,1,1,1);
  digitalWrite(_clock, 1);
  digitalWrite(_shift, 1);
}

void beginwrite(int offset) {
  setbus(0,0,0,0,0,0,0,0);
  digitalWrite(_begin, 1);
  ets_delay_us(1);
  digitalWrite(_shift, 0);
  ets_delay_us(1);
  digitalWrite(_begin, 0);
  digitalWrite(_shift, 1);
  digitalWrite(_clock, 0);
  ets_delay_us(1);
  //CHECKME: Pixels get offset when you pulse _shift?
  for(int i = 0; i<offset; i++) {
    digitalWrite(_shift, 1);
    ets_delay_us(1);
    digitalWrite(_shift, 0);
    ets_delay_us(1);
  }
  digitalWrite(_clock, 1);
  digitalWrite(_shift, 1);
}

void beforewrite() {
  setbus(1,0,0,0,0,0,1,0);
  ets_delay_us(4);
  digitalWrite(_shift, 0);
  setbus(0,0,0,1,0,1,0,0);
  ets_delay_us(3);
  digitalWrite(_clock, 0);
  ets_delay_us(4);
  digitalWrite(_shift, 1);
  digitalWrite(_clock, 1);  
  ets_delay_us(4);
  setbus(1,1,1,1,1,1,1,1);
}

void setbus(int a, int b, int c, int d, int e, int f, int g, int h) {
  digitalWrite(_pins[0], a);
  digitalWrite(_pins[1], b);
  digitalWrite(_pins[2], c);
  digitalWrite(_pins[3], d);
  digitalWrite(_pins[4], e);
  digitalWrite(_pins[5], f);
  digitalWrite(_pins[6], g);
  digitalWrite(_pins[7], h);
}

void setbus(uint8_t* arr) {
  digitalWrite(_pins[0], arr[0]);
  digitalWrite(_pins[1], arr[1]);
  digitalWrite(_pins[2], arr[2]);
  digitalWrite(_pins[3], arr[3]);
  digitalWrite(_pins[4], arr[4]);
  digitalWrite(_pins[5], arr[5]);
  digitalWrite(_pins[6], arr[6]);
  digitalWrite(_pins[7], arr[7]);
}

void disableProc() {
  pinMode(_resetcalc, OUTPUT);
  digitalWrite(_resetcalc, LOW);
}

void enableProc() {
  pinMode(_resetcalc, OUTPUT);
  digitalWrite(_resetcalc, HIGH);
  pinMode(_resetcalc, INPUT);
  setTristate();
}

void restartProc() {
  setTristate();
  disableProc();
  ets_delay_us(10000);
  enableProc();
}

void setTristate() {
  for(int i = 0; i<8; i++) {
    pinMode(_pins[i], INPUT);
  }
  pinMode(_resetcalc, INPUT);
  pinMode(_displayen, INPUT);
  pinMode(_clock, INPUT);
  pinMode(_shift, INPUT);
  pinMode(_myst, INPUT);
  pinMode(_begin, INPUT);
}

void setOutput() {
  digitalWrite(_resetcalc, LOW);
  digitalWrite(_displayen, HIGH);
  digitalWrite(_clock, HIGH);
  digitalWrite(_shift, HIGH);
  digitalWrite(_begin, LOW);
  for(int i = 0; i<8; i++) {
    digitalWrite(_pins[i], HIGH);
    pinMode(_pins[i], OUTPUT);
  }
  pinMode(_resetcalc, OUTPUT);
  pinMode(_displayen, OUTPUT);
  pinMode(_clock, OUTPUT);
  pinMode(_shift, OUTPUT);
  pinMode(_myst, OUTPUT);
  pinMode(_begin, OUTPUT);
}

void initlcd() {

}


void gpioMode(uint8_t gpio, uint8_t mode)
{
  if(mode == INPUT) GPIO.enable_w1tc = ((uint32_t)1 << gpio);
  else GPIO.enable_w1ts = ((uint32_t)1 << gpio);

  ESP_REG(DR_REG_IO_MUX_BASE + esp32_gpioMux[gpio].reg) // Register lookup
    = ((uint32_t)2 << FUN_DRV_S)                        // Set drive strength 2
    | (FUN_IE)                                          // Input enable
    | ((uint32_t)2 << MCU_SEL_S);                       // Function select 2
  GPIO.pin[gpio].val = 1;                               // Set pin HIGH
}

void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
                              int16_t w, int16_t h, bool color) {

  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      if (byte & 0x80)
        //writePixel(x + i, y, color);
        framebuffer[x+i][y] = color;
    }
  }
}