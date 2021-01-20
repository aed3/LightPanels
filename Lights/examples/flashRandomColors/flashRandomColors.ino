#include <Lights.h>

MilliSec timePassed = 0;

TriPanelData panelData = {
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
}

void loop() {
  TriPanel* panel = hex.panels[rand() % 6];
  uint8_t r = rand() % 255, g = rand() % 255, b = rand() % 255;
  
  hex.runFunctionLater(
    [panel, r, g, b]() {
      panel->setColor(LED::Color(r, g, b));
    panel->setBrightness(0);
    panel->clearFunctions();
    panel->fadeIn(128, true, 250);
    hex.runFunctionLater([panel]() { panel->fadeOut(0, true, 1500); }, 250);
  },
  timePassed += rand() % 1000);
 
  hex.show();
}
