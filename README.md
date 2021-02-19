# roku-ecp-example

This is an esp-idf based example project for controlling a Roku TV with an ESP32.
To get started, clone this repository and execute 'idf.py menuconfig' to enter WiFi network details.
Then, flash the ESP32 with 'idf.py -p <YOUR_SERIAL_PORT> flash monitor'

The program will automatically scan the wifi network for Roku devices, and will present you with the devices found.
In the serial monitor, press the index of the device you would like to control.
After choosing a device, the program will display a list of keypress commands you can send to your device. Press 'h' to display this menu again.
Now, you will be able to navigate Up, Down, Left, Right, Back, Home and Select, as well as change the Volume, and input on the TV to HDMI1, 2, or 3.

You can add the roku.c and roku.h files to your own project and follow this example code to make your own ESP32 Roku Remote!
