#include <Arduino.h>
#include <microDS3231.h>
#include <Sgp4.h>
#include <Ticker.h>
#include <GyverOLED.h>
GyverOLED<SSD1306_128x32, OLED_BUFFER> oled;


MicroDS3231 rtc;
Sgp4 sat;

bool flag = 0;

void setup() {
  Serial.begin(9600);
  rtc.begin();

  sat.site(56, 38, 168.0);
  char satname[] = "ISS (ZARYA)";
  char tle_line1[] = "1 25544U 98067A   24042.54835156  .00018456  00000-0  33337-3 0  9995";
  char tle_line2[] = "2 25544  51.6397 222.9386 0002092 231.2912 231.6960 15.49761030438916";
  sat.init(satname,tle_line1,tle_line2);

  oled.init();
  oled.flipV(1);
  oled.flipH(1);
  oled.textMode(BUF_ADD);
  oled.clear(); 
  
}

void loop() {
  if(Serial.available() && !flag){
    if(Serial.read() == 't'){
      rtc.setTime(0, 47, 22, 11, 2, 2024);
    }
    while(Serial.available()){
      Serial.read();
    }
  }

  Serial.print(rtc.getYear());
  Serial.print(" ");
  Serial.print(rtc.getMonth());
  Serial.print(" ");
  Serial.print(rtc.getDate());
  Serial.print(" ");
  Serial.print(rtc.getHours());
  Serial.print(" ");
  Serial.print(rtc.getMinutes());
  Serial.print(" ");
  Serial.print(rtc.getSeconds());
  Serial.print(" ");
  Serial.print(rtc.getUnix(0));
  Serial.print(" ");
  sat.findsat((unsigned long)rtc.getUnix(3));
  Serial.print(sat.satDist);
  Serial.print(" ");
  Serial.print(sat.satLon);
  Serial.print(" ");
  Serial.print(sat.satLat);
  Serial.print(" ");
  Serial.println(sat.satAlt);

  oled.clear();
  oled.home();

  oled.setScale(2);
  oled.setCursorXY(30, 2);
  oled.print(sat.satDist);
  
  oled.setScale(1);
  oled.setCursorXY(4, 22);
  oled.print("Lat");
  oled.setCursorXY(26, 22);
  oled.print(sat.satLat);
  oled.setCursorXY(64, 22);
  oled.print("Lon");
  oled.setCursorXY(86, 22);
  oled.print(sat.satLon);

  oled.setScale(1);

  oled.setCursorXY(2, 12);
  oled.print("  km");

  oled.setCursorXY(4, 2);
  oled.print("Dist");

  oled.update();
}