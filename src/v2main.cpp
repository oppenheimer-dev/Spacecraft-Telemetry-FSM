#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <DHT.h>
#include "proto.h"  

uint32_t lastDHTCheck = 0;
volatile uint32_t last_click_iram = 0;
bool state_manual = false;

float temp = 0;
float hum = 0;

SystemState current_state = STATE_BOOT;
DHT dht(DHT_pin, DHTTYPE);

void update_FSM(){
    switch (current_state)
    {
    case STATE_BOOT:{

        break;
    }
    case STATE_AUTO:{

        break;
    }
    case STATE_MANUAL:{
        
        break;
    }
    case STATE_EMERGENCY:{

        break;
    }
    
    default:
        current_state = STATE_EMERGENCY;
        break;
    }
}

void setupDHT() {
  dht.begin();
}

void DHT_update(){
    uint32_t now = millis();

    if(now - lastDHTCheck >= dhtInterval){
    lastDHTCheck = now;

    float h = dht.readHumidity(); // CELSIUS
    float t = dht.readTemperature();// CELSIUS

    //Chech NaN
    if (isnan(h) ||isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    temp = t;
    hum = h;
    }
}

void IRAM_ATTR manual_switch_iram(void){
    uint64_t now = esp_timer_get_time() / 1000;

    if(now - last_click_iram >= dreb_delay){
        if (current_state == STATE_AUTO) {
            current_state = STATE_MANUAL;
} else if (current_state == STATE_MANUAL) {
        current_state = STATE_AUTO;
}

        state_manual = !state_manual;
    }
}

void setup() {
    Serial.begin(921600); 
  pinMode(Manual_mode, INPUT);
  pinMode(Relay_one , OUTPUT);
  pinMode(Relay_two , OUTPUT);
  pinMode(Button_light , INPUT_PULLUP);
  pinMode(Button_warm , INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Manual_mode), manual_switch_iram, FALLING);

  GPIO.out_w1ts=(1UL << Relay_one);
  GPIO.out_w1ts=(1UL << Relay_two);
    setupDHT(); 
 } 
 void loop() 
 { 
    update_FSM();
 }