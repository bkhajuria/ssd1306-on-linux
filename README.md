# **ssd1306-on-linux**
A simple kernel module to drive I2C-based OLED device SSD1306 and a light-weight library to render digits and symbols.

This is developed and tested on [**Orange Pi 4 LTS**](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/details/orange-pi-4-LTS.html) running **Debian GNU/Linux 11 (bullseye)**. 
However, it can be easily ported to other platforms (that have I2C hardware support) like **Raspberry Pi** by making minor changes.

![ssd1306-on-linux](/assets/images/ssd1306tempdisplay.jpg)

**What's included:**
1. A linux module to drive SSD1306. [[ssd1306temp.c](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/ssd1306temp.c)]
2. A simple rendering library to render digits and symbols on SSD1306. [[digitRenderer](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/digitRenderer)]
3. A Linux I2C subsystem *wrapper* to communicate with SSD1306. [[i2cSend](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/i2cSend)]
4. A utility to create new symbols to display on SSD1306. [[mapGenerator](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/mapGenerator)]
5. A simple standalone service to display system temperature on SSD1306 using systemd. [[user](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/user)]




## Wiring (for Orange Pi 4 LTS)
![Wiring Diagram for Orange Pi 4 LTS and SSD1306](/assets/images/wiring.png)


## Build and Install the Driver (kernel module)

1. Clone the repository to the desired directory on your system by running  
`git clone https://github.com/bkhajuria/ssd1306-on-linux.git` . 

2. Make sure you have the appropriate kernel headers installed to build modules.  
[Installing Kernel Headers](https://www.tecmint.com/install-kernel-headers-in-ubuntu-and-debian/)

    NOTE : Kernel Headers are not readily available for Orange Pi 4 LTS. You need to build them using the [official build system for Orange Pi](https://github.com/orangepi-xunlong/orangepi-build).

    Check the [user manual for Orange Pi 4 LTS](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/service-and-support/Orange-pi-4-LTS.html) for setting up the build system and building kernel headers. 

3. Open the directory where the repository is cloned on the terminal and run ` make `.
4. Upon successful build, object file `ssdtemp1306_dev.ko` will be created.
5. Install the module using `sudo insmod ssdtemp1306_dev.ko` .
6.   Run `dmesg | tail` to check if installation is done correctly.
        ```
        [ xxxx.xxxxxx] Initializing SSD1306 Temerature Display Module...
        [ xxxx.xxxxxx] Creating device file for SSD1306...
        [ xxxx.xxxxxx] Success! : Device numer Major: 234 Minor 0
        [ xxxx.xxxxxx] SSD1306 display ready!
        [ xxxx.xxxxxx] Driver added for SSD1306 Temperature Display
        ```

## Using the Driver to render digits/symbols on the SSD1306 display

The kernel module that we built exposes a device node `/dev/ssd1306temp` that can be used by userspace applications to render digits and symbols on the SSD1306 dsplay module.

1. run `sudo chmod 666 /dev/ssd1306temp` to make the device node writeable by userspace applications.
2. To test the display, write a numeric value to the device node as follows:
    
    `echo "23000" > /dev/ssd1306temp `
3. You can also run a rendering test by writing 'R' or 'r' to the device node.
    
    `echo "R" > /dev/ssd1306temp `

    To stop the render test and clear the display, write 'X' or 'x' to the device node.
    
    `echo "X" > /dev/ssd1306temp `

4. If on Orange Pi 4 LTS, run the script [user/tempDisplay.sh](https://github.com/bkhajuria/ssd1306-on-linux/blob/main/user/tempDisplay.sh) to  display the system temperature on the display every second.

## Running a temperature display service (Orange Pi 4 LTS)
We will run the shell script  [user/tempDisplay.sh](https://github.com/bkhajuria/ssd1306-on-linux/blob/main/user/tempDisplay.sh) as a service using [systemd](https://www.freedesktop.org/wiki/Software/systemd/).
This script reads the system temperature and displays it on the SSD1306 display module every second. The service starts after the system boots up.

To set up the service, we have to:
1. **Insert the driver module at system boot up.**

    To insert our driver module at system boot we have to add the name of our driver to the file `/etc/modules`. Add the name of the driver (without the *.ko* extension) in a new line. NOTE:You may require sudo permissions to write to this file. Then, we have to copy our driver object file `ssd1306temp_dev.ko` to a suitable folder within `/lib/modules/$(uname-r)/kernel/drivers` directory. Let's copy it to `/lib/modules/5.10.43/kernel/drivers/misc/ssd1306temp/` (though you can choose to copy it anywhere within the *drivers* directory) . After copying the object file, run `depmod` within the `ssd1306temp` directory (Learn more about [depmod](https://wiki.debian.org/depmod)). With this arrangement, the module will be automatically loaded during system boot. To test, use `lsmod` after boot up.  
2. **Add a [udev rule](https://opensource.com/article/18/11/udev) to make the device node `/dev/ssd1306temp` writable by user applications after the module is loaded.**
    
    By default, the device file that is generated when we insert our module is read-only, so we need to write a udev rule that changes its permissions to allow user applications to write onto the device file.As such, we dont have to explicitly use chmod to set permissions for the device node everytime we insert the module.

    - Copy the file [user/99-perm.rules](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/user/99-perm.rules) to  `/etc/udev/rules.d/`. This file contains the udev rule to set permissions of our device node.
    - Run `sudo udevadm control –reload-rules`.
    - Run `sudo udevadm trigger`.

    Check the permissions of the device node `/dev/ssd1306temp`. It should be updated. 

3. **Add a simple [systemd](https://www.freedesktop.org/wiki/Software/systemd/) service.**

    The file [user/tempDisplay.service](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/user/tempDisplay.service) specifies a simple systemd service that runs the shell script [user/tempDisplay.sh](https://github.com/bkhajuria/ssd1306-on-linux/blob/main/user/tempDisplay.sh).
    - Open the file [user/tempDisplay.service](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/user/tempDisplay.service) and edit the *ExecStart* path to where the shell script [`tempDisplay.sh`](https://github.com/bkhajuria/ssd1306-on-linux/blob/main/user/tempDisplay.sh) is located on your system.
    - Now copy the file [user/tempDisplay.service](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/user/tempDisplay.service) to `/etc/systemd/system/` directory.
    - Run `sudo systemctl daemon-reload`.
    - Run `sudo systemctl enable tempDisplay.service`
    - Run `sudo systemctl start tempDisplay.service`

Once you are done with these three steps, perform a reboot. Make sure the SSD1306 device is properly connected to the system and the connections are tight. Upon reboot , the module will be inserted, the service will start and we can see the temperature being displayed and updated on the SSD1306 device every second.
![Demo](/assets/images/ssd1306TempDisplayService.gif) 

## Adding new Symbols/Characters
New symbols can be created using the [mapGenerator](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/mapGenerator) utility.
![Add a new Symbol](/assets/images/newSymbol.png)
Below illustration adds and displays the character **'K'** :
- In [mapGen.c](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/mapGenerator/mapGen.c), Add a new 16*16 matrix to the the `symbolMap` array that represents charcter 'K'.
- Increase the value of `NUM_SYMBOLS` macro by 1 (since we added one symbol).
- Build by running `make` in the *mapGenerator* directory.
- Run the generated executable as `./mapGen`.
- It will print the bitmaps for all the digits and symbols.
- Copy the 32-byte bitmap generated for the symbol 'K'. (It should be the last one if you added the 16*16 matrix for 'K' at the end of `symbolMap` array.)
- Add the bitmap to the `symbolMap` array in [digitRenderer.c](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/digitRenderer/digitRenderer.c). Increase the value of `NUM_SYMBOLS` macro by 1.
- Add enumeration for the new character in [digitRenderer.h](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/digitRenderer/digitRenderer.h). 
- To test rendering the new symbol use the `renderSymbol` function. (Refer to `parseAndDisplayTemp` function in [ssd1306temp.c](https://github.com/bkhajuria/ssd1306-on-linux/tree/main/ssd1306temp.c) for usage.)
- Build the module using `make` int the root directory of the repostory. Insert the module and the new symbol should be displayed :) .


## Resources
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [Linux I2C Subsystem](https://docs.kernel.org/i2c/index.html)

