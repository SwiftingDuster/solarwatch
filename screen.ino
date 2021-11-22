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

const int MENU_PLANET_NAME_Y = MENU_PLANET_Y + PLANET_SIZE_Y + 2;  // Position to write the planet name
const int MENU_DIVIDER_Y = 48;                                     // Position to draw divider line
const int MENU_DATETIME_Y = MENU_DIVIDER_Y + 2;                    // Position to write the date and time

// Position to draw the planet in information screen
const int INFO_PLANET_X = 0;
const int INFO_PLANET_Y = 0;

const int INFO_PLANET_NAME_Y = 3;  // Position to write the planet name
const int INFO_PLANET_DATA_Y = INFO_PLANET_Y + PLANET_SIZE_Y + 3; // Position to write azimuth and elevation

const int INFO_NAME_DIVIDER_Y = INFO_PLANET_NAME_Y + 10; // Position to draw divider line below planet name
const int INFO_PLANET_DIRECTION_Y = INFO_NAME_DIVIDER_Y + 5; // Position to draw direction (NSEW) of planet

// Interface states
enum UIState { MainMenu, PlanetInfo };

// Application state
UIState ui; // The active UI screen
DateTime dt; // The current datetime, updated every call to updateScreen()
int planetIndex = 0;  // The active planet displayed in main menu
int infoOffsetMins = 0; // Postivie offset in minutes used to calculate planet information

UIState prevUI;       // To check if UI screen changed between navigation
int lastDTMinute = -1; // To check if RTC clock minute changed and we should redraw clock time
int prevPlanetIndex = -1;      // To check if we should redraw the main menu screen (MainMenu)
int prevPlanetNameWidth = -1;  // To check if we should clear the planet name window area (Main Menu,  PlanetInfo)
int prevInfoOffsetMins = 0; // To check if offset changed and a recalculation is needed (PlanetInfo)
int prevDirectionWidth = -1;  // To check if we should clear the planet name window area (PlanetInfo)
int prevDatetimeWidth = -1;  // To check if we should clear the date/time window area (PlanetInfo)

void initScreen() {
  screen.clearScreen();  // Same as clearWindow(0,0,96,64)
  screen.setFlip(true);
  screen.setFont(FONT10_Pixel7);
  ui = MainMenu;
}

void updateScreen() {
  // Display is 96x64

  if (ui != prevUI) {
    screen.clearScreen();
  }

  dt = getRTCNow();

  switch (ui) {
    case MainMenu:
      drawMenuPlanet(planetIndex);
      break;
    case PlanetInfo:
      drawPlanetInfo(planetIndex);
      break;
  }

  prevUI = ui;
  lastDTMinute = dt.minute;
}

void drawMenuPlanet(int planetIndex) {
  // If planet same as before don't redraw
  if (dt.minute == lastDTMinute && planetIndex == prevPlanetIndex && ui == prevUI) return;

  int fontHeight = screen.getFontHeight();

  // Draw planet
  drawPlanet(planetIndex, MENU_PLANET_X, MENU_PLANET_Y, PLANET_SIZE_X, PLANET_SIZE_Y);

  // Write planet name
  screen.fontColor(TS_8b_White, TS_8b_Black);
  char planetNameDisplay[15];
  ("< " + String(PLANET_NAMES[planetIndex]) + " >").toCharArray(planetNameDisplay, 15);
  int planetNameWidth = screen.getPrintWidth(planetNameDisplay);
  if (planetNameWidth < prevPlanetNameWidth) {  // If taking less space than before, clear pixels in the area first
    int prevX = (SCREEN_W - prevPlanetNameWidth) / 2;
    screen.clearWindow(prevX, MENU_PLANET_NAME_Y, prevPlanetNameWidth, fontHeight);
  }
  int planetNameX = (SCREEN_W - planetNameWidth) / 2;  // Center planet name
  screen.setCursor(planetNameX, MENU_PLANET_NAME_Y);
  screen.print(planetNameDisplay);

  // Draw divider
  screen.drawLine(0, MENU_DIVIDER_Y, 95, MENU_DIVIDER_Y, TS_8b_Yellow);

  // Show date and time below
  screen.setCursor(1, MENU_DATETIME_Y);
  char dtDisplay[17]; // hh:mm dd/MM/yyyy
  sprintf(dtDisplay, "%02d:%02d %02d/%02d/%4d", dt.hour, dt.minute, dt.day, dt.month, dt.year);
  screen.print(dtDisplay);

  prevPlanetIndex = planetIndex;
  prevPlanetNameWidth = planetNameWidth;
}

void drawPlanetInfo(int planetIndex) {
  // If planet same as before don't redraw
  if (planetIndex == prevPlanetIndex && infoOffsetMins == prevInfoOffsetMins && ui == prevUI) return;

  int fontHeight = screen.getFontHeight();

  Serial.println("Draw Planet Info");
  setAstroTime(dt, infoOffsetMins);
  PlanetData data = getPlanetData(planetIndex);

  // Draw picture of selected planet
  drawPlanet(planetIndex, 0, 0, PLANET_SIZE_X, PLANET_SIZE_Y);
  screen.drawLine(INFO_PLANET_X, INFO_PLANET_Y + PLANET_SIZE_Y, PLANET_SIZE_X, PLANET_SIZE_Y, TS_8b_Yellow);
  screen.drawLine(INFO_PLANET_X + PLANET_SIZE_X, INFO_PLANET_Y, PLANET_SIZE_X, PLANET_SIZE_Y, TS_8b_Yellow);

  // Write azimuth and altitude below picture
  screen.fontColor(TS_8b_White, TS_8b_Black);

  char azimuth[6];
  sprintf(azimuth, "A:%.0f", data.azimuth);
  screen.setCursor(INFO_PLANET_X, INFO_PLANET_DATA_Y);
  screen.print(azimuth);

  char altitude[8];
  sprintf(altitude, "E:%3.0f", data.altitude);
  screen.setCursor(INFO_PLANET_X, INFO_PLANET_DATA_Y + fontHeight);
  screen.print(altitude);

  char datetime[4];
  int hour = dt.hour;
  int minute = dt.minute;
  if (infoOffsetMins > 0) {
    if (dt.minute + infoOffsetMins > 59) {
      hour += 1;
      minute = (dt.minute + infoOffsetMins) % 60;
    } else {
      minute = dt.minute + infoOffsetMins;
    }
    sprintf(datetime, "%02d:%02d (T+%02dm)" , hour, minute, infoOffsetMins);
  }
  else {
    sprintf(datetime, "%02d:%02d" , hour, minute);
  }
  int datetimeWidth = screen.getPrintWidth(datetime);
  if (datetimeWidth < prevDatetimeWidth) { // If taking less space than before, clear pixels in the area first
    screen.clearWindow(INFO_PLANET_X, INFO_PLANET_DATA_Y + fontHeight * 2, prevDatetimeWidth, fontHeight);
  }
  screen.setCursor(INFO_PLANET_X, INFO_PLANET_DATA_Y + fontHeight * 2);
  screen.print(datetime);

  // Write planet name
  screen.fontColor(TS_8b_White, TS_8b_Black);
  int planetNameWidth = screen.getPrintWidth(PLANET_NAMES[planetIndex]);
  int planetNameX = PLANET_SIZE_X + (SCREEN_W - PLANET_SIZE_X - planetNameWidth) / 2;
  screen.setCursor(planetNameX, INFO_PLANET_NAME_Y);
  screen.print(PLANET_NAMES[planetIndex]);

  // Draw filled box for planet visibility (above horizon). Green = Visible, Red = Not visible
  if (data.altitude > 0) {
    screen.drawRect(planetNameX + planetNameWidth + 2, INFO_PLANET_NAME_Y + 2, 6, 6, TSRectangleFilled, TS_8b_Green);
  } else {
    screen.drawRect(planetNameX + planetNameWidth + 2, INFO_PLANET_NAME_Y + 2, 6, 6, TSRectangleFilled, TS_8b_Red);
  }

  // Draw seperator under planet name
  screen.drawLine(INFO_PLANET_X + PLANET_SIZE_X, INFO_NAME_DIVIDER_Y, SCREEN_W, INFO_NAME_DIVIDER_Y, TS_8b_Yellow);

  // Write NSEW direction
  char* direction = azimuthToNSEW(data.azimuth);
  int directionWidth = screen.getPrintWidth(direction);
  if (directionWidth < prevDirectionWidth) {
    int prevX = PLANET_SIZE_X + (SCREEN_W - PLANET_SIZE_X - prevDirectionWidth) / 2;
    screen.clearWindow(prevX, INFO_PLANET_DIRECTION_Y, prevDirectionWidth, fontHeight);
  }
  int dirX = PLANET_SIZE_X + (SCREEN_W - PLANET_SIZE_X - directionWidth) / 2;
  screen.fontColor(TS_8b_Green, TS_8b_Black);
  screen.setCursor(dirX, INFO_PLANET_DIRECTION_Y);
  screen.print(direction);

  // Write planet rise/set time
  screen.fontColor(TS_8b_White, TS_8b_Black);
  int riseMins = (data.rise - floor(data.rise)) * 60;
  char planetRise[12]; // Rise: hh:mm
  sprintf(planetRise, "Rise: %02.0f:%02d", data.rise, riseMins);
  int planetRiseWidth = screen.getPrintWidth(planetRise);
  int planetRiseX = PLANET_SIZE_X + (SCREEN_W - PLANET_SIZE_X - planetRiseWidth) / 2;
  screen.setCursor(planetRiseX, INFO_PLANET_DIRECTION_Y + fontHeight);
  screen.print(planetRise);

  int setMins = (data.set - floor(data.set)) * 60;
  char planetSet[11]; // Set: hh:mm
  sprintf(planetSet, "Set: %02.0f:%02d", data.set, setMins);
  int planetSetWidth = screen.getPrintWidth(planetSet);
  int planetSetX = PLANET_SIZE_X + (SCREEN_W - PLANET_SIZE_X - planetSetWidth) / 2;
  screen.setCursor(planetSetX, INFO_PLANET_DIRECTION_Y + fontHeight * 2);
  screen.print(planetSet);

  prevPlanetIndex = planetIndex;
  prevPlanetNameWidth = planetNameWidth;
  prevDirectionWidth = directionWidth;
  prevDatetimeWidth = datetimeWidth;
  prevInfoOffsetMins = infoOffsetMins;
}

void drawPlanet(int planetIndex, int posX, int posY, int sizeX, int sizeY) {
  screen.setX(posX, posX + sizeX - 1);
  screen.setY(posY, posY + sizeY - 1);
  screen.startData();
  screen.writeBuffer(PLANET_PIXELS[planetIndex], PLANET_SIZE_X * PLANET_SIZE_Y);
  screen.endTransfer();
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
  if (dt.hour == 23) {
    int maxOffset = (60 - dt.minute) / 10;
    if (infoOffsetMins > maxOffset) infoOffsetMins = maxOffset;
  } else if (infoOffsetMins > 60)
    infoOffsetMins = 60;
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
      } else if (screen.getButtons(TSButtonLowerRight)) {
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
      } else if (screen.getButtons(TSButtonLowerRight)) {
        incInfoOffset();
        Serial.println("Planet Info Offset: " + (String)infoOffsetMins);
      }
      break;
  }
}
