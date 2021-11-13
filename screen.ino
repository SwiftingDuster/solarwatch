const FONT_INFO& font10pt = thinPixel7_10ptFontInfo;
const FONT_INFO& font22pt = liberationSansNarrow_22ptFontInfo;

const int SCREEN_W = 96;
const int SCREEN_H = 64;

const int PLANET_SIZE_X = 32;
const int PLANET_SIZE_Y = 32;
const int PLANET_X = (SCREEN_W - PLANET_SIZE_X) / 2;
const int PLANET_Y = 0;
const int PLANET_NAME_Y = PLANET_Y + PLANET_SIZE_Y + 5;

const int DIVIDER_Y = 48;

const int DATETIME_X = 7;
const int DATETIME_Y = DIVIDER_Y + 2; // Date/time is two pixels below divider.

void initScreen() {
  screen.clearScreen(); // Same as clearWindow(0,0,96,64)
  screen.setFlip(true);
}

void updateDisplay(int planetIndex) {
  // Display is 96x64
  //screen.clearScreen();

  // Show planets on top
  drawPlanetMenu(planetIndex);

  // Draw divider
  screen.drawLine(0, DIVIDER_Y, 95, DIVIDER_Y, TS_8b_Yellow);

  // Show date and time below
  drawDateTime();
}

void drawPlanetMenu(int planetIndex) {
  // Draw planet
  screen.setX(PLANET_X, PLANET_X + PLANET_SIZE_X - 1);
  screen.setY(PLANET_Y, PLANET_Y + PLANET_SIZE_Y - 1);
  screen.startData();
  screen.writeBuffer(PLANET_DATA[planetIndex], PLANET_SIZE_X * PLANET_SIZE_Y);
  screen.endTransfer();

  // Write planet name
  char displayBuffer[14];
  strcpy(displayBuffer, "< ");
  strcat(displayBuffer, PLANET_NAMES[planetIndex]);
  strcat(displayBuffer, " >");
  strcat(displayBuffer, "\0");
  screen.setFont(font10pt);
  const int PLANET_NAME_X = (SCREEN_W - screen.getPrintWidth(displayBuffer)) / 2;
  screen.setCursor(PLANET_NAME_X, PLANET_NAME_Y);
  screen.println(displayBuffer);
}

void drawDateTime() {
  //screen.clearWindow(DATETIME_X, DATETIME_Y, SCREEN_W - DATETIME_X, SCREEN_H - DATETIME_Y);
  screen.setFont(font10pt);
  screen.setCursor(DATETIME_X, DATETIME_Y);

  int hours = rtc.getHours();
  int minutes = rtc.getMinutes();
  //int seconds = rtc.getSeconds();
  int day = rtc.getDay();
  int month = rtc.getMonth();
  int year = rtc.getYear();

  printZeroPadded(hours);
  screen.print(":");
  printZeroPadded(minutes);
  //screen.print(":");
  //printZeroPadded(seconds);
  screen.print(" ");
  printZeroPadded(day);
  screen.print('/');
  printZeroPadded(month);
  screen.print('/');
  printZeroPadded(year);
}

char* printZeroPadded(int val) {
  if (val < 10) {
    screen.print('0');
  }
  screen.print(val);
}

void checkButtons(int* planetIndex) {
  if (screen.getButtons(TSButtonLowerLeft)) {
    (*planetIndex)--;
    if (*planetIndex < 0) {
      *planetIndex = PLANET_COUNT - 1;
    }
    Serial.println("Prev Click");
  }
  if (screen.getButtons(TSButtonLowerRight)) {
    (*planetIndex)++;
    if (*planetIndex >= PLANET_COUNT) {
      *planetIndex = 0;
    }
    Serial.println("Next Click");
  }
}
