#include "DigiKeyboard.h"
#include <Entropy.h>

#define LED 1
#define BUT1 0
#define BLEN 125

// Variables will change:
int ledState = HIGH;         
int buttonState;             
int lastButtonState = LOW;
uint8_t rnd;   

long lastDebounceTime = 0;
long debounceDelay = 50;

int getRandomNumber(){
  return Entropy.random(1,7);
}

void blinkLed(){
    digitalWrite(LED,HIGH);
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(BLEN/2);
    digitalWrite(LED,LOW);
    DigiKeyboard.delay(BLEN/2);
}

void bRand(){
  char str[16];
  int x;
  int r = getRandomNumber();
  for (x = 0; x < r; x++){
    blinkLed();
  }
  DigiKeyboard.write(itoa(r, str, 10));
}

void setup() {
  char seed[32];
  Entropy.initialize();
  uint32_t seed_value;
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  seed_value = Entropy.random();
  DigiKeyboard.write("Seed: ");
  DigiKeyboard.write(itoa(seed_value, seed, 10));
  randomSeed(seed_value);
}

void loop() {

  int reading = digitalRead(BUT1);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    buttonState = reading;
    if (buttonState == HIGH){
      digitalWrite(LED,LOW);
    } else {
      bRand();
    }
  }

  lastButtonState = reading;

  DigiKeyboard.update();
  DigiKeyboard.sendKeyStroke(0);
}
