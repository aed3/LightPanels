#include <Lights.h>
#include <Wire.h>

void parseSignal(const uint8_t* data, int length);

TriPanelData panelData[] = {TriPanelData(5, 80, CL_LT, CW, CL_RB),
  TriPanelData(10, 80, CL_MT, CW, CL_MB),
  TriPanelData(6, 80, CL_RT, CCW, CL_LB),
  TriPanelData(11, 80, CL_RB, CCW, CL_RB),
  TriPanelData(12, 80, CL_MB, CW, CL_RB),
  TriPanelData(9, 80, CL_LB, CCW, CL_RT)};

const LEDColor panelColors[6] = {
  LED::Color(78, 167, 43),
  LED::Color(255, 221, 0),
  LED::Color(244, 150, 17),
  LED::Color(214, 0, 126),
  LED::Color(3, 145, 212),
  LED::Color(139, 51, 140),
};

Hexagon hex(panelData);
bool bootActice = false;

MilliSec flowToEachPanel(MilliSec timePassed) {
  hex.panels[1]->fillFromCorner(1, panelColors[1], CL_RT, 500);
  hex.runFunctionLater(
    []() { hex.panels[1]->fillToCorner(1, 0, 500); }, timePassed += 600);
  hex.runFunctionLater(
    []() { hex.panels[4]->fillFromCorner(1, panelColors[4], 500); },
    timePassed);
  hex.runFunctionLater(
    []() { hex.panels[4]->fillToCorner(1, 0, CL_RB, 500); }, timePassed += 600);
  hex.runFunctionLater(
    []() { hex.panels[3]->fillFromCorner(1, panelColors[3], CL_MB, 500); },
    timePassed);
  hex.runFunctionLater(
    []() { hex.panels[3]->fillToCorner(1, 0, 500); }, timePassed += 600);
  hex.runFunctionLater(
    []() { hex.panels[0]->fillFromCorner(1, panelColors[0], 500); },
    timePassed);
  hex.runFunctionLater(
    []() { hex.panels[0]->fillToCorner(1, 0, CL_LB, 500); }, timePassed += 600);
  hex.runFunctionLater(
    []() { hex.panels[5]->fillFromCorner(1, panelColors[5], CL_LT, 500); },
    timePassed);
  hex.runFunctionLater(
    []() { hex.panels[5]->fillToCorner(1, 0, 500); }, timePassed += 600);
  hex.runFunctionLater(
    []() { hex.panels[2]->fillFromCorner(1, panelColors[2], 500); },
    timePassed);
  hex.runFunctionLater(
    []() { hex.panels[2]->fillToCorner(1, 0, CL_MT, 500); }, timePassed += 600);
  return timePassed;
}

void bootSequence() {
  MilliSec timePassed = 0;
  bootActice = true;

  timePassed += flowToEachPanel(timePassed);

  MilliSec panelStartTime = timePassed + 600;
  for (size_t i = 0; i < 6; i++) {
    TriPanel* panel = hex.panels[i];
    timePassed = panelStartTime;
    hex.runFunctionLater(
      [i, panel]() { panel->fillFromCorner(1, panelColors[i], 1500); },
      timePassed);
    timePassed += 1500;
  }

  for (size_t j = 1; j <= 18; j++) {
    hex.runFunctionLater([]() { hex.colorShift(); }, timePassed += 1500 / j);
  }

  panelStartTime = timePassed;
  for (size_t i = 0; i < 6; i++) {
    TriPanel* panel = hex.panels[i];
    timePassed = panelStartTime;
    hex.runFunctionLater(
      [panel]() {
        const int outsideSegment = panel->segSideIndex[panel->outerSide];
        panel->segments[outsideSegment].setColor(0);
      },
      timePassed += 1000);

    hex.runFunctionLater(
      [panel]() { panel->colorSpin(1, 254); }, timePassed += 500);

    hex.runFunctionLater(
      [panel]() { panel->fillToCorner(1, LED::Color(0, 0, 0), 500); },
      timePassed += 1000);

    hex.runFunctionLater(
      [panel]() {
        panel->rainbow(0, 250);
        panel->breathe(panel->getBrightness(), 2000);
      },
      timePassed += 1500);
  }
}

void receiveEvent(int bytes) {
  uint8_t data[bytes];
  int taken = 0;

  while (Wire.available()) {
    data[taken] = Wire.read();
    taken++;
  }

  if (bootActice) {
    hex.clearFunctions();
    bootActice = false;
  }

  parseSignal(data);
  /*
    Serial.print("Recieved: ");
    for (size_t i = 0; i < bytes; i++) {
      Serial.print(data[i]);
      Serial.print(", ");
    }
    Serial.print("\nNum Bytes: ");
    Serial.println(bytes);
    Serial.print("Taken: ");
    Serial.println(taken);
  */
}

void setup() {
  Wire.begin(1);
  Wire.onReceive(receiveEvent);

  currentTime = millis();
  hex.begin();
  bootSequence();
}

void loop() { hex.show(); }
