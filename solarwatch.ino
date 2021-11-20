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

typedef struct PlanetData {
  double altitude;
  double azimuth;
} PlanetData;

extern char* PLANET_NAMES[];

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

  // Init real time clock
  rtc.begin();
  rtc.setDate(20, 11, 2021 - 2000);  // dd/mm/yy
  rtc.setTime(16, 30, 00);

  astro.begin();
  astro.setTimeZone(8);
  astro.rejectDST();
  astro.setGMTdate(2021, 11, 20); // yyyy,MM,dd
  astro.setLocalTime(16, 30, 0);  // hh,mm,ss

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
  setAstroTime(y, M, d, h, m, s);
  Serial.println("Set date to: " + (String)y + " " + (String)M + " " + (String)d);
  Serial.println("Set time to: " + (String)h + " " + (String)m + " " + (String)s);
}

// Set the date and time
void setAstroTime(int y, int M, int d, int h, int m, int s) {
  astro.setGMTdate(y, M, d); // yyyy,MM,dd
  astro.setLocalTime(h, m, s);  // hh,mm,ss
}

// Set latitude and longtitude
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

PlanetData getPlanetData(int planetIndex) {
  char* name = PLANET_NAMES[planetIndex];
  if (strcmp(name, "Mercury") == 0) {
    astro.doMercury();
  } else if (strcmp(name, "Venus") == 0) {
    astro.doVenus();
  } else if (strcmp(name, "Mars") == 0) {
    astro.doMars();
  } else if (strcmp(name, "Jupiter") == 0) {
    astro.doJupiter();
  } else if (strcmp(name, "Saturn") == 0) {
    astro.doSaturn();
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
