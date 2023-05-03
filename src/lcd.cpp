#include <Arduino.h>
#include "../include/lcd.h"

CasioLCD::CasioLCD() : _font(NULL)
{
  Prepare();
}

void CasioLCD::Prepare() {
    pinMode(ON, INPUT_PULLUP);
    BusTristate();
    gpio_set_drive_capability((gpio_num_t)D0, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)D1, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)D2, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)D3, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)D4, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)D5, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)D6, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)D7, GPIO_DRIVE_CAP_0);

    gpio_set_drive_capability((gpio_num_t)DISPLAYEN, GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability((gpio_num_t)CLOCK, GPIO_DRIVE_CAP_1);
    gpio_set_drive_capability((gpio_num_t)SHIFT, GPIO_DRIVE_CAP_1);
    gpio_set_drive_capability((gpio_num_t)MYST, GPIO_DRIVE_CAP_1);
    gpio_set_drive_capability((gpio_num_t)FIRST_LINE_MARKER, GPIO_DRIVE_CAP_1);

}

void CasioLCD::BusTristate() {
    pinMode(DISPLAYEN ,INPUT);
    pinMode(CLOCK ,INPUT);
    pinMode(SHIFT ,INPUT);
    pinMode(MYST ,INPUT);
    pinMode(FIRST_LINE_MARKER ,INPUT);
    SetBusTristate(true);
    pinMode(RESET, INPUT);
}

void CasioLCD::TakeoverBus() {
    digitalWrite(DISPLAYEN, HIGH);
    digitalWrite(RESET, LOW);
    digitalWrite(CLOCK, HIGH);
    digitalWrite(SHIFT, HIGH);
    digitalWrite(MYST, HIGH);
    digitalWrite(FIRST_LINE_MARKER, LOW);

    pinMode(DISPLAYEN ,OUTPUT);
    pinMode(RESET, OUTPUT);
    pinMode(CLOCK ,OUTPUT);
    pinMode(SHIFT ,OUTPUT);
    pinMode(MYST ,OUTPUT);
    pinMode(FIRST_LINE_MARKER, OUTPUT);
}

void CasioLCD::Writeframebuffer() {
    setCpuFrequencyMhz(240);
    pinMode(CLOCK ,OUTPUT);
    pinMode(SHIFT ,OUTPUT);
    pinMode(MYST ,OUTPUT);
    pinMode(FIRST_LINE_MARKER, OUTPUT);
    int x = 0;
    BeforeWrite();
    BeginWrite(0);

    //The display has 64 rows
    for(int i = 0; i < 64; i++) {
        //Each Row gets 8x packets of 24xData/8bit which results in 192, our x resolution 
        for(int j = 0; j < 8; j++) {
            //First 8 bits
            FBtoBus(x, i);
            WriteByte();
            x += 8;
            //Second 8 bits
            FBtoBus(x, i);
            WriteByte();
            x += 8;
            //3rd 8 bits
            FBtoBus(x, i);
            WriteByte();
            x += 8;
        }
        x = 0;
    }
    pinMode(CLOCK ,INPUT_PULLUP);
    pinMode(SHIFT ,INPUT_PULLUP);
    pinMode(MYST ,INPUT_PULLUP);
    pinMode(FIRST_LINE_MARKER ,INPUT_PULLDOWN);
    setCpuFrequencyMhz(10);
}

void CasioLCD::FBtoBus(int x, int i) {
    SetBusTristate(false);
    SetBus(
        framebuffer[x + 0][i],
        framebuffer[x + 1][i],
        framebuffer[x + 2][i],
        framebuffer[x + 3][i],
        framebuffer[x + 4][i],
        framebuffer[x + 5][i],
        framebuffer[x + 6][i],
        framebuffer[x + 7][i]
    );
}

//Writes the byte that is currently set on the bus into the lcd controller / onto the lcd
void CasioLCD::WriteByte() {
    WriteGPIO(CLOCK, LOW);
    WriteGPIO(SHIFT, LOW);
    ets_delay_us(2);
    //SetBus(255);
    SetBusTristate(true);
    WriteGPIO(CLOCK, HIGH);
    WriteGPIO(SHIFT, HIGH);
}

void CasioLCD::BeginWrite(int offset) {
    SetBusTristate(false);
    SetBus(0);
    WriteGPIO(FIRST_LINE_MARKER, HIGH);
    WriteGPIO(SHIFT, LOW);
    WriteGPIO(FIRST_LINE_MARKER, LOW);
    WriteGPIO(SHIFT, HIGH);
    WriteGPIO(CLOCK, LOW);
    //ets_delay_us(1);
    //CHECKME: Pixels get offset when you pulse _shift?
    for(int i = 0; i<offset; i++) {
        WriteGPIO(SHIFT, HIGH);
        ets_delay_us(1);
        WriteGPIO(SHIFT, LOW);
        ets_delay_us(1);
    }
    WriteGPIO(CLOCK, HIGH);
    WriteGPIO(SHIFT, HIGH);
    SetBusTristate(true);
}

//Resets the position in the lcd controller to 0x0
void CasioLCD::ResetPosition() {
    SetBus(0);
    WriteGPIO(CLOCK, LOW);
    WriteGPIO(SHIFT, LOW);
    WriteGPIO(MYST, LOW);
    ets_delay_us(1);
    WriteGPIO(MYST, HIGH);
    ets_delay_us(2);
    SetBus(255);
    WriteGPIO(CLOCK, HIGH);
    WriteGPIO(SHIFT, HIGH);
}

//Sets databus high, like me (lol)
void CasioLCD::ResetBus() {
    SetBus(255);
}
//This replicates what gets sent to the lcd before any data gets written
void CasioLCD::BeforeWrite() {
    SetBusTristate(false);
    SetBus(130); //10000010
    //ets_delay_us(4);
    WriteGPIO(SHIFT, LOW);
    SetBus(20); //00010100
    //ets_delay_us(3);
    WriteGPIO(CLOCK, LOW);
    //ets_delay_us(4);
    WriteGPIO(SHIFT, HIGH);
    WriteGPIO(CLOCK, HIGH);
    //ets_delay_us(4);
    //ResetBus();
    SetBusTristate(true);
}

//ESP32 code
void CasioLCD::SetBus(uint8_t bus) {
    digitalWrite(D0,((bus >> 7)  & 0x01));
    digitalWrite(D1,((bus >> 6)  & 0x01));
    digitalWrite(D2,((bus >> 5)  & 0x01));
    digitalWrite(D3,((bus >> 4)  & 0x01));
    digitalWrite(D4,((bus >> 3)  & 0x01));
    digitalWrite(D5,((bus >> 2)  & 0x01));
    digitalWrite(D6,((bus >> 1)  & 0x01));
    digitalWrite(D7,((bus >> 0)  & 0x01));

    //TODO: Use w1tc to set corresponding pins to 0 :P
    //Gpio register
    /*REG_WRITE(GPIO_OUT_W1TC_REG, 0xFFFFFFFF);

    uint32_t reg = 0;

    reg |= ((bus >> 0)  & 0x01) << D0;
    reg |= ((bus >> 1)  & 0x01) << D1;
    reg |= ((bus >> 2)  & 0x01) << D2;
    reg |= ((bus >> 3)  & 0x01) << D3;
    reg |= ((bus >> 4)  & 0x01) << D4;
    reg |= ((bus >> 5)  & 0x01) << D5;
    reg |= ((bus >> 6)  & 0x01) << D6;
    reg |= ((bus >> 7)  & 0x01) << D7;

    //Set values of W1TS_REG (gpio 0-32)
    REG_WRITE(GPIO_OUT_W1TS_REG, reg);
    Serial.print(reg);*/
    
}
void CasioLCD::SetBus(int a, int b, int c, int d, int e, int f, int g, int h) {
    digitalWrite(D0, a);
    digitalWrite(D1,b);
    digitalWrite(D2,c);
    digitalWrite(D3,d);
    digitalWrite(D4,e);
    digitalWrite(D5,f);
    digitalWrite(D6,g);
    digitalWrite(D7,h);
}

void CasioLCD::SetBusTristate(bool state) {
  if(state) {
    pinMode(D0 ,INPUT_PULLUP);
    pinMode(D1 ,INPUT_PULLUP);
    pinMode(D2 ,INPUT_PULLUP);
    pinMode(D3 ,INPUT_PULLUP);
    pinMode(D4 ,INPUT_PULLUP);
    pinMode(D5 ,INPUT_PULLUP);
    pinMode(D6 ,INPUT_PULLUP);
    pinMode(D7 ,INPUT_PULLUP);
  }
  else {
    pinMode(D0 ,OUTPUT);
    pinMode(D1 ,OUTPUT);
    pinMode(D2 ,OUTPUT);
    pinMode(D3 ,OUTPUT);
    pinMode(D4 ,OUTPUT);
    pinMode(D5 ,OUTPUT);
    pinMode(D6 ,OUTPUT);
    pinMode(D7 ,OUTPUT);
  }
}

//ESP32 code
void CasioLCD::WriteGPIO(int pin, bool state) {
    //TODO: CODE
    //Read gpio register

    digitalWrite(pin, state);
    //Write gpio register with mask
    //int reg = 0;
    //reg |= 1 << pin;

    //Push register 
    //REG_WRITE(GPIO_OUT_W1TS_REG, 0x00010001);
}

//need to reverse engineer
void CasioLCD::Init() {
    SetBus(0b00000001);
    WriteGPIO(SHIFT, LOW);
    WriteGPIO(SHIFT, HIGH);
    SetBus(0b00000001);
    WriteGPIO(CLOCK, LOW);
    WriteGPIO(SHIFT, LOW);
    WriteGPIO(CLOCK,HIGH);
    WriteGPIO(SHIFT, HIGH);
    SetBus(255);
}

//Need to reverse engineer
void CasioLCD::setContrast(uint8_t contrast) {
    SetBusTristate(false);
    //SetBus(0b01111111); //Sets contrast 

    WriteGPIO(SHIFT, LOW);
    WriteGPIO(CLOCK, LOW);
    WriteGPIO(MYST, LOW);
    //SetBus(0b10000001); TURNS OFF DISPLAY
    WriteGPIO(SHIFT, LOW);
    WriteGPIO(SHIFT, HIGH);

    //First the value(00h-2Ah) gets sent with CLK pulsed low
    SetBus(contrast);
    WriteGPIO(SHIFT, LOW);
    WriteGPIO(SHIFT, HIGH);
    //WriteGPIO(CLOCK, LOW);
    //ets_delay_us(1);
    //WriteGPIO(SHIFT, LOW);
    //ets_delay_us(1);
    //WriteGPIO(SHIFT, HIGH);
    //All high
    WriteGPIO(CLOCK, HIGH);
    WriteGPIO(SHIFT, HIGH);
    WriteGPIO(MYST, HIGH);
    //ets_delay_us(2);
    //SetBus(255);
    SetBusTristate(true);
    ets_delay_us(1);
}

void CasioLCD::setReset(bool state) {
    digitalWrite(RESET, state);
}



void CasioLCD::DrawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
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
        framebuffer[x+i][y] = color;
    }
  }
}

void CasioLCD::ClearFB() {
    for(int x = 0; x<192; x++) {
    for(int y = 0; y<64;y++) {
      framebuffer[x][y] = 0;
    }
  }
}


void CasioLCD::textFont(const Font& which)
{
  _font = &which;
}

int CasioLCD::textFontWidth() const
{
  return (_font ? _font->width : 0);
}

int CasioLCD::textFontHeight() const
{
  return (_font ? _font->height : 0);
}

void CasioLCD::text(const char* str, int x, int y, bool color)
{
  if (!_font) {
    return;
  }

  while (*str) {
    int c = *str++;

    if (c == '\n') {
      y += _font->height;
      x = 0;
    } else if (c == '\r') {
      x = 0;
    } else if (c == 0xc2 || c == 0xc3) {
      // drop
    } else {
      const uint8_t* b = _font->data[c];

      if (b == NULL) {
        b =  _font->data[0x20];
      }

      if (b) {
        bitmap(b, x, y, _font->width, _font->height, color);
      }

      x += _font->width;
    }
  }
}

void CasioLCD::text(const char* str, int x, int y, bool color, bool onlyfirstline)
{
  if (!_font) {
    return;
  }

  while (*str) {
    int c = *str++;

    if (c == '\n') {
      if(onlyfirstline) return;
      y += _font->height;
      x = 0;
    } else if (c == '\r') {
      x = 0;
    } else if (c == 0xc2 || c == 0xc3) {
      // drop
    } else {
      const uint8_t* b = _font->data[c];

      if (b == NULL) {
        b =  _font->data[0x20];
      }

      if (b) {
        bitmap(b, x, y, _font->width, _font->height, color);
      }

      x += _font->width;
    }
  }
}

void CasioLCD::rect(int x, int y, int width, int height, bool _fill)
{
  int x1 = x;
  int y1 = y;
  int x2 = x1 + width - 1;
  int y2 = y1 + height - 1;

  for (x = x1; x <= x2; x++) {
    for (y = y1; y <= y2; y++) {
      if ((x == x1 || x == x2 || y == y1 || y == y2) && !_fill) {
        // stroke
        framebuffer[x][y] = true;
      } else if (_fill) {
        // fill
        framebuffer[x][y] = true;
      }
    }
  }
}

void CasioLCD::line(int x1, int y1, int x2, int y2)
{

  if (x1 == x2) {
    for (int y = y1; y <= y2; y++) {
        framebuffer[x1][y] = true;
    }
  } else if (y1 == y2) {
    for (int x = x1; x <= x2; x++) {
        framebuffer[x][y1] = true;
    }
  } else if (abs(y2 - y1) < abs(x2 - x1)) {
    if (x1 > x2) {
      lineLow(x2, y2, x1, y1);
    } else {
      lineLow(x1, y1, x2, y2);
    }
  } else {
    if (y1 > y2) {
      lineHigh(x2, y2, x1, y1);
    } else {
      lineHigh(x1, y1, x2, y2);
    }
  }
}

void CasioLCD::lineHigh(int x1, int y1, int x2, int y2)
{
  int dx = x2 - x1;
  int dy = y2 - y1;
  int xi = 1;

  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }
  
  int D = 2 * dx - dy;
  int x = x1;

  for (int y = y1; y <= y2; y++) {
    framebuffer[x][y] = true;
    if (D > 0) {
       x += xi;
       D -= 2 * dy;
    }

    D += 2 * dx;
  }
}

void CasioLCD::lineLow(int x1, int y1, int x2, int y2)
{
  int dx = x2 - x1;
  int dy = y2 - y1;
  int yi = 1;

  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }

  int D = 2 * dy - dx;
  int y = y1;

  for (int x = x1; x <= x2; x++) {
    framebuffer[x][y] = true;
    if (D > 0) {
      y += yi;
      D -= (2 * dx);
    }

    D += (2 * dy);
  }
}

void CasioLCD::bitmap(const uint8_t* data, int x, int y, int width, int height, bool color)
{
  if ((data == NULL) || ((x + width) < 0) || ((y + height) < 0) || (x >= 192) || (y >= 64) || ((x+width) >= 192) || (y+height) >= 64) {
    // offscreen
    //Serial.println("Bitmap out of screen");
    return;
  }
  if(y == 1) {color = false;} //HACK: invert first line

  for (int j = 0; j < height; j++) {
    uint8_t b = data[j];

    for (int i = 0; i < width; i++) {
      if (b & (1 << (7 - i))) {
        framebuffer[x+i][y+j] = color;
      } else {
        framebuffer[x+i][y+j] = !color;
      }
    }
  }
}