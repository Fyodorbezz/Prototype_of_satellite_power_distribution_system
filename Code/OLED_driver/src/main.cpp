#include <Arduino.h>
#include <SPI.h>
#include <Arduino_ST7735_Fast.h>

#define	BLACK   0x0000
//#define	BLUE    0x001F
#define	BLUE    	0xF800
#define	RED     	0x001F
#define DARK_RED 	0x0013	
#define	GREEN   0x07E0
#define DARK_GREEN 	0x04C0
#define CYAN    0xFFE0
#define MAGENTA 0xF81F
#define DARK_MAGENTA 	0x9813
#define LIGHT_YELLOW 	0x9FFF
#define YELLOW  0x07FF 
#define DARK_YELLOW 0x04D3
#define WHITE   0xFFFF
#define ORANGE 0x041F

struct Normalised_values{
  short current[9];
  short voltage[9];
  bool warnings[6];
};

Normalised_values normalised_values;

String Warnings[6]= {"Input 1 no voltage  ",
                     "Input 2 no voltage  ",
                     "Load 1 no voltage  ", 
                     "Load 2 no voltage  ",
                     "Load 3 no voltage  ",
                     "Load 2 overcurrent  "};

Arduino_ST7735 tft1 = Arduino_ST7735(10, 14, 9);
Arduino_ST7735 tft2 = Arduino_ST7735(8, 14, 7);

float in1_cur = 0;
float in1_vol = 0;

float in2_cur = 0;
float in2_vol = 0;

void draw_grid();
void draw_parameters();
void update_parameters();

void setup() {
  Serial.begin(115200);

  tft1.init();
  tft1.setRotation(3);
  tft1.setCursor(0, 0);
  tft1.fillRect(0,0,128,128,BLACK);

  tft2.init();
  tft2.setRotation(3);
  tft2.setCursor(0, 0);
  tft2.fillRect(0,0,128,128,WHITE);

  draw_grid();
  draw_parameters();
}

unsigned long long update_timer = 0;

void loop() {
  Serial.println(Serial.available());
  if(Serial.available() >= sizeof(Normalised_values)){
    Serial.readBytes((byte*)&normalised_values, sizeof(Normalised_values));
    
    for(int i =0; i<9; i++){
      Serial.print(normalised_values.current[i]);
      Serial.print(" ");
    }
    Serial.print("  ");
    for(int i =0; i<9; i++){
      Serial.print(normalised_values.voltage[i]);
      Serial.print(" ");
    }
    Serial.println(millis());
    while(Serial.available() > 0) {
      char t = Serial.read();
    }
    update_timer = millis();
    //draw_grid();
    update_parameters();

    if(normalised_values.voltage[0] == -10){
      tft2.setCursor(0, 0);
      tft2.fillRect(0,0,128,128,WHITE);

      tft1.fillRect(0,0,128,128,BLACK);

      draw_grid();
      draw_parameters();
      update_timer = millis();
    }
  }
  
}

void draw_grid(){
  tft1.drawLine(1,3,127,3,YELLOW);
  tft1.drawLine(1,3,1,127,YELLOW);
  tft1.drawLine(127,3,127,127,YELLOW);
  tft1.drawLine(64,3,64,72,YELLOW);
  tft1.drawLine(1,26,127,26,YELLOW);
  tft1.drawLine(1,49,127,49,YELLOW);
  tft1.drawLine(0,72,127,72,YELLOW);
  tft1.drawLine(102,72,102,103,YELLOW);
  tft1.drawLine(0,105,127,105,YELLOW);
  tft1.drawLine(0,127,127,127,YELLOW);
  tft1.drawLine(64,105,64,127,YELLOW);

  tft2.drawLine(1,3,127,3,YELLOW);
  tft2.drawLine(1,3,1,127,YELLOW);
  tft2.drawLine(127,3,127,127,YELLOW);
  tft2.drawLine(0,127,127,127,YELLOW);
}

void draw_parameters(){
  //tft1.clearScreen();

  tft1.setTextSize(1);

  tft1.setTextColor(BLUE, BLACK);

  tft1.setCursor(4, 6+4);
  tft1.print("IN1");
  tft1.setCursor(24, 2+4);
  tft1.print(in1_cur, 3);
  tft1.setCursor(56, 2+4);
  tft1.print("A");
  tft1.setCursor(24, 12+4);
  tft1.print(in1_vol, 3);
  tft1.setCursor(56, 12+4);
  tft1.print("V");

  tft1.setTextColor(GREEN, BLACK);

  tft1.setCursor(4, 6+27);
  tft1.print("IN2");
  tft1.setCursor(24, 2+27);
  tft1.print(in2_cur, 3);
  tft1.setCursor(56, 2+27);
  tft1.print("A");
  tft1.setCursor(24, 12+27);
  tft1.print(in2_vol, 3);
  tft1.setCursor(56, 12+27);
  tft1.print("V");

  tft1.setCursor(4, 12+73);
  tft1.print("Bat");
  
  tft1.setCursor(24, 2+73);
  tft1.print("Charge");
  tft1.setCursor(62, 2+73);
  tft1.print(in2_cur, 3);
  tft1.setCursor(94, 2+73);
  tft1.print("A");

  tft1.setCursor(24, 12+73);
  tft1.print("Dischr");
  tft1.setCursor(62, 12+73);
  tft1.print(in2_cur, 3);
  tft1.setCursor(94, 12+73);
  tft1.print("A");

  tft1.setCursor(62, 22+73);
  tft1.print(in2_vol, 3);
  tft1.setCursor(94, 22+73);
  tft1.print("V");

  tft1.setTextColor(BLUE, BLACK);

  tft1.setCursor(2+2, 2+106);
  tft1.print("Bus");
  tft1.setCursor(2+2, 12+106);
  tft1.print(" A ");
  tft1.setCursor(22+2, 2+106);
  tft1.print(in1_cur, 3);
  tft1.setCursor(54+2, 2+106);
  tft1.print("A");
  tft1.setCursor(22+2, 12+106);
  tft1.print(in1_vol, 3);
  tft1.setCursor(54+2, 12+106);
  tft1.print("V");

  tft1.setTextColor(BLUE, BLACK);

  tft1.setCursor(2+65, 2+4);
  tft1.print("OUT");
  tft1.setCursor(2+65, 12+4);
  tft1.print(" A ");
  tft1.setCursor(22+65, 2+4);
  tft1.print(in1_cur, 3);
  tft1.setCursor(54+65, 2+4);
  tft1.print("A");
  tft1.setCursor(22+65, 12+4);
  tft1.print(in1_vol, 3);
  tft1.setCursor(54+65, 12+4);
  tft1.print("V");

  tft1.setTextColor(GREEN, BLACK);

  tft1.setCursor(2+65, 2+27);
  tft1.print("OUT");
  tft1.setCursor(2+65, 12+27);
  tft1.print(" B ");
  tft1.setCursor(22+65, 2+27);
  tft1.print(in2_cur, 3);
  tft1.setCursor(54+65, 2+27);
  tft1.print("A");
  tft1.setCursor(22+65, 12+27);
  tft1.print(in2_vol, 3);
  tft1.setCursor(54+65, 12+27);
  tft1.print("V");

  tft1.setCursor(2+65, 2+50);
  tft1.print("OUT");
  tft1.setCursor(2+65, 12+50);
  tft1.print(" C ");
  tft1.setCursor(22+65, 2+50);
  tft1.print(in2_cur, 3);
  tft1.setCursor(54+65, 2+50);
  tft1.print("A");
  tft1.setCursor(22+65, 12+50);
  tft1.print(in2_vol, 3);
  tft1.setCursor(54+65, 12+50);
  tft1.print("V");

  tft1.setTextColor(GREEN, BLACK);

  tft1.setCursor(2+65, 2+106);
  tft1.print("Bus");
  tft1.setCursor(2+65, 12+106);
  tft1.print(" B ");
  tft1.setCursor(22+65, 2+106);
  tft1.print(in2_cur, 3);
  tft1.setCursor(54+65, 2+106);
  tft1.print("A");
  tft1.setCursor(22+65, 12+106);
  tft1.print(in2_vol, 3);
  tft1.setCursor(54+65, 12+106);
  tft1.print("V");
  
}

void update_parameters(){
  for(int i=0; i<9; i++){
    if(abs(normalised_values.current[i])>2000 || abs(normalised_values.current[i])<10){
      normalised_values.current[i] = 0;
    }
    else{
      normalised_values.current[i] = abs(normalised_values.current[i]);
    }
  }

  for(int i=0; i<8; i++){
    if(abs(normalised_values.voltage[i])>20000 || abs(normalised_values.voltage[i])<10){
      normalised_values.voltage[i] = 0;
    }
    else{
      normalised_values.voltage[i] = abs(normalised_values.voltage[i]);
    }
  }

  tft1.setTextColor(BLUE, BLACK);
  tft1.setCursor(24, 2+4);
  tft1.print(normalised_values.current[2]/1000.0, 3);
  tft1.setCursor(24, 12+4);
  if(normalised_values.voltage[6] >= 10000){
    tft1.print(normalised_values.voltage[6]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[6]/1000.0, 3);
  }
  

  tft1.setTextColor(GREEN, BLACK);
  
  tft1.setCursor(24, 2+27);
  tft1.print(normalised_values.current[3]/1000.0, 3);
  tft1.setCursor(24, 12+27);
  if(normalised_values.voltage[7] >= 10000){
    tft1.print(normalised_values.voltage[7]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[7]/1000.0, 3);
  }

  tft1.setCursor(62, 2+73);
  tft1.print(normalised_values.current[0]/1000.0, 3);
  tft1.setCursor(62, 12+73);
  tft1.print(normalised_values.current[1]/1000.0, 3);
  tft1.setCursor(62, 22+73);
  if(normalised_values.voltage[5] >= 10000){
    tft1.print(normalised_values.voltage[5]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[5]/1000.0, 3);
  }
  
  tft1.setTextColor(BLUE, BLACK);
  
  tft1.setCursor(22+2, 2+106);
  tft1.print(normalised_values.current[5]/1000.0, 3);
  tft1.setCursor(22+2, 12+106);
  if(normalised_values.voltage[3] >= 10000){
    tft1.print(normalised_values.voltage[3]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[3]/1000.0, 3);
  }
  
  tft1.setCursor(22+65, 2+4);
  tft1.print(normalised_values.current[7]/1000.0, 3);
  tft1.setCursor(22+65, 12+4);
  if(normalised_values.voltage[0] >= 10000){
    tft1.print(normalised_values.voltage[0]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[0]/1000.0, 3);
  }

  tft1.setTextColor(GREEN, BLACK);

  tft1.setCursor(22+65, 2+27);
  tft1.print(normalised_values.current[8]/1000.0, 3);
  tft1.setCursor(22+65, 12+27);
  if(normalised_values.voltage[1] >= 10000){
    tft1.print(normalised_values.voltage[1]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[1]/1000.0, 3);
  }
  tft1.setCursor(22+65, 2+50);
  tft1.print(normalised_values.current[4]/1000.0, 3);
  
  tft1.setCursor(22+65, 12+50);
  if(normalised_values.voltage[2] >= 10000){
    tft1.print(normalised_values.voltage[2]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[2]/1000.0, 3);
  }
  tft1.setCursor(22+65, 2+106);
  tft1.print(normalised_values.current[6]/1000.0, 3);
  tft1.setCursor(22+65, 12+106);
  if(normalised_values.voltage[4] >= 10000){
    tft1.print(normalised_values.voltage[4]/1000.0, 2);
  }
  else{
    tft1.print(normalised_values.voltage[4]/1000.0, 3);
  }

  tft2.setTextColor(RED, WHITE);
  short tmp_count = 0;
  for (int i=0; i<6; i++){
    if (normalised_values.warnings[i]){
      tft2.setCursor(4, 8*(tmp_count+1));
      tft2.print(Warnings[i]);
      tmp_count++;
    }
  }
  tft2.setTextColor(RED, WHITE);
  for (int i=tmp_count; i<7; i++){
    tft2.setCursor(4, 8*(i+1));
    tft2.print("                          ");
  }
}
