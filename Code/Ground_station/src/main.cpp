#include <Arduino.h>
#include "GyverTimers.h"
#include <SPI.h>
#include <Arduino_ST7735_Fast.h>

#include <SoftwareSerial.h>

#define MUL_0 4
#define MUL_1 3
#define MUL_2 2
#define MUL_3 A0

#define BTN_0 A1
#define BTN_1 A2

#define LED_0G 9
#define LED_0R 10
#define LED_1G 5
#define LED_1R 6

#define payload_A_bus_A 1
#define payload_A_bus_B 21
#define payload_B_bus_A 19
#define payload_B_bus_B 5
#define payload_C_bus_A 4
#define payload_C_bus_B 18

#define IN1_bus_A 13
#define IN1_bus_B 9
#define IN2_bus_A 11
#define IN2_bus_B 17

#define BAT_bus_A 16
#define BAT_bus_B 10
#define BAT_chr_IN1 8
#define BAT_chr_IN2 12

#define IN1_syste_power 20
#define IN2_syste_power 0
#define BAT_syste_power 6

#define IN1_en 15
#define IN2_en 3
#define BAT_en 2
#define BAT_chr_en 14
#define auto 7

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
struct cell
{
  float red_val = 0;
  float green_val = 0;
  short red_val_last = 0;
  short green_val_last = 0;
  float red_val_cur = 0;
  float green_val_cur = 0;
  bool last_last_state = 1;
  bool last_state = 1;
  bool unprocessed_click = 0;
  short click_count = 0;

  void update(volatile uint8_t& p1, volatile uint8_t& p2, short pins[3]){
    green_val_cur += green_val;
    if(int(green_val_cur) > green_val_last){
      green_val_last ++;
      bitSet(p1, pins[0]);
    }
    else{
      bitClear(p1, pins[0]);
    }
    if(green_val_last == 1000){
      green_val_last=0;
      green_val_cur=0;
    }

    red_val_cur += red_val;
    if(int(red_val_cur) > red_val_last){
      red_val_last ++;
      bitSet(p1, pins[1]);
    }
    else{
      bitClear(p1, pins[1]);
    }
    if(red_val_last == 1000){
      red_val_last=0;
      red_val_cur=0;
    }

    bool BTN0_state = (p2 & 1<<pins[2]) >> pins[2];

    if(last_last_state == 1 && last_state == 0 && BTN0_state == 0){
      click_count++;
      unprocessed_click=1;
      
    }
    last_last_state = last_state;
    last_state = BTN0_state;
  }
};

short cur_raw=0;
cell grid[22];

byte buttons_mapping[] = {1, 21, 19, 5, 4, 18, 13, 9, 11, 17, 16, 10, 8, 12, 20, 0, 6, 15, 3, 2 ,14, 7};

struct Normalised_values2{
  short voltage[1];
  byte switch_states[22];
  bool warnings[6];
};

Normalised_values2 normalised_values2;

void update_parameters();

void setup() {
  Serial.begin(57600);
  //radio.begin(300);
  
  pinMode(MUL_0, OUTPUT);
  pinMode(MUL_1, OUTPUT);
  pinMode(MUL_2, OUTPUT);
  pinMode(MUL_3, OUTPUT);

  pinMode(LED_0G, OUTPUT);
  pinMode(LED_0R, OUTPUT);
  pinMode(LED_1G, OUTPUT);
  pinMode(LED_1R, OUTPUT);

  Timer2.setPeriod(150);
  Timer2.enableISR(CHANNEL_A);
  Timer2.restart();
}

ISR(TIMER2_A) {
  cur_raw++;
  if(cur_raw == 11){
    cur_raw = 0;
  }
  short cur_raw_tmp = cur_raw+5;

  bitClear(PORTB, 2);
  bitClear(PORTB, 1);
  bitClear(PORTD, 6);
  bitClear(PORTD, 5);
  
  bitWrite(PORTD, 4, cur_raw_tmp%2);
  bitWrite(PORTD, 3, (cur_raw_tmp/2)%2);
  bitWrite(PORTD, 2, (cur_raw_tmp/4)%2);
  bitWrite(PORTC, 0, cur_raw_tmp/8);

  short tmp[3] = {2, 1, 1};
  grid[cur_raw*2].update(PORTB, PINC, tmp);
  short tmp1[3] = {6, 5, 2};
  grid[cur_raw*2+1].update(PORTD, PINC, tmp1);
}

void loop() {
  /*for(int i=0; i<22; i++){
    Serial.print(grid[i].red_val);
    Serial.print(" ");
  }
  Serial.println();*/

  for(int i=0; i<22; i++){
    if (grid[i].unprocessed_click){
      Serial.write("B");
      Serial.write("u");
      for (int j=0; j<22; j++){
        if(buttons_mapping[j] == i){
          Serial.write(j);
        }
      }
      grid[i].unprocessed_click = 0;
    }
  }
  //Serial.println(Serial.available());
  if(Serial.available() >= sizeof(normalised_values2)){
    Serial.readBytes((byte*)&normalised_values2, sizeof(Normalised_values2));

    for (int i=0; i<21; i++){
      Serial.println(normalised_values2.switch_states[i]);
      if(normalised_values2.switch_states[i] == 0){
        grid[buttons_mapping[i]].green_val = 1;
        grid[buttons_mapping[i]].red_val = 1;
      }
      else if (normalised_values2.switch_states[i] == 1){
        grid[buttons_mapping[i]].green_val = 1;
        grid[buttons_mapping[i]].red_val = 0;
      }
      else if (normalised_values2.switch_states[i] == 2){
        grid[buttons_mapping[i]].green_val = 0.3;
        grid[buttons_mapping[i]].red_val = 1;
      }
      else if (normalised_values2.switch_states[i] == 3){
        grid[buttons_mapping[i]].green_val = 0;
        grid[buttons_mapping[i]].red_val = 1;
      }
    }

    
    while(Serial.available() > 0) {
      char t = Serial.read();
    }
  }
}