/*
* This simple Linux kernel module drives the SSD1306 128x64 OLED Display using I2C protocol.
* It makes use of the Linux I2C Subsystem and a light-weight rendering library "digitRenderer"
* (part of this repository) to render numeric digits and symbols on the display. 
*/

#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/slab.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>

#include<linux/i2c.h>

#include "i2cSend/i2cSend.h"
#include"digitRenderer/digitRenderer.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple 0.96 inch SSD1306 (128x64) OLED Display Driver to display temperature.");
MODULE_AUTHOR("Brij Bhushan Khajuria <bkhajur1@asu.edu>");
MODULE_VERSION("0.1");


//Constructs for Character device
#define DRIVER_NAME "ssd1306temp"
#define DRIVER_CLASS "ssd1306"

static dev_t ssd1306_dev_nr;
static struct class *ssd1306temp;
static struct cdev ssd1306temp_cdev;

static unsigned char buffer[32];

static symbol scale = DEGREE_CELSIUS;   //Default unit for temperature.

/*Display Temperature Value on SSD1306*/
/*Temperature value parsing is specific to Orange Pi 4 LTS Device.If on any other device, check the documentation and make changes accordingly.*/
void parseAndDisplayTemp(unsigned char *buf,const unsigned int size){
    int x=3,y=80;
    renderSymbol(NULLSYM,x,96); //Bugfix: clear unit symbol if it is shifted to right.
    if(buf[0]=='c' || buf[0]=='C'){
        scale=DEGREE_CELSIUS;
        renderSymbol(scale,x,y);
        return;
    }
    else if(buf[0]=='f' || buf[0]=='F'){
        scale=DEGREE_FAHRENHEIT;
        renderSymbol(scale,x,y);
        return;
    }
    else if(buf[0]=='-' || (buf[0]>='0' && buf[0]<='9')){
        int i=size-2; //ignore null character
        int m=1;
        int temp=0;
        bool negative=false;
        if(buf[0]=='-'){
            negative=true;
        }
        while(i>=0 &&(buf[i]>='0' && buf[i]<='9')){
            temp+=(buf[i]-'0')*m;
            m*=10;
            i--;
        }
        if(i>0 && buf[i]!='-'){
            goto invalid;
        }
        temp=temp/1000;
        if(temp/1000000>0){
            temp=99999; //Display MAX value instead of overflowing.
        }
        if(temp/1000 >0){
            y=96;
        }
        renderSymbol(scale,x,y);
        while(temp>0){
            y-=16;
            renderDigit(temp%10,x,y);
            temp/=10;
        }
        if(negative){
            y-=16;
            renderSymbol(MINUS,x,y);
        }

        while(y>=0){    //Clear Stale higher significance digits if number of digits decrease.
            y-=16;
            renderSymbol(NULLSYM,x,y);
        }
        return;
    }
invalid:
    printk(KERN_ERR"Invalid data!");
}

/*Handle write to device file*/
static ssize_t ssd1306tempWrite(struct file * file,const char * user_buffer, size_t count, loff_t *offs){
        int size,discard,delta;
        size = count<=sizeof(buffer)? count : sizeof(buffer);
        discard=copy_from_user(buffer,user_buffer,size);
        delta=size-discard;
        //printk(KERN_ALERT"Temperature Value: %s",buffer);
        parseAndDisplayTemp(buffer,size);
        return delta;
}


/*Handle opening device file - Nothing to do here*/
static int ssd1306tempOpen(struct inode * deviceFile,struct file *instance){
        return 0;
}

/*Handle closing device file - Nothing to do here*/
static int ssd1306tempClose(struct inode * deviceFile,struct file *instance){
        return 0;
}

static struct file_operations ssd1306temp_fOps={
        .owner=THIS_MODULE,
        .open=ssd1306tempOpen,
        .release=ssd1306tempClose,
        .write=ssd1306tempWrite
};

//I2C Adapter -I2c Bus
#define I2C_BUS 8                           //For Orange Pi 4 LTS Device. Checkout documentation for your device for the bus number.
struct i2c_adapter *ssd1306_i2c_adapter;

//I2c Client/Slave -SSD1306
#define CLIENT_NAME "TEMP_DISPLAY"
#define CLIENT_I2C_ADDR 0x3c                //For SSD1306, this is printed on the back side of the display module. Check documentation.
struct i2c_client *ssd1306_i2c_client;

    //I2C board info structure
struct i2c_board_info ssd1306_board_info={
    I2C_BOARD_INFO(CLIENT_NAME,CLIENT_I2C_ADDR)
};

struct i2c_device_id ssd1306_i2c_device_id[]={
    {CLIENT_NAME,0},
    {}
};

    //Probe function for SSD1306
static int ssd1306_i2c_probe(struct i2c_client * client, const struct i2c_device_id *device_id){
    initDisplay(); //Ititialize SSD1306 
    //renderTest(); //Display all digits and symbols
    return 0;
}

    //Remove function for SSD1306
static int ssd1306_i2c_remove(struct i2c_client * client){
    clearDisplay();
    sendCommand(SSD1306_DISPLAY_OFF); //Entire display OFF
    return 0;
}

    //SSD1306 driver structure
struct i2c_driver ssd1306_i2c_driver={
    .driver={
        .name=CLIENT_NAME,
        .owner=THIS_MODULE
    },
    .probe=ssd1306_i2c_probe,
    .remove=ssd1306_i2c_remove,
    .id_table=ssd1306_i2c_device_id

};

static int __init ssd1306temp_init(void){
    int ret=0,addRet;

    printk(KERN_INFO"Creating device file for SSD1306...");
    if(alloc_chrdev_region(&ssd1306_dev_nr,0,1,DRIVER_NAME)<0){
        printk(KERN_ERR"Could not allocate Device number..");
        goto allocErr;
    }

    printk(KERN_INFO"Success! : Device numer Major: %d Minor %d",ssd1306_dev_nr>>20,ssd1306_dev_nr&0xfffff);
    if((ssd1306temp=class_create(THIS_MODULE,DRIVER_CLASS))==NULL){
        printk(KERN_ERR"Couldnot create class for device!\n");
        goto classErr;
    }

    if(device_create(ssd1306temp,NULL,ssd1306_dev_nr,NULL,DRIVER_NAME)==NULL){
        printk("Cannot create device file!");
        goto fileErr;
    }

    cdev_init(&ssd1306temp_cdev,&ssd1306temp_fOps);

    if(cdev_add(&ssd1306temp_cdev,ssd1306_dev_nr,1)<0){
        printk("Device registration failed!");
        goto addErr;
    }

    printk(KERN_INFO"Initializing SSD1306 Temerature Display Module...");
    ssd1306_i2c_adapter = i2c_get_adapter(I2C_BUS);
    if(ssd1306_i2c_adapter!=NULL){
        ssd1306_i2c_client=i2c_new_client_device(ssd1306_i2c_adapter, &ssd1306_board_info);
        if(ssd1306_i2c_client!=NULL){
            addRet=i2c_add_driver(&ssd1306_i2c_driver);
            if(addRet<0){
                ret=addRet;
                goto addDriverError;
            }
            else{
                printk(KERN_INFO"Driver added for SSD1306 Temperature Display");
                goto cleanUp;
            }
        }
        else{
            printk(KERN_ERR"Cannot create new i2c client device!");
            ret=-EINVAL;
            goto clientError;
        }
    }
    else{
        printk(KERN_ERR"Cannot get i2c Adapter for bus:%d",I2C_BUS);
        ret=-ENODEV;
        goto adapterError;
    }

addDriverError:
    i2c_unregister_device(ssd1306_i2c_client);
clientError:
adapterError:
cleanUp:
    if(ssd1306_i2c_adapter!=NULL) 
        i2c_put_adapter(ssd1306_i2c_adapter);
    return ret;

addErr:
    device_destroy(ssd1306temp,ssd1306_dev_nr);
fileErr:
    class_destroy(ssd1306temp);
classErr:
    unregister_chrdev(ssd1306_dev_nr,DRIVER_NAME);
allocErr:
    return -1;

}
static void __exit ssd1306temp_exit(void){
    printk(KERN_ALERT"Removing SSD1306 Temerature Display Module...");
    i2c_unregister_device(ssd1306_i2c_client);
    i2c_del_driver(&ssd1306_i2c_driver);
    cdev_del(&ssd1306temp_cdev);
    device_destroy(ssd1306temp,ssd1306_dev_nr);
    class_destroy(ssd1306temp);
    unregister_chrdev(ssd1306_dev_nr,DRIVER_NAME);
}

module_init(ssd1306temp_init);
module_exit(ssd1306temp_exit);