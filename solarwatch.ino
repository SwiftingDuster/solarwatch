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

char* coordinates = reinterpret_cast<char*>(ble_rx_buffer);

int lat1 = 0;
int lat2 = 0;
int lat3 = 0;
int lon1 = 0;
int lon2 = 0;
int lon3 = 0;
/*------------------------------------------------------------------------------------------*/

typedef struct PlanetData {
  double altitude;
  double azimuth;
} PlanetData;

extern char* PLANET_NAMES[];

void setup() {
  // Init screen
  Wire.begin();
  screen.begin();
  screen.setBrightness(10);
  initScreen();

  // Init real time clock
  rtc.begin();
  rtc.setDate(13, 11, 2021 - 2000);  // dd/mm/yy
  rtc.setTime(18, 00, 00);

  // Init bluetooth
  BLEsetup();

  // Init serial for debug
  Serial.begin(9600);
  delay(3000);
  Serial.println("Starting...");

  astro.begin();
  astro.setTimeZone(8);
  astro.rejectDST();
  astro.setGMTdate(2021, 11, 13);
  astro.setLocalTime(17, 00, 0.0);  // hh,mm,ss

  // Set geographic location to Singapore
  double latitude = astro.decimalDegrees(1, 26, 33.f);
  double longitude = astro.decimalDegrees(103, 47, 54.f);
  astro.setLatLong(latitude, longitude);
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
    Serial.print("Phone : ");
    Serial.println((char*)ble_rx_buffer);

    int init_size = strlen(coordinates);
    int arr[init_size];
    int i = 0;

    char* p = strtok(coordinates, " ");
    while (p != NULL) {
      arr[i++] = atoi(p);
      p = strtok(NULL, " ");
    }

    lat1 = arr[0];
    lat2 = arr[1];
    lat3 = arr[2];
    lon1 = arr[3];
    lon2 = arr[4];
    lon3 = arr[5];

    double latitude = astro.decimalDegrees(lat1, lat2, lat3);
    double longitude = astro.decimalDegrees(lon1, lon2, lon3);
    astro.setLatLong(latitude, longitude);
    Serial.println("Set latitude to: " + (String)latitude);
    Serial.println("Set longitude to: " + (String)longitude);

    ble_rx_buffer_len = 0;  //clear afer reading
  }
}

PlanetData getPlanetData(int planetIndex) {
  char* name = PLANET_NAMES[planetIndex];
  if (strcmp(name, "Mercury") == 0) {
    astro.doMercury();
  } else if (strcmp(name, "Venus") == 0) {
    astro.doMercury();
  } else if (strcmp(name, "Mars") == 0) {
    astro.doMercury();
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
