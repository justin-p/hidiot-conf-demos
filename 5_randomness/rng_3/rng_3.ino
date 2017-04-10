#include <avr/eeprom.h>
#include "DigiKeyboard.h"

#define LED 1
#define BUT1 0
#define BLEN 500

/*==============================================================================
  Call reseedRandom once in setup to start random on a new sequence.  Uses
  four bytes of EEPROM.
==============================================================================*/

void reseedRandom( uint32_t* address )
{
  static const uint32_t HappyPrime = 127807 /*937*/;
  uint32_t raw;
  unsigned long seed;

  // Read the previous raw value from EEPROM
  raw = eeprom_read_dword( address );

  // Loop until a seed within the valid range is found
  do
  {
    // Incrementing by a prime (except 2) every possible raw value is visited
    raw += HappyPrime;

    // Park-Miller is only 31 bits so ignore the most significant bit
    seed = raw & 0x7FFFFFFF;
  }
  while ( (seed < 1) || (seed > 2147483646) );

  // Seed the random number generator with the next value in the sequence
  srandom( seed ); 

  // Save the new raw value for next time
  eeprom_write_dword( address, raw );
}

inline void reseedRandom( unsigned short address )
{
  reseedRandom( (uint32_t*)(address) );
}


/*==============================================================================
  So the reseedRandom raw value can be initialized allowing different
  applications or instances to have different random sequences.

  Generate initial raw values...

  https://www.random.org/cgi-bin/randbyte?nbytes=4&format=h
  https://www.fourmilab.ch/cgi-bin/Hotbits?nbytes=4&fmt=c&npass=1&lpass=8&pwtype=3

==============================================================================*/

void reseedRandomInit( uint32_t* address, uint32_t value )
{
  eeprom_write_dword( address, value );
}

inline void reseedRandomInit( unsigned short address, uint32_t value )
{
  reseedRandomInit( (uint32_t*)(address), value );
}


uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;


// Variables will change:
int ledState = HIGH;         
int buttonState;             
int lastButtonState = LOW;   

long lastDebounceTime = 0;
long debounceDelay = 50;

int getRandomNumber(){
  return random(6) + 1;
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
  reseedRandom( &reseedRandomSeed );
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
