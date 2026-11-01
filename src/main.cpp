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
#define RED 13
#define GREEN 12
#define sensor 25

DHT dht(dht_pin, DHT11);

uint32_t last_check = 0;
uint32_t interval = 2000;

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

    Serial.println("Temp:");
    Serial.println(temp);

    Serial.println("Hum:");
    Serial.println(hum);

  }

}

void setup() {
  Serial.begin(9600); 
  
  pinMode(RELE1 , OUTPUT);
  pinMode(sensor , INPUT);
  pinMode(RED, OUTPUT); 
  pinMode(GREEN,OUTPUT);

  GPIO.out_w1ts=(1UL <<RELE1);

  setupDHT();

}

void loop() {

  if(GPIO.in & (1UL << sensor)){
    GPIO.out_w1tc=(1UL <<RELE1 );
    //Serial.println("RELEAY ON");
  }else{
    GPIO.out_w1ts=(1UL <<RELE1);
    //Serial.println("RELAY OFF");
  }

  DHT_update();

}
