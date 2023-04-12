#!/bin/bash
## This is a simple bash script to display system temperature value on 
## SSD1306 display module. 
## This is specific to Orange Pi 4 LTS device. Refer to your device's 
## documentation and make changes.
## This script is run as a service using systemd and is started after 
## the system boots up. (Refer to user/tempDisplay.service) 

sleep 10    ##Give enough time for the driver to load and initialize after start up.
while [ true ]
do
    cat /sys/class/thermal/thermal_zone0/temp > /dev/ssd1306temp    ##Read temperature and write to device file after every second.
    sleep 1
done
