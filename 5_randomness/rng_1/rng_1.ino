#include "DigiKeyboard.h"
#define LED 1
#define BUT1 0
#define BLEN 500

// Variables will change:
int ledState = HIGH;         
int buttonState;             
int lastButtonState = LOW;   

long lastDebounceTime = 0;
long debounceDelay = 50;

// Borrowed this code from Debian. If GNU thinks it's legit, it must be good.
int getRandomNumber(){
  return 4; // chosen by fair dice roll.
            // guaranteed to be random.
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
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
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
