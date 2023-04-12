ccflags-y :=-std=gnu99
obj-m := ssd1306temp_dev.o
ssd1306temp_dev-objs := ssd1306temp.o digitRenderer/digitRenderer.o i2cSend/i2cSend.o

KDIR = /lib/modules/$(shell uname -r)/build
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
