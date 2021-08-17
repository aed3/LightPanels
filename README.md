# DIY Light Panel Library

This is the library used to program custom made triangular light panels that when 6 are made, can be formed into a hexagon as seen [in this video](https://youtu.be/h9Bk7rdjjDE). That video also gives a guide on what you'll need to build it and how to make your own.

This library is split up into 3 sections:

1. Lights: Code that runs the lights directly
2. Camera: Code used on an ESP32-Cam to send the color of what's on a screen to the lights
3. Action: Code for bluetooth communication between the camera and a device controlling the lights

It is assumed that all three sections are running on separate Arduino boards. It is theoretically possible to have all of it run off one (particularly the ESP32 Cam), and may be worth a try doing.

---

## Installation

1. Follow the [instructions from Adafruit's NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel#installation) to install that library
1. Click on the "Code" button in the top right of this page
1. Select "Download Zip" (It's always a good idea to look through the code on this page first to make sure you know what you're downloading)
1. In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library, then select the file you just downloaded


## How to use

There are 4 examples in this library. I'd recommend starting by looking at [basicPanelExample](examples/Lights/basicPanelExample) and [basicHexagonExample](examples/Lights/basicHexagonExample) to see how it works. Once you understand that (and potentially have made light panels of your own), you can see all the functions currently in the library used in [allFunctionTest](examples/Lights/allFunctionTest). This is the file that produced the light file at the beginning of the video linked above.

In the future, I'll add more guidance into how all this works and why it's written the way it is.
