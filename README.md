# park-ranger
An example application for TTL sensor, garage parking assistant with [TS-7553-V2](https://www.embeddedarm.com/products/TS-7553-V2) and [Maxbotix Ultrasonic Rangefinder (HRLV-MaxSonar-EZ1 MB1013)](https://maxbotix.com/Ultrasonic_Sensors/MB1013.htm).  The LCD screen of the TS-7553-V2 enclosure displays range in feet and instructs driver on when to pull forward, slow down, and stop.  The sonar sensor is setup to use TTL-level UART by soldering the jumper on the back of the PCB and connected to */dev/ttymxc6* (Pin 10 on the HD1 header, [see manual]( https://wiki.embeddedarm.com/wiki/TS-7553-V2#UARTs)).

![Image of Breadboard Wiring Diagram](https://github.com/embeddedarm/park-ranger/raw/master/ts-7553-v2-park-ranger-demo-breadboard.jpg)


# Installation
All these steps should be done on the TS-7553-V2 as root.


1. Install cairo dev headers

    ```apt-get install libcairo2-dev```
    
2. Clone repository

    ```git clone https://github.com/embeddedarm/park-ranger.git```
    
3. Change into project directory

    ```cd park-ranger```
  
4. Compile program

    ```make```
  
5. Install to /usr/local/bin/

   ```make install```
  

# Running
A systemd service file has been provided as an example for starting park-ranger automatically, to be copied to */etc/systemd/system/* and enabled using `systemctl enable park-ranger.service`.  This is how you'd do it manually.

1. Ensure keypad drivers are disabled

    ```rmmod gpio_keys```

    ```echo “blacklist gpio_keys” > /etc/modprobe.d/fbdev-blacklist.conf```

2. Ensure LCD screen drivers have been installed

   ```modprobe ts-st7565p-fb```
   
3. Setup LCD screen using helper program

   ```lcd-helper```

4. Run the program

   ```park-ranger```
