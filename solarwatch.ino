#include <SiderealPlanets.h>

#define Serial SerialUSB

SiderealPlanets astro;
double latitude, longitude;
double rightAscension, declination;
double altitude, azimuth;

void setup() {
  Serial.begin(9600);
  //delay(2000);
  Serial.println("Starting...");
  astro.begin();
  astro.setTimeZone(8);
  astro.rejectDST();
  astro.setGMTdate(2021, 11, 11);
  astro.setLocalTime(23, 15, 0.0);

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
  while (1);
}
