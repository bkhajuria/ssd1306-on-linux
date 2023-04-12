/*
* This utility wraps the linux i2c subsystem API to send commands and data to 
* the SSD1306 128x64 OLED Display module.
*/
#include<linux/i2c.h>
#include "i2cSend.h"

extern struct i2c_client *ssd1306_i2c_client;

//Send command to SSD1306
int sendCommand(unsigned char command){
    unsigned char buf[2];
    buf[0]=0x00;
    buf[1]=command;
    return i2c_master_send(ssd1306_i2c_client,buf,2);
}

//Send a byte of data to SSD1306
int sendDataByte(unsigned char byte){
    unsigned char buf[2];
    buf[0]=0x40;
    buf[1]=byte;
    return i2c_master_send(ssd1306_i2c_client,buf,2);
}

//Send a block of data to SSD1306
//Memory pointed by the block argument is to be freed by the callee.
int sendDataBlock(unsigned char *block,const unsigned int size){
    unsigned char *buf =kmalloc(size*(sizeof(unsigned char))+1,GFP_KERNEL);
    int ret;
    buf[0]=0x40;
    memcpy(buf+1,block,size);
    ret=i2c_master_send(ssd1306_i2c_client,buf,size+1);
    kfree(buf);
    return ret;
}