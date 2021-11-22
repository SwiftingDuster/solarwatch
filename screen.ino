const FONT_INFO& FONT10_Pixel7 = thinPixel7_10ptFontInfo;
const FONT_INFO& FONT8_LibSans = liberationSansNarrow_8ptFontInfo;

const int SCREEN_W = 96;
const int SCREEN_H = 64;

// Resolution of planet images in pixels
const int PLANET_SIZE_X = 32;
const int PLANET_SIZE_Y = 32;

// Position to draw the planet in main menu
const int MENU_PLANET_X = (SCREEN_W - PLANET_SIZE_X) / 2;
const int MENU_PLANET_Y = 0;

const int MENU_PLANET_NAME_Y = MENU_PLANET_Y + PLANET_SIZE_Y + 2; // Position to write the planet name
const int MENU_DIVIDER_Y = 48;                                    // Position to draw divider line
const int MENU_DATETIME_Y = MENU_DIVIDER_Y + 2;                   // Position to write the date and time

// Position to draw the planet in information screen
const int INFO_PLANET_X = 0;
const int INFO_PLANET_Y = 0;

const int INFO_PLANET_NAME_Y = 5; // Position to write the planet name
const int INFO_PLANET_DATA_Y = INFO_PLANET_Y + PLANET_SIZE_Y + 1;

// Interface states
enum UIState { MainMenu, PlanetInfo };

UIState ui;
UIState prevUI; // Track UI change between screen updates
int planetIndex = 0; // The active planet displayed in main menu

int lastPlanetIndex = -1; // To check if we should redraw the main menu screen
int lastPlanetNameWidth = -1; // To check if we should clear the planet name window area

int infoOffsetMins = 0;
int lastInfoOffsetMins = 0;

void initScreen() {
  screen.clearScreen(); // Same as clearWindow(0,0,96,64)
  screen.setFlip(true);
  screen.setFont(FONT10_Pixel7);
  ui = MainMenu;
}

void updateScreen() {
  // Display is 96x64

  if (ui != prevUI) {
    screen.clearScreen();
  }

  switch (ui) {
    case MainMenu:
      screen.fontColor(TS_8b_White, TS_8b_Black);
      // Show planets on top
      drawMenuPlanet(planetIndex);
      // Draw divider
      screen.drawLine(0, MENU_DIVIDER_Y, 95, MENU_DIVIDER_Y, TS_8b_Yellow);
      // Show date and time below
      drawMenuDateTime();
      break;
    case PlanetInfo:
      drawPlanetInfo(planetIndex);
      break;
  }

  prevUI = ui;
}

void drawMenuPlanet(int planetIndex) {
  // If planet same as before don't redraw
  if (planetIndex == lastPlanetIndex && ui == prevUI) return;

  // Draw planet
  drawPlanet(planetIndex, MENU_PLANET_X, MENU_PLANET_Y, PLANET_SIZE_X, PLANET_SIZE_Y);

  // Write planet name
  char planetNameDisplay[15];
  ("< " + String(PLANET_NAMES[planetIndex]) + " >").toCharArray(planetNameDisplay, 15);
  int printWidth = screen.getPrintWidth(planetNameDisplay);
  if (printWidth < lastPlanetNameWidth) { // If taking less space than before, clear pixels in the area first
    int prevX = (SCREEN_W - lastPlanetNameWidth) / 2;
    screen.clearWindow(prevX, MENU_PLANET_NAME_Y, lastPlanetNameWidth, screen.getFontHeight());
  }
  int planetNameX = (SCREEN_W - printWidth) / 2; // Center planet name
  screen.setCursor(planetNameX, MENU_PLANET_NAME_Y);
  screen.println(planetNameDisplay);

  lastPlanetIndex = planetIndex;
  lastPlanetNameWidth = printWidth;
}

void drawMenuDateTime() {
  screen.setCursor(SCREEN_W * 0.02, MENU_DATETIME_Y);
  DateTime dt = getRTCNow();

  printZeroPadded(dt.hour);
  screen.print(":");
  printZeroPadded(dt.minute);
  screen.print(" ");
  printZeroPadded(dt.day);
  screen.print('/');
  printZeroPadded(dt.month);
  screen.print('/');
  printZeroPadded(dt.year);
}

void drawPlanetInfo(int planetIndex) {
  // If planet same as before don't redraw
  if (planetIndex == lastPlanetIndex && infoOffsetMins == lastInfoOffsetMins && ui == prevUI) return;

  Serial.println("Draw Planet Info");
  setAstroTime(getRTCNow(), infoOffsetMins);
  PlanetData data = getPlanetData(planetIndex);

  // Draw picture of selected planet
  drawPlanet(planetIndex, 0, 0, PLANET_SIZE_X, PLANET_SIZE_Y);
  screen.drawLine(INFO_PLANET_X, INFO_PLANET_Y + PLANET_SIZE_Y, PLANET_SIZE_X, PLANET_SIZE_Y, TS_8b_Yellow);
  screen.drawLine(INFO_PLANET_X + PLANET_SIZE_X, INFO_PLANET_Y, PLANET_SIZE_X, PLANET_SIZE_Y, TS_8b_Yellow);

  // Write azimuth and altitude below picture
  screen.fontColor(TS_8b_White, TS_8b_Black);
  screen.setCursor(INFO_PLANET_X, INFO_PLANET_DATA_Y);
  screen.println("A:" + (String)round(data.azimuth));
  screen.setCursor(INFO_PLANET_X, INFO_PLANET_DATA_Y + screen.getFontHeight());
  screen.println("E:" + (String)round(data.altitude));
  screen.setCursor(INFO_PLANET_X, INFO_PLANET_DATA_Y + screen.getFontHeight() * 2);
  screen.println(infoOffsetMins);

  // Write planet name
  screen.fontColor(TS_8b_White, TS_8b_Black);
  int printWidth = screen.getPrintWidth(PLANET_NAMES[planetIndex]);
  int planetNameX = PLANET_SIZE_X + (SCREEN_W - PLANET_SIZE_X - printWidth) / 2;
  screen.setCursor(planetNameX, INFO_PLANET_NAME_Y);
  screen.println(PLANET_NAMES[planetIndex]);

  // Draw filled box for planet visibility (above horizon). Green = Visible, Red = Not visible
  if (data.altitude > 0) {
    screen.drawRect(planetNameX + printWidth + 2, INFO_PLANET_NAME_Y + 2, 6, 6, TSRectangleFilled, TS_8b_Green);
  } else {
    screen.drawRect(planetNameX + printWidth + 2, INFO_PLANET_NAME_Y + 2, 6, 6, TSRectangleFilled, TS_8b_Red);
  }

  // Draw seperator under planet name
  int lineY = INFO_PLANET_NAME_Y + screen.getFontHeight();
  screen.drawLine(INFO_PLANET_X + PLANET_SIZE_X, lineY , SCREEN_W, lineY, TS_8b_Yellow);

  // Write NSEW direction
  //screen.fontColor(TS_8b_White, TS_8b_Black);
  //screen.setCursor(planetNameX, lineY + 1);
  //screen.setCursor(planetNameX, lineY + screen.getFontHeight());

  lastPlanetIndex = planetIndex;
  lastInfoOffsetMins = infoOffsetMins;
}

void drawPlanet(int planetIndex, int posX, int posY, int sizeX, int sizeY) {
  screen.setX(posX, posX + sizeX - 1);
  screen.setY(posY, posY + sizeY - 1);
  screen.startData();
  screen.writeBuffer(PLANET_PIXELS[planetIndex], PLANET_SIZE_X * PLANET_SIZE_Y);
  screen.endTransfer();
}

void printZeroPadded(int val) {
  if (val < 10) {
    screen.print('0');
  }
  screen.print(val);
}

void nextPlanet() {
  planetIndex++;
  if (planetIndex >= PLANET_COUNT) {
    planetIndex = 0;
  }
}

void prevPlanet() {
  planetIndex--;
  if (planetIndex < 0) {
    planetIndex = PLANET_COUNT - 1;
  }
}

void incInfoOffset() {
  infoOffsetMins += 10;
  // Offset normally caps at 60
  // When its 11pm, max offset is the number of 10min intervals without reaching midnight.
  // E.g. At 11.35pm, max offset is 20.
  DateTime dt = getRTCNow();
  if (dt.hour == 23) {
    int maxOffset = (60 - dt.minute) / 10;
    if (infoOffsetMins > maxOffset) infoOffsetMins = maxOffset;
  }
  else if (infoOffsetMins > 60) infoOffsetMins = 60;
}

void decInfoOffset() {
  infoOffsetMins -= 10;
  if (infoOffsetMins < 0) infoOffsetMins = 0;
}

void checkButtons() {
  switch (ui) {
    case MainMenu:
      // Select planet
      if (screen.getButtons(TSButtonUpperRight)) {
        ui = PlanetInfo;
        infoOffsetMins = 0;
        Serial.println("UIState: MainMenu -> PlanetInfo");
      }
      // Left/right planet selection
      if (screen.getButtons(TSButtonLowerLeft)) {
        prevPlanet();
        Serial.println("Prev Planet");
      }
      else if (screen.getButtons(TSButtonLowerRight)) {
        nextPlanet();
        Serial.println("Next Planet");
      }
      break;
    case PlanetInfo:
      if (screen.getButtons(TSButtonUpperLeft)) {
        ui = MainMenu;
        Serial.println("UIState: PlanetInfo -> MainMenu");
      }
      // Time offset
      if (screen.getButtons(TSButtonLowerLeft)) {
        decInfoOffset();
        Serial.println("Planet Info Offset: " + (String)infoOffsetMins);
      }
      else if (screen.getButtons(TSButtonLowerRight)) {
        incInfoOffset();
        Serial.println("Planet Info Offset: " + (String)infoOffsetMins);
      }
      break;
  }
}
