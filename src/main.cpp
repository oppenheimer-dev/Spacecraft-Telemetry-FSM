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
uint32_t interval_rele = 10000;
uint32_t interval = 2000;
bool rele1_active = false;

float temp = 0;
float hum = 0;

void setupDHT() {
  dht.begin();
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
      Serial.println("CLIMATE CONTROL ON");
      GPIO.out_w1ts = (1UL << RED);
    }else{
      GPIO.out_w1tc = (1UL << RED);
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
  if(GPIO.in & (1UL << sensor)){
    GPIO.out_w1tc=(1UL <<RELE1 ); // ON
      rele1_active = true;
      last_activ = now;
  }

  if(rele1_active &(now - last_activ >= interval_rele)){ // offf
    GPIO.out_w1ts=(1UL << RELE1);
    rele1_active = false;
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

}

void loop() {
  relay_control();
  DHT_update();
  heartbeat();

}
