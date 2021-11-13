#include <SiderealPlanets.h>
#include <Wire.h>
#include <TinyScreen.h>
#include <RTCZero.h>

#define Serial SerialUSB

SiderealPlanets astro;
double latitude, longitude;
double rightAscension, declination;
double altitude, azimuth;

TinyScreen screen = TinyScreen(TinyScreenDefault);
RTCZero rtc;

void setup() {
  // Init screen
  Wire.begin();
  screen.begin();
  screen.setBrightness(10);
  initScreen();

  // Init real time clock
  rtc.begin();
  rtc.setDate(13, 11, 2021 - 2000); // dd/mm/yy
  rtc.setTime(18, 00, 00);

  // Init serial for debug
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting...");
  
  astro.begin();
  astro.setTimeZone(8);
  astro.rejectDST();
  astro.setGMTdate(2021, 11, 13);
  astro.setLocalTime(17, 00, 0.0); // hh,mm,ss

  // Set geographic location to Singapore
  latitude = astro.decimalDegrees(1, 26, 33.f);
  longitude = astro.decimalDegrees(103, 47, 54.f);
  astro.setLatLong(latitude, longitude);

  astro.doJupiter();
  rightAscension = astro.getRAdec();
  declination = astro.getDeclinationDec();
  Serial.println(rightAscension);
  Serial.println(declination);

  astro.setRAdec(rightAscension, declination);
  astro.doRAdec2AltAz();
  altitude = astro.getAltitude();
  azimuth = astro.getAzimuth();
  Serial.println(altitude);
  Serial.println(azimuth);
}

void loop() {
  updateDisplay();
  delay(300);
}
