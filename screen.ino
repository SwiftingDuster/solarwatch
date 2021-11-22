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
const int INFO_PLANET_DATA_Y = INFO_PLANET_Y + PLANET_SIZE_Y + 1; // Position to write azimuth and elevation

const int INFO_NAME_DIVIDER_Y = INFO_PLANET_NAME_Y + 10; // Position of divider line below planet name
const int INFO_PLANET_DIRECTION_Y = INFO_NAME_DIVIDER_Y + 5;

// Interface states
enum UIState { MainMenu, PlanetInfo };

UIState ui;
UIState prevUI;       // Track UI change between screen updates
int planetIndex = 0;  // The active planet displayed in main menu
int infoOffsetMins = 0; // Postivie offset in minutes used to calculate planet information

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

  switch (ui) {
    case MainMenu:
      screen.fontColor(TS_8b_White, TS_8b_Black);
      // Show planets on top
      drawMenuPlanet(planetIndex);
      break;
    case PlanetInfo:
      drawPlanetInfo(planetIndex);
      break;
  }

  prevUI = ui;
}

void drawMenuPlanet(int planetIndex) {
  // If planet same as before don't redraw
  if (planetIndex == prevPlanetIndex && ui == prevUI) return;

  int fontHeight = screen.getFontHeight();

  // Draw planet
  drawPlanet(planetIndex, MENU_PLANET_X, MENU_PLANET_Y, PLANET_SIZE_X, PLANET_SIZE_Y);

  // Write planet name
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
  DateTime dt = getRTCNow();
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
  setAstroTime(getRTCNow(), infoOffsetMins);
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
  DateTime dt = getRTCNow();
  if (infoOffsetMins > 0) {
    if (dt.minute + infoOffsetMins >= 60) {
      dt.hour += 1;
      dt.minute = (dt.minute + infoOffsetMins) % 60;
    }
    sprintf(datetime, "%02d:%02d (T+%02dm)" , dt.hour, dt.minute + infoOffsetMins, infoOffsetMins);
  }
  else {
    sprintf(datetime, "%02d:%02d" , dt.hour, dt.minute);
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

  //char elevation[8];
  //sprintf(elevation, "%.1f", data.altitude);
  //int elevPrintWidth = screen.getPrintWidth(elevation);
  //int elevX = PLANET_SIZE_X + (SCREEN_W - PLANET_SIZE_X - elevPrintWidth) / 2;
  //screen.fontColor(TS_8b_White, TS_8b_Black);
  //screen.setCursor(elevX, INFO_PLANET_DIRECTION_Y + fontHeight);
  //screen.print(elevation);

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

char* azimuthToNSEW(double azimuth) {
  if (azimuth < 0) {
    do {
      azimuth += 360;
    } while (azimuth < 0);
  } else if (azimuth > 359) {
    do {
      azimuth -= 360;
    } while (azimuth > 359);
  }

  if (azimuth < 22.5) {  // 0 to 22.4 is North
    return "North";
  } else if (azimuth < 67.5) {  // 22.5 to 67.4 is North-East
    return "North-East";
  } else if (azimuth < 112.5) {  // 67.5 to 112.4 is East
    return "East";
  } else if (azimuth < 157.5) {  // 112.5 to 157.4 is South-East
    return "South-East";
  } else if (azimuth < 202.5) {  // 157.5 to 202.4 is South
    return "South";
  } else if (azimuth < 247.5) {  // 205.5 to 247.4 is South-West
    return "South-West";
  } else if (azimuth < 292.5) {  // 247.5 to 292.4 is West
    return "West";
  } else if (azimuth < 337.5) {  // 292.5 to 337.4 is North-West
    return "North-West";
  } else {  // 337.5-360 is North
    return "North";
  }
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
