#ifndef MILO_HEXAGON
#define MILO_HEXAGON

#include "Lights.h"
#ifndef ARDUINO
Serial Serial;
unsigned long micros(void) { return time_length<std::micro>(); }
unsigned long millis(void) { return time_length<std::milli>(); }
#endif

MilliSec currentTime = 0;

/*
  1 2 3
  🔺🔻🔺
  🔻🔺🔻
  6 5 4
*/
Hexagon::Hexagon() {
  static TriPanel LT(5, 80, CL_LT, CW, CL_RB);
  static TriPanel MT(10, 80, CL_MT, CW, CL_MB);
  static TriPanel RT(6, 80, CL_RT, CCW, CL_LB);
  static TriPanel RB(11, 80, CL_RB, CCW, CL_RB);
  static TriPanel MB(12, 80, CL_MB, CW, CL_RB);
  static TriPanel LB(9, 80, CL_LB, CCW, CL_RT);

  panels.push_back(&LT);
  panels.push_back(&MT);
  panels.push_back(&RT);
  panels.push_back(&RB);
  panels.push_back(&MB);
  panels.push_back(&LB);
}

Hexagon::Hexagon(TriPanelData pd[])  {
  std::vector<bool> panelLocationCheck(6, false);

  for (int i = 0; i < 6; i++) {
    CornerLocation overallLocation = pd[i].local;
    if (panelLocationCheck[overallLocation]) {
      Serial.print(
        "You supplied two of more panels that have the same location as "
        "panel ");
      Serial.println(i);
      return;
    }

    panelLocationCheck[overallLocation] = true;
  }

  for (int i = 0; i < 6; i++) {
    if (!panelLocationCheck[i]) {
      std::string missingLocation = "";
      switch (i) {
        case CL_MT:
          missingLocation = "MIDDLE TOP";
          break;
        case CL_MB:
          missingLocation = "MIDDLE BOTTOM";
          break;
        case CL_LT:
          missingLocation = "LEFT TOP";
          break;
        case CL_LB:
          missingLocation = "LEFT BOTTOM";
          break;
        case CL_RT:
          missingLocation = "RIGHT TOP";
          break;
        case CL_RB:
          missingLocation = "RIGHT BOTTOM";
          break;
      }
      Serial.print("You are missing the ");
      Serial.print(missingLocation.c_str());
      Serial.println(" panel.");
    }
  }

  static TriPanel a(pd[0].pin, pd[0].numLeds, pd[0].local, pd[0].spin,
    pd[0].startLocation);
  static TriPanel b(pd[1].pin, pd[1].numLeds, pd[1].local, pd[1].spin,
    pd[1].startLocation);
  static TriPanel c(pd[2].pin, pd[2].numLeds, pd[2].local, pd[2].spin,
    pd[2].startLocation);
  static TriPanel d(pd[3].pin, pd[3].numLeds, pd[3].local, pd[3].spin,
    pd[3].startLocation);
  static TriPanel e(pd[4].pin, pd[4].numLeds, pd[4].local, pd[4].spin,
    pd[4].startLocation);
  static TriPanel f(pd[5].pin, pd[5].numLeds, pd[5].local, pd[5].spin,
    pd[5].startLocation);

  panels.push_back(&a);
  panels.push_back(&b);
  panels.push_back(&c);
  panels.push_back(&d);
  panels.push_back(&e);
  panels.push_back(&f);
}

Hexagon::~Hexagon() {}

void Hexagon::playFunctionSequence() {
  while (!functionSequence.empty()) {
    const MilliSec waitTime = functionSequence.front().first;
    const std::function<void()> fn = functionSequence.front().second;
    const bool timesUp = waitTime < currentTime;
    if (timesUp) {
      fn();
      functionSequence.pop_front();
    }
    else if (!timesUp) {
      return;
    }
  }
}

template <class Function>
void Hexagon::forEachPanel(Function fn) {
  for (TriPanel* panel : panels) {
    fn(panel);
  }
}

void Hexagon::breath(
  uint8_t maxBrightness, MilliSec fadeSpeed, LEDColor color) {
  forEachPanel(
    [=](TriPanel* panel) { panel->breath(maxBrightness, fadeSpeed, color); });
}

void Hexagon::colorShift(MilliSec timeDelay) {
  std::vector<LEDColor> lastPanelColor = panels[5]->getColor();

  for (int i = 5; i; i--) {
    panels[i]->setColor(panels[i - 1]->getColor(), timeDelay);
  }

  panels[0]->setColor(lastPanelColor);
}

void Hexagon::rainbowTimed(MilliSec duration, uint8_t speed) {
  forEachPanel([=](TriPanel* panel) { panel->rainbowTimed(duration, speed); });
}

void Hexagon::rainbow(double loops, uint8_t speed) {
  forEachPanel([=](TriPanel* panel) { panel->rainbow(loops, speed); });
}

void Hexagon::runFunctionLater(std::function<void()> fn, MilliSec timeDelay) {
  timeDelay += currentTime;
  if (functionSequence.empty()) {
    functionSequence.push_back({timeDelay, fn});
    return;
  }

  for (auto it = functionSequence.begin(); it != functionSequence.end(); ++it) {
    if (it->first >= timeDelay) {
      functionSequence.insert(it, {timeDelay, fn});
      return;
    }
  }

  functionSequence.push_back({timeDelay, fn});
}

void Hexagon::clearFunctions() {
  forEachPanel([](TriPanel* panel) { panel->clearFunctions(); });
  functionSequence.clear();
}

void Hexagon::begin() {
  currentTime = millis();
  forEachPanel([](TriPanel* panel) { panel->begin(); });
  setColor(LED::Color(0, 0, 0));
}

void Hexagon::setBrightness(uint8_t b) {
  forEachPanel([b](TriPanel* panel) { panel->setBrightness(b); });
}

void Hexagon::setColor(LEDColor color, MilliSec timeDelay) {
  forEachPanel([=](TriPanel* panel) { panel->setColor(color, timeDelay); });
}

void Hexagon::show() {
  playFunctionSequence();
  forEachPanel([](TriPanel* panel) { panel->show(); });
  currentTime = millis();
}

#endif  // MILO_HEXAGON
