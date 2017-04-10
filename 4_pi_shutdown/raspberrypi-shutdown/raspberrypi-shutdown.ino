  #include "DigiKeyboard.h"
  #define LED 1
  #define BUT1 0
  #define BUT2 2

int buttonState;
int button2State;
int lastButtonstate;
int lastButton2state;
int shuttingDown = 0;
int pos = 1;
unsigned long lastDebounceTime = 0;
unsigned long lastDebounceDelay = 0;
int pin[] = {1,2,1,2};
int pinlen = 3; // One less than actual length because of my crappy coding
int pressed = 0;

void login(){
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(250);
    DigiKeyboard.write("pi");
    DigiKeyboard.sendKeyStroke(KEY_ENTER);  
    DigiKeyboard.delay(250);
    DigiKeyboard.write("raspberry");
    DigiKeyboard.sendKeyStroke(KEY_ENTER);  
}

void shutoff(){
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(250);
    DigiKeyboard.write(" sudo shutdown -h now");
    DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

void consoleSwitch(){
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(250);
    DigiKeyboard.sendKeyStroke(KEY_F1, MOD_CONTROL_LEFT| MOD_ALT_LEFT);
    DigiKeyboard.delay(500);
}

void setup() {
    pinMode(LED, OUTPUT);
    blinkLed(250);
    pinMode(BUT1, INPUT_PULLUP);
    pinMode(BUT2, INPUT_PULLUP);
}

void blinkLed(int len){
    digitalWrite(LED,HIGH);
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(len/2);
    digitalWrite(LED,LOW);
    DigiKeyboard.delay(len/2);
}

void checkButton(){
    if (digitalRead(BUT1)==LOW) {
      pressed = 1;
      blinkLed(250);
    } else if (digitalRead(BUT2) == LOW){
      pressed = 2;
      blinkLed(125);
      blinkLed(125);
    } else {
      pressed = 0;
    }

    if (pressed > 0) {
      if (pressed == pin[pos]){
        pos++;
        if (pos > pinlen) {
          shuttingDown = 1;
          pressed = 0;
          consoleSwitch();
          shutoff();
        }
        DigiKeyboard.delay(500);  
      } else {
        pressed = 0;
        if (pos > pinlen) {
          pos = 0;
          shuttingDown = 0;
          blinkLed(250);
          blinkLed(250);
          blinkLed(250);
        }
      }
    }
}
  void loop() {

    if (shuttingDown == 1){
      blinkLed(2000);
    } else {
      checkButton();
    }
  }
