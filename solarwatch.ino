#include <SiderealPlanets.h>
#include <Wire.h>
#include <TinyScreen.h>
#include <RTCZero.h>
#include <STBLE.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Serial SerialUSB

SiderealPlanets astro;

TinyScreen screen = TinyScreen(TinyScreenDefault);
RTCZero rtc;

/*Bluetooth------------------------------------------------------------------------------------------*/
uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0
/*------------------------------------------------------------------------------------------*/

typedef struct DateTime {
  int year, month, day;
  int hour, minute, second;
} DateTime;

typedef struct PlanetData {
  double altitude;
  double azimuth;
} PlanetData;

extern char* PLANET_NAMES[];

void btConnection() {
  aci_loop();  //Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.

  // Check if data is available
  if (ble_rx_buffer_len) {
    Serial.print("Bluetooth RX: ");
    Serial.println((char*)ble_rx_buffer);

    // Command to set datetime (prefix D)
    if (ble_rx_buffer[0] == 'D') {
      setRTCTime(ble_rx_buffer + 1);
    }
    // Command to set geo coordinates (prefix C)
    else if (ble_rx_buffer[0] == 'C') {
      setGeoCoordinate(ble_rx_buffer + 1);
    }

    ble_rx_buffer_len = 0;  //clear afer reading
  }
}

// Set latitude and longtitude used for SiderealPlanets' calculation.
void setGeoCoordinate(uint8_t *b) {
  // Expect coordinates string in the form "deg min sec deg min sec" (LAT LONG). E.g. 1 26 33 103 47 54
  char* coordinates = (char*)b;

  int init_size = strlen(coordinates);
  int arr[init_size];
  int i = 0;

  char* p = strtok(coordinates, " ");
  while (p != NULL) {
    arr[i++] = atoi(p);
    p = strtok(NULL, " ");
  }

  int lat1 = arr[0];
  int lat2 = arr[1];
  int lat3 = arr[2];
  int lon1 = arr[3];
  int lon2 = arr[4];
  int lon3 = arr[5];

  double latitude = astro.decimalDegrees(lat1, lat2, lat3);
  double longitude = astro.decimalDegrees(lon1, lon2, lon3);
  astro.setLatLong(latitude, longitude);
  Serial.println("Set latitude to: " + (String)latitude);
  Serial.println("Set longitude to: " + (String)longitude);
}

// Set the date and time used for SiderealPlanets' calculation.
void setAstroTime(DateTime dt, int minuteOffset = 0) {
  int mins = dt.minute + minuteOffset;
  if (mins > 59) {
    dt.hour++;
    mins %= 60;
  }
  bool setD = astro.setGMTdate(dt.year, dt.month, dt.day); // yyyy,MM,dd
  bool setT = astro.setLocalTime(dt.hour, mins, dt.second);  // hh,mm,ss
  if (!setD || !setT ) {
    Serial.println("setAstroTime(): Couldn't set datetime.");
  }
  Serial.println((String)dt.year + "/" + (String)dt.month + "/" + (String)dt.day);
  Serial.println((String)dt.hour + "h:" + (String)mins + "m:" + (String)dt.second);
}

// Set the date and time on the hardware Real Time Clock.
void setRTCTime(uint8_t *buffer) {
  // Expect datetime string in the form "yyyy mm dd hh mm ss". E.g. 2015 03 05 11 48 42
  int y, M, d, h, m, s;
  char * next;
  y = strtol((char *)buffer, &next, 10);
  M = strtol(next, &next, 10);
  d = strtol(next, &next, 10);
  h = strtol(next, &next, 10);
  m = strtol(next, &next, 10);
  s = strtol(next, &next, 10);
  rtc.setTime(h, m, s);
  rtc.setDate(d, M, y - 2000);
  DateTime dt = { y, M, d, h, m, s };
  setAstroTime(dt);
  Serial.println("Set date to: " + (String)y + " " + (String)M + " " + (String)d);
  Serial.println("Set time to: " + (String)h + " " + (String)m + " " + (String)s);
}

// Get datetime of hardware clock.
DateTime getRTCNow() {
  int year = rtc.getYear() + 2000;
  int month = rtc.getMonth();
  int day = rtc.getDay();
  int hour = rtc.getHours();
  int minute = rtc.getMinutes();
  int second = rtc.getSeconds();
  DateTime dt = {year, month, day, hour, minute, second};
  return dt;
}

PlanetData getPlanetData(int planetIndex) {
  char* name = PLANET_NAMES[planetIndex];
  if (strcmp(name, "Sun") == 0) {
    astro.doSun();
  } else if (strcmp(name, "Mercury") == 0) {
    astro.doMercury();
  } else if (strcmp(name, "Venus") == 0) {
    astro.doVenus();
  } else if (strcmp(name, "Moon") == 0) {
    astro.doMoon();
  } else if (strcmp(name, "Mars") == 0) {
    astro.doMars();
  } else if (strcmp(name, "Jupiter") == 0) {
    astro.doJupiter();
  } else if (strcmp(name, "Saturn") == 0) {
    astro.doSaturn();
  } else if (strcmp(name, "Uranus") == 0) {
    astro.doUranus();
  } else if (strcmp(name, "Neptune") == 0) {
    astro.doNeptune();
  }
  double rightAscension = astro.getRAdec();
  double declination = astro.getDeclinationDec();
  astro.setRAdec(rightAscension, declination);

  astro.doRAdec2AltAz();
  double altitude = astro.getAltitude();
  double azimuth = astro.getAzimuth();

  PlanetData data = { altitude, azimuth };
  return data;
}

void setup() {
  // Init serial for debug
  Serial.begin(9600);
  delay(3000);
  Serial.println("Starting...");

  // Init screen
  Wire.begin();
  screen.begin();
  screen.setBrightness(10);
  initScreen();

  const int y = 2021;
  const int M = 11;
  const int d = 22;
  const int h = 19;
  const int m = 0;
  const int s = 0;

  // Init real time clock
  rtc.begin();
  rtc.setDate(d, M, y - 2000);  // dd/mm/yy
  rtc.setTime(h, m, s);

  // Init SiderealPlanets
  astro.begin();
  astro.setTimeZone(8);
  astro.rejectDST();
  setAstroTime(getRTCNow());

  // Set geographic location to Singapore
  double latitude = astro.decimalDegrees(1, 26, 33.f);
  double longitude = astro.decimalDegrees(103, 47, 54.f);
  astro.setLatLong(latitude, longitude);

  // Init bluetooth
  BLEsetup();
}

void loop() {
  btConnection(); // Poll bluetooth data
  checkButtons();
  updateScreen();
  delay(300);
}
