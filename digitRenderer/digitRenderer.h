/*
* This light-weight library provides functionality to render numeric digits and symbols 
* on the SSD1306 128x64 OLED Display module.
* It makes use of the "mapGen" utility (part of this repository) to create 16x16 bitmaps 
* for digits and symbols.
* Using the magGen utility, it can be extended to include alphabets and other ASCII/UTF 
* characters and render text on the display.
* To communicate with the SSD1306 device, it uses "i2csend" utility (part of this repsitory)
* to send commands and data (bitmaps).
*/

#ifndef DIGIT_RENDERER_H
#define DIGIT_RENDERER_H

typedef enum{
    DEGREE_CELSIUS,
    DEGREE_FAHRENHEIT,
    MINUS,
    NULLSYM,
    KELVIN      //Enumeration for the newly added character 'K'
} symbol; 

/*16x16 pixels font*/
#define FONT_WIDTH              0x0f   //16 columns
#define FONT_HEIGHT             0x01   //2 pages

#define NULL_BYTE               0x00

void renderDigit(int digit,const int x,const int y);
void renderSymbol(symbol sym,const int x,const int y);
void renderTest(void);
void initDisplay(void);
void clearDisplay(void);
#endif