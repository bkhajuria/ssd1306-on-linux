/*
* This utility wraps the linux i2c subsystem API to send commands and data to 
* the SSD1306 128x64 OLED Display module.
*/
#ifndef I2CSEND_H
#define I2CSEND_H



/*SSD1306 Commands used in this implementation. For entire list of commands refer to the documentation for SSD1306.*/
#define SSD1306_SET_CONTRAST                0x81
#define SSD1306_SET_DISPLAY_ON_RAM          0xa4
#define SSD1306_SET_DISPLAY_NORMAL          0xa6
#define SSD1306_DISPLAY_OFF                 0xae
#define SSD1306_DISPLAY_ON                  0xaf
#define SSD1306_SET_MEM_ADDR_MODE           0x20
#define SSD1306_SET_COLUMN                  0x21
#define SSD1306_SET_PAGE                    0x22
#define SSD1306_SET_SEGMENT_REMAP127        0xa1
#define SSD1306_SET_CLK_DR_OSC_FRQ          0xd5
#define SSD1306_SET_MUX_RATIO               0xa8
#define SSD1306_SET_DISPLAY_OFFSET          0xd3
#define SSD1306_SET_DISPLAY_START_FIRST     0x40
#define SSD1306_SET_CHARGE_PUMP             0x8d
#define SSD1306_SET_COM_OUT_DESCENDING      0xc8
#define SSD1306_SET_COM_CONFIG              0xda
#define SSD1306_SET_PRE_CHARGE_PERIOD       0xd9
#define SSD1306_SET_VCOMH_DESELECT_LEVEL    0xdb
#define SSD1306_SET_SCROLL_OFF              0x2e

/*Initialization values*/
#define CONTRAST                0xff
#define ADDR_MODE_HZ            0x00
#define CLK_DR_OSC_FRQ          0x80
#define MUX64                   0x3f
#define OFFSET_VALUE            0x00
#define ENABLE_PUMP             0x14
#define COM_CONFIG              0x12
#define PERIOD                  0x22
#define VCOMH_DESELECT_LEVEL    0x20

#define DISPLAY_BEGIN_COL       0x00
#define DISPLAY_END_COL         0x7f
#define DISPLAY_BEGIN_PAGE      0x00
#define DISPLAY_END_PAGE        0x07




int sendCommand(unsigned char command);
int sendDataByte(unsigned char byte);
int sendDataBlock(unsigned char *block,const unsigned int size);

#endif