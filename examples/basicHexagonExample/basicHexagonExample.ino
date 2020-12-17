#include <Lights.h>

// A short example where all the panels in a hexagon
// spin through the color wheel for 30 seconds
// then turn white

/* Use an array of "TriPanelData" to send the information
  about each panel to the Hexagon they're apart of.
  The values 5 values for each panel:
    The pin the panel's lights are connected to
    The number of LEDs within the panel
    The location of the panel within the hexagon 
        M = Middle
        L = Left
        R = Right

        T = Top
        B = Bottom
    The direction the LEDs are wrapped within the panel
    The corner where the LEDs start within the panel
*/ 
TriPanelData panelData[] = {
  TriPanelData(5, 80, CL_LT, CW, CL_RB),
  TriPanelData(6, 80, CL_MT, CW, CL_MB),
  TriPanelData(7, 80, CL_RT, CCW, CL_LB),
  TriPanelData(8, 80, CL_RB, CCW, CL_RB),
  TriPanelData(9, 80, CL_MB, CW, CL_RB),
  TriPanelData(10, 80, CL_LB, CCW, CL_RT)
};

Hexagon hex(panelData);

void setup() {
  hex.begin();
  hex.rainbowTimed(30000);
  hex.setColor(LED::Color(255, 255, 255), 30000);
}

void loop() {
  hex.show();
}