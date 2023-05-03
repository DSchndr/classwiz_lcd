#include <Arduino.h>
#include "lcd.h"
#include "../include/Keypad_MC17.h"
#include <Wire.h>
#include "../include/Pages.h"
#include "../include/Keyboard.h"
#include "../include/bitmap.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include "driver/adc.h"

RTC_DATA_ATTR int bootCount = 0;

#define I2CADDR 0x20
const byte ROWS = 7; 
const byte COLS = 8; 
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {K_Xhoch,  ')',  '+', K_RIGHT, K_DEL,     '*',   K_SIN,     K_DOWN},
  {K_LN,     K_MP,  0 , 0,       0,         0,     K_TAN,     K_X},
  {K_LOG,    K_SD, '-', K_MODE,  K_AC,      '/',   K_COS,     K_SUM},
  {0,        ',',  '=', 0,       K_10hochx, K_ANS, '0',       0},
  {K_WURZEL, K_ENG,'2', K_ALPHA, '8',       '5',   K_ANF,     K_CALC},
  {K_Xquad,  '(',  '3', K_UP,    '9',       '6',   K_Xhmin1,  K_LEFT},
  {K_BRUCH,  K_STO,'1', K_SHIFT, '7',       '4',   K_KLA_MIN, K_OPTN},
};
byte rowPins[ROWS] = {14, 13, 12, 11, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 4, 3, 2, 1, 0}; 

//initialize an instance of class NewKeypad
static Keypad_MC17 CKeypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR); 


static CasioLCD lcd;

void Snake();
void DrawMainMenu();
void HandleMainEnter();
void HandleKey(char key);
void DrawPage(int Page);
void PagesMenu(int offset);

int x = 0;
int y = 0;
int mainmenuindex = 0;
int pagemenuindex = 0;
int menuhandler = 0;
int pageindex = 0;
bool a = false;


void setup() {
    adc_power_off();
    WiFi.mode(WIFI_OFF); 
    btStop();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);
    ++bootCount;
    if (bootCount <= 4) {
        esp_deep_sleep_start();
    } else {
        bootCount = 1;
    }
    Serial.begin(115200);
    Wire.begin( );
    Wire.setClock( 1700000 );
    lcd.BusTristate();
    lcd.textFont(Font_4x6);
    setCpuFrequencyMhz(10); //40mhz xtal
}

void loop() {
    if (digitalRead(ON) == LOW) {
        setCpuFrequencyMhz(240);
        delay(1000);
        lcd.TakeoverBus();
        lcd.Writeframebuffer();
        digitalWrite(17, LOW);
        pinMode(17, OUTPUT);
        pinMode(17, INPUT);
        CKeypad.begin();
        CKeypad.setDebounceTime(1);
        a = true;
        menuhandler = 0;
        DrawMainMenu();
        Serial.write("ON Low");
        setCpuFrequencyMhz(10);
    }
    if (a) {
        char Key = CKeypad.getKey();
        if (Key != NO_KEY){
            setCpuFrequencyMhz(240); //40mhz xtal
            Serial.println(Key);
            HandleKey(Key);
            setCpuFrequencyMhz(10); //40mhz xtal
        }
    }
}

void HandleKey(char key) {
    //Serial.print("HandleKey | menuhandler: ");
    //Serial.println(menuhandler);
    if(menuhandler == 0) {
        switch(key) {
        case K_UP:
            if(mainmenuindex > 0) mainmenuindex--;
            DrawMainMenu();
            break;
        case K_DOWN:
            if(mainmenuindex < 4) mainmenuindex++;
            DrawMainMenu();
            break;
        case '=':
            HandleMainEnter();
            break;
        default:
            break;
        }
        return;
    }
    if(menuhandler == 1) {
        switch(key) {
        case K_LEFT:
            if(pageindex > 0) pageindex--;
            DrawPage(pageindex);
            break;
        case K_RIGHT:
            if(pageindex < PAGECOUNT - 1) pageindex++;
            DrawPage(pageindex);
            break;
        case K_UP:
            menuhandler = 0;
            DrawMainMenu();
            break;
        case K_AC:
        case '=':
            CKeypad.end();
            lcd.BusTristate();
            a = false;
            bootCount = 0;
            esp_deep_sleep_start();
            //HandleMainEnter();
            break;
        default:
            break;
        }
        return;
    }
    if(menuhandler == 2) {
        switch(key) {
        case K_UP:
            if(pagemenuindex > 0) pagemenuindex--;
            PagesMenu(pagemenuindex/10);
            break;
        case K_DOWN:
            if(pagemenuindex < PAGECOUNT - 1) pagemenuindex++;
            PagesMenu(pagemenuindex/10);
            break;
        case '=':
            DrawPage(pagemenuindex);
            break;
        case K_AC:
            menuhandler = 0;
            DrawMainMenu();
        default:
            break;
        }
        return;
    }
}

void HandleMainEnter() {
    //Serial.print("HandleMainEnter: ");
    //Serial.println(mainmenuindex);
    switch(mainmenuindex) {
        case 0: //Exit
            CKeypad.end();
            lcd.BusTristate();
            a = false;
            break;
        case 1:
            CKeypad.end();
            lcd.BusTristate();
            a = false;
            bootCount = 0;
            esp_deep_sleep_start();
            break;
        case 2:
            menuhandler = 1;
            DrawPage(0);
            break;
        case 3:
            menuhandler = 2;
            pagemenuindex = 0;
            PagesMenu(0);
            break;
        case 4: 
            Snake();

        default:
            break;
    }
}

void Snake() {
        /*lcd.ClearFB();
        lcd.rect(0,3,192,64, true);
        lcd.textFont(Font_5x7);
        lcd.text("S N A K E", 70, 30, false);
        lcd.text("---------", 70, 38, false);
        lcd.Writeframebuffer();
        vTaskDelay(2000);*/
        
        int plx = 10;
        int ply = 10;
        CKeypad.setDebounceTime(1);

        while(true) {
            CKeypad.getKeys();
               for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
               {
                     int key = 0;
                    switch (CKeypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                       case PRESSED:
                       case HOLD:
                           key = 1;
                           break;
                    }
                    if(key) {
                    switch(CKeypad.key[i].kchar) {
                        case K_UP:
                            ply--;
                            break;
                        case K_DOWN:
                            ply++;
                            break;
                        case K_LEFT:
                            plx--;
                            break;
                        case K_RIGHT:
                            plx++;
                            break;
                    } }
                 
               }
             
        lcd.ClearFB();
        lcd.DrawBitmap(plx, ply, SPRITE_SNEK, 8, 8, true);
        lcd.DrawBitmap(plx-8, ply, SPRITE_SNEK_BLOK, 8, 8, true);
        lcd.DrawBitmap(plx-8*2, ply, SPRITE_SNEK_BLOK, 8, 8, true);
        lcd.Writeframebuffer();
        }
}

void DrawMainMenu() {
    lcd.ClearFB();
    lcd.textFont(Font_5x7);
    lcd.text("CasioMod v0.1", 60, 2, true);
    lcd.text("1: Exit (Press [ON] to return)", 0, 2 + lcd.textFontHeight(), 0 ^ mainmenuindex );
    lcd.text("2: GO INTO DEEP SLEEP", 0, 2 + lcd.textFontHeight() * 2, 1 ^ mainmenuindex );
    lcd.text("3: Show Pages", 0, 2 + lcd.textFontHeight() * 3, 2 ^ mainmenuindex );
    lcd.text("4: Content", 0, 2 + lcd.textFontHeight() * 4, 3 ^ mainmenuindex );
    lcd.text("5: TestFunc", 0, 2 + lcd.textFontHeight() * 5, 4 ^ mainmenuindex );
    lcd.Writeframebuffer();
}

void DrawPage(int Page) {
    lcd.ClearFB();
    lcd.textFont(Font_4x6);

    lcd.text(Pages[Page], 0, 1, true);

    lcd.Writeframebuffer();
}

void PagesMenu(int offset) {
    lcd.ClearFB();
    lcd.textFont(Font_4x6);
    int cnt = 0;
    for(int i = 10 * offset; (i < 10 * (offset + 1)) && (i <= PAGECOUNT); i++) {
        //char buf[192];
        //const char* str = Pages[i];
        //for(int j = 0; j <= 192; j++) {
        //    int c = *str+j;
        //    if (c == '\n') break;
        //    buf[j] = c;
        //}
        lcd.text(Pages[i], 0, 2 + lcd.textFontHeight()*cnt++, i ^ pagemenuindex, true);
    }
    lcd.Writeframebuffer();
}