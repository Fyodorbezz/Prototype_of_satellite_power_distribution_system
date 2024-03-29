#include <Arduino.h>
#include<SPI.h>
#include <Wire.h>
#include<Adafruit_ADS1X15.h>
#include <Adafruit_PCF8575.h>
#include <Adafruit_PCF8574.h>


#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define ADC1_INT 2
#define ADC2_INT 3
#define ADC1_MUL_A 4
#define ADC1_MUL_B 5
#define ADC2_MUL_A 6
#define ADC2_MUL_B 7

#define VOL_CHN_0 A6
#define VOL_CHN_1 A7
#define VOL_MUL_A A0
#define VOL_MUL_B A1

#define BAT_VIN_CNTRL A2
#define SOL1_VIN_CNTRL A3
#define SOL2_VIN_CNTRL 8

#define BAT_CHR_1 2
#define BAT_CHR_2 3

#define BAT_BUS_A 4
#define BAT_BUS_B 5

#define IN1_BUS_A 6
#define IN1_BUS_B 7

#define IN2_BUS_A 11
#define IN2_BUS_B 10

#define BUS_A_LOAD_A 8
#define BUS_B_LOAD_A 9
#define BUS_A_LOAD_B 13
#define BUS_B_LOAD_B 12
#define BUS_A_LOAD_C 14
#define BUS_B_LOAD_C 15

#define RADIO_CE 9
#define RADIO_CSN 10

byte switch_mapping[] = {8, 9, 13, 12, 14, 15, 6, 7, 11, 10, 4, 5, 2, 3, A3, 8, A2, 4, 2, 0, 1};
byte switch_states[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

byte sensor_mapping[] = {3, 4, 3, 4, 3, 4, 6, 6, 7, 7, 5, 5, 6, 7, 6, 7, 5};

struct Normalised_values{
  short current[9];
  short voltage[9];
};

struct Normalised_values2{
  short voltage[1];
  byte switch_states[22];
  bool warnings[6];
};

Normalised_values normalised_values;
Normalised_values2 normalised_values2;

void change_state(short index, bool inv){
  if (switch_states[index] == inv && normalised_values.voltage[sensor_mapping[index]] > 100){
    normalised_values2.switch_states[index] = 0;
  }
  else if (switch_states[index] == !inv && normalised_values.voltage[sensor_mapping[index]] > 100){
    normalised_values2.switch_states[index] = 1;
  }
  else if (switch_states[index] == !inv && normalised_values.voltage[sensor_mapping[index]] < 100){
    normalised_values2.switch_states[index] = 2;
  }
  else if (switch_states[index] == inv && normalised_values.voltage[sensor_mapping[index]] < 100){
    normalised_values2.switch_states[index] = 3;
  }
}




float current_normalised[9];
float voltage_normalised[8];

int current_ranges[9] = {20, 20, 5, 5, 5, 5, 5, 5, 5};
float voltage_multipliers[8] = {2.4286, 2.4286, 2.4286, 2.4286, 2.4286, 1, 2, 2};

struct adc{
  Adafruit_ADS1115 ads;
  bool ready = 0;
  short mul_state = 0;
  float results[5][5];
  float filtered[5];
  bool skip=0;

  void update(short number, float ref){
    if(ready){
      if(skip){
        skip = 0;
        ads.getLastConversionResults();
      }
      else{
        short mul_state_old = mul_state;
        skip = 1;
        //Serial.println(millis());
        float result = ads.computeVolts(ads.getLastConversionResults());
        //float result=0;
        //Serial.println(millis());

        mul_state++;
        if (mul_state == 5){
          mul_state = 0;
          skip = 1;
          ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, true);
        }
        if(mul_state == 4){
          skip = 1;
          if (number == 0){
            ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_3, true);
          }
          else{
            ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, true);
          }
        }
        else{
          if (number == 0){
            bitWrite(PORTD, 4, mul_state & 1);
            bitWrite(PORTD, 5, (mul_state & 2)>>1);
          }
          else{
            bitWrite(PORTD, 6, mul_state & 1);
            bitWrite(PORTD, 7, (mul_state & 2)>>1);
          }
        }

        for(int i=1; i<5; i++){
          results[mul_state_old][i-1] = results[mul_state_old][i];
        }
        results[mul_state_old][4] = result;
        float filter_tmp[5];
        for(int i=0; i<5; i++){
          filter_tmp[i] = results[mul_state_old][i];
        }
        for(int i=0; i<5; i++){
          for(int j=0; j<5-i-1; j++){
            if(filter_tmp[j] > filter_tmp[j+1]){
              float tmp = filter_tmp[j];
              filter_tmp[j] = filter_tmp[j+1];
              filter_tmp[j+1] = tmp;
            }
          }
        }
        filtered[mul_state_old] = filter_tmp[2];
        //Serial.println((filtered[mul_state_old]/(ref/2)));
        if(!(mul_state_old == 4 && !number)){
          float tmp = ((ref/2 - filtered[mul_state_old])/(ref/2))*current_ranges[mul_state_old+4*number];
          tmp*=-1;
          tmp -= 0.01;
          if (tmp < 0){
            tmp = 0;
          }
          tmp *= 1000;
          normalised_values.current[mul_state_old+4*number] = int(tmp+0.5);
        }
        ready = 0;
      }


    }
  }
};

adc adc1;
adc adc2;

Adafruit_PCF8575 pcf;
Adafruit_PCF8574 pcf2;

RF24 radio(RADIO_CE, RADIO_CSN);
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

unsigned long long data_send_timer=0;

void ADC1_ready(){
  adc1.ready = 1;
}

void ADC2_ready(){
  adc2.ready = 1;
}

short adc0_multiplecsor_state = 0;
int adc0_results[8][5];
int adc0_filtered[8];
bool start_flag = 1;

void ADC0_update(){
  if(!bit_is_set(ADCSRA, ADSC) || start_flag){
    start_flag = 0;
    uint8_t low, high;

    low  = ADCL;
	  high = ADCH;

    short adc0_multiplecsor_state_old = adc0_multiplecsor_state;

    adc0_multiplecsor_state++;
    if(adc0_multiplecsor_state == 8){
      adc0_multiplecsor_state = 0;
    }
    bitWrite(PORTC, 1, (adc0_multiplecsor_state & 2)>>1);
    bitWrite(PORTC, 0, adc0_multiplecsor_state & 1);
    ADCSRB = (ADCSRB & ~(1 << 0)) | ((((6 + ((adc0_multiplecsor_state & 4)>>2)) >> 3) & 0x01) << 0);
    ADMUX = (1 << 6) | ((6 + ((adc0_multiplecsor_state & 4)>>2)) & 0x07);
    delayMicroseconds(100);
    sbi(ADCSRA, ADSC);

    for(int i=1; i<5; i++){
          adc0_results[adc0_multiplecsor_state_old][i-1] = adc0_results[adc0_multiplecsor_state_old][i];
    }
    adc0_results[adc0_multiplecsor_state_old][4] = (high << 8) | low;
    int filter_tmp[5];
    for(int i=0; i<5; i++){
      filter_tmp[i] = adc0_results[adc0_multiplecsor_state_old][i];
    }
    for(int i=0; i<5; i++){
      for(int j=0; j<5-i-1; j++){
        if(filter_tmp[j] > filter_tmp[j+1]){
          float tmp = filter_tmp[j];
          filter_tmp[j] = filter_tmp[j+1];
          filter_tmp[j+1] = tmp;
        }
      }
    }
    adc0_filtered[adc0_multiplecsor_state_old] = filter_tmp[2];
    float tmp = ((adc0_filtered[adc0_multiplecsor_state_old]/1024.0)*adc1.filtered[4])*voltage_multipliers[adc0_multiplecsor_state_old];
    tmp -= 0.01;
    if (tmp < 0){
      tmp = 0;
    }
    tmp *= 1000;
    normalised_values.voltage[adc0_multiplecsor_state_old] = int(tmp+0.5);
  }
}

void setup() {
  Serial.begin(500000);

  pinMode(ADC1_MUL_A, OUTPUT);
  pinMode(ADC1_MUL_B, OUTPUT);
  pinMode(ADC2_MUL_A, OUTPUT);
  pinMode(ADC2_MUL_B, OUTPUT);
  pinMode(VOL_MUL_A, OUTPUT);
  pinMode(VOL_MUL_B, OUTPUT);

  pinMode(SOL1_VIN_CNTRL, OUTPUT);
  pinMode(SOL2_VIN_CNTRL, OUTPUT);
  pinMode(BAT_VIN_CNTRL, OUTPUT);

  digitalWrite(SOL1_VIN_CNTRL, LOW);
  digitalWrite(SOL2_VIN_CNTRL, LOW);
  digitalWrite(BAT_VIN_CNTRL, LOW);

  attachInterrupt(0, ADC1_ready, FALLING);
  attachInterrupt(1, ADC2_ready, FALLING);

  adc1.ads.begin(0x48);
  adc2.ads.begin(0x4B);
  adc1.ads.setDataRate(RATE_ADS1115_860SPS);
  adc2.ads.setDataRate(RATE_ADS1115_860SPS);
  adc1.ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, true);
  adc2.ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, true);
  
  pcf.begin(0x20, &Wire);
  pcf2.begin(0x39 ,&Wire);
  pcf.pinMode(15, OUTPUT);
  pcf.pinMode(14, OUTPUT);
  pcf.pinMode(13, OUTPUT);
  pcf.pinMode(12, OUTPUT);
  pcf.pinMode(11, OUTPUT);
  pcf.pinMode(10, OUTPUT);
  pcf.pinMode(9, OUTPUT);
  pcf.pinMode(8, OUTPUT);
  pcf.pinMode(7, OUTPUT);
  pcf.pinMode(6, OUTPUT);
  pcf.pinMode(5, OUTPUT);
  pcf.pinMode(4, OUTPUT);
  pcf.pinMode(3, OUTPUT);
  pcf.pinMode(2, OUTPUT);
  pcf.pinMode(1, OUTPUT);
  pcf.pinMode(0, OUTPUT);


  pcf2.pinMode(0, OUTPUT);
  pcf2.pinMode(1, OUTPUT);
  pcf2.pinMode(2, OUTPUT);
  pcf2.pinMode(3, OUTPUT);
  pcf2.pinMode(4, OUTPUT);
  pcf2.pinMode(5, OUTPUT);
  pcf2.pinMode(6, OUTPUT);
  pcf2.pinMode(7, OUTPUT);
  pcf2.digitalWrite(0, LOW);
  pcf2.digitalWrite(1, HIGH);
  pcf2.digitalWrite(2, LOW);
  pcf2.digitalWrite(3, LOW);
  pcf2.digitalWrite(4, LOW);
  pcf2.digitalWrite(5, LOW);
  pcf2.digitalWrite(6, LOW);
  pcf2.digitalWrite(7, LOW);

  pcf.digitalWrite(15, LOW);
  pcf.digitalWrite(14, LOW);
  pcf.digitalWrite(13, LOW);
  pcf.digitalWrite(12, LOW);
  pcf.digitalWrite(11, LOW);
  pcf.digitalWrite(10, LOW);
  pcf.digitalWrite(9, LOW);
  pcf.digitalWrite(8, LOW);
  pcf.digitalWrite(7, LOW);
  pcf.digitalWrite(6, LOW);
  pcf.digitalWrite(5, LOW);
  pcf.digitalWrite(4, LOW);
  pcf.digitalWrite(3, LOW);
  pcf.digitalWrite(2, LOW);
  pcf.digitalWrite(1, LOW);
  pcf.digitalWrite(0, LOW);

  Serial.print(radio.begin());
  //delay(1000000);
  //radio.setAutoAck(1);
  //radio.setRetries(0, 15);
  //radio.enableAckPayload();
  //radio.setPayloadSize(32);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.setChannel(0x5A);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setPayloadSize(40);
  radio.powerUp();
  radio.startListening();

  byte tmp = 'S';
  radio.write(&tmp, 1);

}

void loop() {
  if(millis() - data_send_timer>=200){
    data_send_timer = millis();
    for(int i =0; i<5; i++){
      Serial.print(adc1.filtered[i]);
      Serial.print(" ");
    }
    Serial.print("  ");
    for(int i =0; i<5; i++){
      Serial.print(adc2.filtered[i]);
      Serial.print(" ");
    }
    Serial.print("  ");
    for(int i =0; i<9; i++){
      Serial.print(normalised_values.current[i]);
      Serial.print(" ");
    }
    Serial.print("  ");
    for(int i =0; i<8; i++){
      Serial.print(adc0_filtered[i]);
      Serial.print(" ");
    }
    Serial.print("  ");
    for(int i =0; i<9; i++){
      Serial.print(normalised_values.voltage[i]);
      Serial.print(" ");
    }
    Serial.println("");

    normalised_values2.voltage[0] = normalised_values.voltage[7];
    //for (int i=0; i<14; i++){
    //  normalised_values2.switch_states[i] = switch_states[i];
    //}

    radio.stopListening();
    
    radio.write(&normalised_values, 32);
    radio.write(&normalised_values2, sizeof(normalised_values2));
    radio.startListening();
  }

  if(radio.available()){
    byte tmp[3];
    radio.read(tmp, 3);
    Serial.println(tmp[2]);
    Serial.println(switch_mapping[tmp[2]]);
    if (tmp[0] == 'B' && tmp[1] == 'u'){
      if (tmp[2] < 14){
        if (switch_states[tmp[2]] == 0){
          switch_states[tmp[2]] = 1;
          pcf.digitalWrite(switch_mapping[tmp[2]], 1);
        }
        else if (switch_states[tmp[2]] == 1){
          switch_states[tmp[2]] = 0;
          pcf.digitalWrite(switch_mapping[tmp[2]], 0);
        }
      }
      else if(tmp[2] >= 14 && tmp[2] < 17){
        if (switch_states[tmp[2]] == 0){
          switch_states[tmp[2]] = 1;
          digitalWrite(switch_mapping[tmp[2]], 1);
        }
        else if (switch_states[tmp[2]] == 1){
          switch_states[tmp[2]] = 0;
          digitalWrite(switch_mapping[tmp[2]], 0);
        }
      }
      else if(tmp[2] >= 17 && tmp[2] < 21){
        if (switch_states[tmp[2]] == 0){
          switch_states[tmp[2]] = 1;
          pcf2.digitalWrite(switch_mapping[tmp[2]], 1);
        }
        else if (switch_states[tmp[2]] == 1){
          switch_states[tmp[2]] = 0;
          pcf2.digitalWrite(switch_mapping[tmp[2]], 0);
        }
      }
    }
  }

  for (int i=0; i<14; i++){
    change_state(i, 0);
  }
  for (int i=14; i<17; i++){
    change_state(i, 1);
  }

  if(switch_states[2] || switch_states[3]){
    normalised_values2.warnings[5] = 0;
  }
  if (normalised_values.current[8] > 150){
    switch_states[2] = 0;
    switch_states[3] = 0;
    pcf.digitalWrite(switch_mapping[2], 0);
    pcf.digitalWrite(switch_mapping[3], 0);
    normalised_values2.warnings[5] = 1;
  }

  if (switch_states[17]){
    normalised_values2.switch_states[17] = 3;
  }
  else if (switch_states[17] == 0){
    normalised_values2.switch_states[17] = 1;
  }

  if (switch_states[18]){
    normalised_values2.switch_states[18] = 3;
  }
  else if (switch_states[18] == 0){
    normalised_values2.switch_states[18] = 1;
  }

  if (switch_states[19]){
    normalised_values2.switch_states[19] = 3;
  }
  else if (switch_states[19] == 0){
    normalised_values2.switch_states[19] = 1;
  }

  if (switch_states[20]){
    normalised_values2.switch_states[20] = 3;
  }
  else if (switch_states[20] == 0){
    normalised_values2.switch_states[20] = 1;
  }

  if(normalised_values.voltage[6] < 1000){
    normalised_values2.warnings[0] = 1;
  }
  else{
    normalised_values2.warnings[0] = 0;
  }
  if(normalised_values.voltage[7] < 1000){
    normalised_values2.warnings[1] = 1;
  }
  else{
    normalised_values2.warnings[1] = 0;
  }
  
  if(normalised_values.voltage[0] < 1000){
    normalised_values2.warnings[2] = 1;
  }
  else{
    normalised_values2.warnings[2] = 0;
  }
  if(normalised_values.voltage[1] < 1000){
    normalised_values2.warnings[3] = 1;
  }
  else{
    normalised_values2.warnings[3] = 0;
  }
  if(normalised_values.voltage[2] < 1000){
    normalised_values2.warnings[4] = 1;
  }
  else{
    normalised_values2.warnings[4] = 0;
  }


  //Serial.print("a");
  //Serial.println(millis());
  //Serial.print(micros());
  //Serial.print(" ");
  //analogRead(VOL_CHN_0);
  //Serial.println(micros());
  
  adc1.update(0, adc1.filtered[4]);
  //Serial.print("b");
  //Serial.println(millis());
  adc2.update(1, adc1.filtered[4]);
  //Serial.print("c");
  //Serial.println(millis());
  ADC0_update();
  //Serial.print("d");
  //Serial.println(millis());
}