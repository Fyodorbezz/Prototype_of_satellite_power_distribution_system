#include <Arduino.h>
#include <SoftwareSerial.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define RADIO_CE 9
#define RADIO_CSN 10

RF24 radio(RADIO_CE, RADIO_CSN);
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

SoftwareSerial host (2, 3);

struct Normalised_values{
  short current[9];
  short voltage[9];
  bool warnings[6];
};

struct Normalised_values2{
  short voltage[1];
  byte switch_states[22];
  bool warnings[6];
};

Normalised_values normalised_values;
Normalised_values2 normalised_values2;


void setup() {
  host.begin(57600);
  Serial.begin(115200);
  
  radio.begin();
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.setChannel(0x5A);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setPayloadSize(40);
  radio.powerUp();
  radio.startListening();
}

bool synk = 0;
bool flag = 0;

unsigned long long update_timer = 0;

void loop() {
  if(radio.available()){
    if (!synk){
      byte tmp;
      radio.read(&tmp, 1);
      if (tmp == 'S'){
        synk = 1;
      }
      normalised_values.voltage[0] = -10;
      Serial.write((byte*)&normalised_values, sizeof(Normalised_values));
    }
    else{
      if (!flag){
        radio.read(&normalised_values, 32);
        flag = 1;
        normalised_values.voltage[7] = normalised_values2.voltage[0];
        for (int i=0; i<6; i++){
          normalised_values.warnings[i] = normalised_values2.warnings[i];
        }
        Serial.write((byte*)&normalised_values, sizeof(Normalised_values));
      }
      else{
        byte tmp[5];
        radio.read(&normalised_values2, sizeof(Normalised_values2));
        flag = 0;
        host.write((byte*)&normalised_values2, sizeof(Normalised_values2));
      }
      update_timer = millis();
    }
    /*for(int i =0; i<9; i++){
      Serial.print(normalised_values.current[i]);
      Serial.print(" ");
    }
    Serial.print("  ");
    for(int i =0; i<8; i++){
      Serial.print(normalised_values.voltage[i]);
      Serial.print(" ");
    }
    Serial.println(normalised_values2.voltage[0]);*/
  }
  if(host.available() >= 3){
    byte tmp[3];
    host.readBytes(tmp, 3);
    //Serial.println("gk");
    if (tmp[0] == 'B' && tmp[1] == 'u'){
      radio.stopListening();
      radio.write(&tmp, 3);
      radio.startListening();
    }
  }
}