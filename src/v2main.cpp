#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <DHT.h>
#include "proto.h"  

uint32_t lastDHTCheck = 0;
uint32_t lastSolarCheck = 0;
uint32_t last_solar = 0;
volatile uint32_t last_click_iram = 0;

float temp = 0;
float hum = 0;

volatile SystemState current_state = STATE_BOOT;
volatile SolarState solar_state = SOLAR_STOWED;
volatile uint8_t is_solar_good = 0;
volatile bool solar_bool = false;
DHT dht(DHT_pin, DHTTYPE);

void setupDHT() {
  dht.begin();
}

void DHT_update(){
    uint32_t now = millis();

    if(now - lastDHTCheck >= dhtInterval){
    lastDHTCheck = now;

    float h = dht.readHumidity(); // CELSIUS
    float t = dht.readTemperature();// CELSIUS

    //Check NaN
    if (isnan(h) ||isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    temp = t;
    hum = h;
    }
}

void sollar_deploy() {
  uint32_t now = millis();
  if (solar_state == SOLAR_DEPLOYED) return;
  if (!solar_bool) {
    solar_bool = true;
    solar_state = SOLAR_DEPLOYING;
    last_solar = now;
  }

  if (solar_state == SOLAR_DEPLOYING && (now - last_solar >= 5000)) {
    solar_state = SOLAR_DEPLOYED;
  }
}


void sollar_panels(){
    uint32_t now = millis();
  if (now - lastSolarCheck < solarInterval) {
    return;
  }
  lastSolarCheck = now;
    uint16_t solar_read = analogRead(solar);
    static float filtered_solar = solar_read; 
    const float k = 0.1f;
    filtered_solar = filtered_solar + k * (solar_read - filtered_solar);
    uint16_t solar_map_read = map((int32_t)filtered_solar, 0 ,4095,0 , 100);

    if(solar_map_read <= 8){
        solar_state = SOLAR_STOWED;
        solar_bool = false;
        is_solar_good = 0;
    }
    else if(solar_map_read >= 12 && solar_map_read <= 48){
        is_solar_good = 1;
        sollar_deploy();
    }
    else if (solar_map_read >= 52){
        is_solar_good = 2;
        sollar_deploy();
    }
  
    
  }

void IRAM_ATTR manual_switch_iram(void){
    uint64_t now = esp_timer_get_time() / 1000;

    if(now - last_click_iram >= dreb_delay){
        last_click_iram = now;
        if (current_state == STATE_AUTO) {
            current_state = STATE_MANUAL;
} else if (current_state == STATE_MANUAL) {
        current_state = STATE_AUTO;
}

    }
}

void climate_control() {
    if (temp < 23.5f) {
        GPIO.out_w1tc = (1UL << Relay_two); //  ON
    } else if (temp > 24.5f) {
        GPIO.out_w1ts = (1UL << Relay_two); //  OFF
    }
}

void manual_control(){
    static uint32_t last_click = 0;
    static bool light_state = false;
    static bool warm_state = false;
    static bool last_btn_light = true;
    static bool last_btn_warm = true;

    uint32_t now = millis();

    if(now - last_click >= 150){ 
        last_click = now;


        bool btn_light = (GPIO.in & (1UL << Button_light));
        bool btn_warm  = (GPIO.in & (1UL << Button_warm));

        // Если кнопку света только что НАЖАЛИ 1 - 0 
        if (!btn_light && last_btn_light) {
            light_state = !light_state;
            if (light_state) GPIO.out_w1tc = (1UL << Relay_one);
            else             GPIO.out_w1ts = (1UL << Relay_one);
        }

        // Если кнопку тепла только что НАЖАЛИ 1 - 0
        if (!btn_warm && last_btn_warm) {
            warm_state = !warm_state;
            if (warm_state) GPIO.out_w1tc = (1UL << Relay_two);
            else            GPIO.out_w1ts = (1UL << Relay_two);
        }

        last_btn_light = btn_light;
        last_btn_warm = btn_warm;
    }
}

void update_FSM(){
    switch (current_state)
    {
    case STATE_BOOT: {
        if (millis() >= 2000) {
            float h = dht.readHumidity();
            float t = dht.readTemperature();
            
            if (isnan(h) || isnan(t)) {
                Serial.println(F("[CRITICAL] DHT11 Boot Failure!"));
                current_state = STATE_EMERGENCY;
            } else {
                temp = t;
                hum = h;
                lastDHTCheck = millis();
                Serial.println(F("[SYSTEM] Boot OK -> Switching to AUTO"));
                current_state = STATE_AUTO;
            }
        }
        break;
    }
    case STATE_AUTO:{
        DHT_update();
        sollar_panels();
        climate_control();
        GPIO.out_w1tc = (1UL << BLUE);
        break;
    }
    case STATE_MANUAL:{
        DHT_update();
        sollar_panels();
        manual_control();
        GPIO.out_w1ts = (1UL << BLUE);
        break;
    }
    case STATE_EMERGENCY:{
        GPIO.out_w1ts = (1UL << Relay_one);
        GPIO.out_w1ts = (1UL << Relay_two);
        current_state = STATE_MANUAL;
        break;
    }
    
    default:
        current_state = STATE_EMERGENCY;
        break;
    }
}

void send_telemetry() {
    static uint32_t last_send = 0;
    uint32_t now = millis();
    
    if (now - last_send < 1000) return;
    last_send = now;

    TelemetryPacket packet;
    packet.sync1 = SYNC_BYTE_1; // 0xAA
    packet.sync2 = SYNC_BYTE_2; // 0x55
    packet.hum = (uint8_t)hum;
    packet.temp = (uint16_t)(temp * 10.0f); // 24.5C -> 245
    
    packet.relay1 = !(GPIO.out & (1UL << Relay_one)); 
    packet.relay2 = !(GPIO.out & (1UL << Relay_two)); 
    
    packet.light_sensor_state = is_solar_good;
    packet.manual = (current_state == STATE_MANUAL) ? 1 : 0;

    packet.check_sum = calculate_checksum((uint8_t*)&packet, sizeof(TelemetryPacket) - 1);

    Serial.write((uint8_t*)&packet, sizeof(TelemetryPacket));
}

void setup() {
    Serial.begin(921600); 
  pinMode(Manual_mode, INPUT);
  pinMode(solar, INPUT);

  pinMode(Relay_one , OUTPUT);
  pinMode(Relay_two , OUTPUT);
  pinMode(Button_light , INPUT_PULLUP);
  pinMode(Button_warm , INPUT_PULLUP);
  pinMode(BLUE, OUTPUT); 
  attachInterrupt(digitalPinToInterrupt(Manual_mode), manual_switch_iram, FALLING);

  GPIO.out_w1ts=(1UL << Relay_one);
  GPIO.out_w1ts=(1UL << Relay_two);
    setupDHT(); 
 } 
 void loop() 
 { 
    update_FSM();
    send_telemetry();
 }// I`m very tired.Ad astra bro! 
 // By Dr.oppie
 //14.11.2026|18:18