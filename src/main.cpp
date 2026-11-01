#include <Arduino.h>
#include <stdint.h>

#define RELE1 4
#define RED 13
#define GREEN 12
#define sensor 25

void setup() {
  Serial.begin(9600); 
  
  pinMode(RELE1 , OUTPUT);
  pinMode(sensor , INPUT);
  pinMode(RED, OUTPUT); 
  pinMode(GREEN,OUTPUT);

  GPIO.out_w1ts=(1UL <<RELE1);

}

void loop() {

  if(GPIO.in & (1UL << sensor)){
    GPIO.out_w1tc=(1 << GREEN );
    GPIO.out_w1tc=(1UL <<RELE1 );
    Serial.println("RELEAY ON");
    GPIO.out_w1ts=(1 << RED );
  }else{
    GPIO.out_w1tc=(1 << RED);
    GPIO.out_w1ts=(1UL <<RELE1);
    Serial.println("RELAY OFF");
    GPIO.out_w1ts=(1 << GREEN );
  }

  delay(200);
}
