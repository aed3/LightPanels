#include <Lights.h>

// A short example where one panel changes colors every second
// from red, to green, then to blue in a loop

TriPanel panel(5, 80, CL_LT, CW, CL_RB);
MilliSec timePassed = 0;

void setup() {
  panel.begin();
}

void loop() {
  // When using individual panels and not the full hexagon,
  // make sure to call this line at the start of the loop
  // function so the panels know how many milliseconds 
  // have passed and its animations work properly
  currentTime = millis();


  MilliSec timePassed = currentTime % 3000;
  if (timePassed < 1000) {
    panel.setColor(LED::Color(255, 0, 0));
  }
  else if (timePassed < 2000) {
    panel.setColor(LED::Color(0, 255, 0));
  }
  else if (timePassed < 3000) {
    panel.setColor(LED::Color(0, 0, 255));
  }
 
  panel.show();
}