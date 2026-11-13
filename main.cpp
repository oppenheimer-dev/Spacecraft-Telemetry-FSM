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

#define ldr_pin 34
#define dht_pin 27
#define RELE1 4
#define RELE2 5
#define RED 13
#define GREEN 12
#define sensor 25
#define kill_switch 0
#define manual_switch 26


DHT dht(dht_pin, DHT11);
uint32_t last_beat = 0;
uint32_t last_check = 0;
uint32_t last_activ = 0;
uint32_t last_send = 0;
uint32_t last_ldr = 0;
uint32_t last_click_switch = 0;
uint32_t last_display_update = 0;
volatile uint64_t last_click_iram = 0;
const uint32_t green_interval = 1000;
const uint32_t interval_rele = 10000;
const uint32_t dht_interval = 2000;
const uint32_t ldr_interval = 1000;
const uint32_t display_interval = 500;
const uint32_t dreb_delay = 200;
uint32_t pack_id = 0;
bool rele1_active = false;
bool rele2_active = false;
volatile bool manual = false;
uint32_t uptime_ti = millis();
uint32_t mont_count = 0;
uint16_t light_ldr = 0;
uint8_t sollar_status = 0;
uint8_t climat = 0;



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
      
  if(!manual){
    display.print(F("ST SYSTEM|AUTO"));
  }else{
    display.print(F("ST SYSTEM|MANUAL"));
  }

    
    

    display.setCursor(0, 10);
    display.print(F("Temp: "));
    display.print(temp, 1);
    display.print(F("C"));

    display.setCursor(0, 19);
    display.print(F("Hum:  "));
    display.print(hum, 1);
    display.print(F("%"));

    display.setCursor(0, 28); 
    display.print(F("SP: "));
    display.print(sollar_status); 


    display.setCursor(0, 37);
    display.print(F("Motion:  "));
    display.print(mont_count);
    
    display.setCursor(0, 46);
    display.print(F("ID: "));
    display.print(pack_id);

    display.setCursor(0, 55);
    display.print(F("by DR.OPPIE"));


    display.display();
}

void DHT_update(){
  uint32_t current = millis();
  if(current - last_check >= dht_interval){
    last_check = current;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if(isnan(h) || isnan(t)){
      Serial.println("read error");
      return;
    }

    temp = t;
    hum = h;

    noInterrupts();
    bool current_manual = manual;
    interrupts();

    if(current_manual){
      return;
    }
    if(temp < 23.5){
      GPIO.out_w1tc = (1UL << RELE2);
      rele2_active = true;
      Serial.println("CLIMATE CONTROL ON");
      GPIO.out_w1ts = (1UL << RED);
    }else if(temp > 24.5 && rele2_active) {
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
  if(GPIO.in && (1UL << sensor)&& !rele1_active){
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


void ldr_update(){
  uint32_t now = millis();

  if(now - last_ldr >= ldr_interval){
    last_ldr = now;
    if(GPIO.in1.val & (1UL << (ldr_pin - 32))){
      Serial.println("SOLLAR PANNELS DEPLOYED");
      sollar_status = 1;
    }else{
      Serial.println("SOLLAR PANNELS STOWED");
      sollar_status = 0;
    }
    
  }

}

void telemetry_send() {
    uint32_t now = millis();
    if(now - last_send >= 4000){
        last_send = now;
        pack_id += 1;

        SpacecraftPacket telemetry_packet = {
            .uptime = now / 1000,        
            .id = pack_id,
            .motion_count = mont_count,
            .S_stat = sollar_status
        };

        telemetry_packet.checksum = calculate_checksum(
            (uint8_t*)&telemetry_packet, 
            sizeof(SpacecraftPacket) - sizeof(uint8_t)
        );
        
        Serial.write((uint8_t*)&telemetry_packet, sizeof(telemetry_packet));
    }
}

void IRAM_ATTR manual_switch_iram(){
  uint64_t now = esp_timer_get_time() / 1000;
  if(now - last_click_iram > dreb_delay){
    last_click_iram = now;
    manual = !manual;
  }
}

void click(){
  uint32_t current_time = millis();

  noInterrupts();
  bool current_manual = manual;
  interrupts();

  if(current_manual &&(!(GPIO.in &(1UL << kill_switch)))){
    if (current_time - last_click_switch > dreb_delay){
      last_click_switch = current_time;
      rele2_active = !rele2_active;

      if(rele2_active){
          GPIO.out_w1tc = (1UL << RELE2);
          GPIO.out_w1ts = (1UL << RED);
        }else{
          GPIO.out_w1ts = (1UL << RELE2);
          GPIO.out_w1tc = (1UL << RED);
        }
    }
  }
}

void display_update() {
    uint32_t now = millis();
    if (now - last_display_update >= display_interval) {
        last_display_update = now;
        
        updateDisplay();
    }
}

void setup() {
  Serial.begin(921600); 
  pinMode(kill_switch, INPUT);
  pinMode(RELE1 , OUTPUT);
  pinMode(RELE2 , OUTPUT);
  pinMode(sensor , INPUT);
  pinMode(RED, OUTPUT); 
  pinMode(GREEN,OUTPUT);
  pinMode(ldr_pin,INPUT);
  pinMode(manual_switch , INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(manual_switch), manual_switch_iram, FALLING);

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
  ldr_update();
  heartbeat();
  telemetry_send();
  click();
  display_update();
}//ad astra broo! Prod by Dr.oppie 