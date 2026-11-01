#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define dht_pin 27
#define RELE1 4
#define RELE2 5
#define RED 13
#define GREEN 12
#define sensor 25

DHT dht(dht_pin, DHT11);
uint32_t green_interval = 1000;
uint32_t last_beat = 0;
uint32_t last_check = 0;
uint32_t last_activ = 0;
uint32_t last_send = 0;
uint32_t interval_rele = 10000;
uint32_t interval = 2000;
uint32_t pack_id = 0;
bool rele1_active = false;
bool rele2_active = false;
uint32_t uptime_ti = millis();
uint8_t rele_count = 0;
uint32_t mont_count = 0;


typedef struct {
    uint32_t uptime;      
    uint32_t id;
    uint8_t  relay_status;
    uint32_t motion_count;
} SpacecraftPacket;

float temp = 0;
float hum = 0;

void setupDHT() {
  dht.begin();
}

void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print(F("ST SYSTEM"));

    display.setCursor(0, 12);
    display.print(F("Temp: "));
    display.print(temp, 1);
    display.print(F("C"));

    display.setCursor(0, 22);
    display.print(F("Hum:  "));
    display.print(hum, 1);
    display.print(F("%"));

    display.setCursor(0, 34);
    display.print(F("Relays: "));
    display.print(rele_count);

    display.setCursor(0, 44);
    display.print(F("Motion: "));
    display.print(mont_count);

    display.setCursor(0, 54);
    display.print(F("ID: "));
    display.print(pack_id);

    display.display();
}

void DHT_update(){
  uint32_t current = millis();
  if(current - last_check >= interval){
    last_check = current;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if(isnan(h) || isnan(t)){
      Serial.println("read error");
      return;
    }

    temp = t;
    hum = h;

    if(temp < 27.0){
      GPIO.out_w1tc = (1UL << RELE2);
      rele2_active = true;
      Serial.println("CLIMATE CONTROL ON");
      GPIO.out_w1ts = (1UL << RED);
    }else{
      GPIO.out_w1tc = (1UL << RED);
      rele2_active = false;
      GPIO.out_w1ts = (1UL << RELE2);
      Serial.println("CLIMATE CONTROLL OFF");
    }

    Serial.println("Temp:");
    Serial.println(temp);

    Serial.println("Hum:");
    Serial.println(hum);

  }

}

void heartbeat(){
  uint32_t now = millis();

  if(now - last_beat >= green_interval){
    GPIO.out_w1ts = (1UL << GREEN);
    last_beat = now;
  }

  if(now - last_beat >= 80){
    GPIO.out_w1tc = (1UL << GREEN);
  }
}

void relay_control(){
  uint32_t now = millis();
  if(GPIO.in & (1UL << sensor)&& !rele1_active){
    GPIO.out_w1tc=(1UL <<RELE1 ); 
      rele1_active = true;
      last_activ = now;
      mont_count += 1;
  }

  if(rele1_active &(now - last_activ >= interval_rele)){ 
    GPIO.out_w1ts=(1UL << RELE1);
    rele1_active = false;
  }

}

void relay_status() {
    if (rele1_active && rele2_active) {
        rele_count = 3; 
    } 
    else if (rele1_active) {
        rele_count = 1;
    } 
    else if (rele2_active) {
        rele_count = 2;
    }else {
        rele_count = 0;
    }
}

void telemetry_send() {
    uint32_t now = millis();
    if(now - last_send >= 4000){
        last_send = now;
        pack_id += 1;

        relay_status();
        SpacecraftPacket telemetry_packet = {
            .uptime = now / 1000,        
            .id = pack_id,
            .relay_status = rele_count,
            .motion_count = mont_count
        };
        
        Serial.write((uint8_t*)&telemetry_packet, sizeof(telemetry_packet));
        updateDisplay();
    }
}

void setup() {
  Serial.begin(9600); 
  
  pinMode(RELE1 , OUTPUT);
  pinMode(RELE2 , OUTPUT);
  pinMode(sensor , INPUT);
  pinMode(RED, OUTPUT); 
  pinMode(GREEN,OUTPUT);

  GPIO.out_w1ts=(1UL << RELE1);
  GPIO.out_w1ts=(1UL << RELE2);

  setupDHT();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
  }

  display.clearDisplay();
  updateDisplay();
}

void loop() {
  relay_control();
  DHT_update();
  heartbeat();
  telemetry_send();
  updateDisplay();
}