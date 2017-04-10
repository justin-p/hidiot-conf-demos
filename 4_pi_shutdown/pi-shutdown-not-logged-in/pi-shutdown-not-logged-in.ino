  #include "DigiKeyboard.h"
  #define LED 1

void login(){
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(250);
    DigiKeyboard.write(PSTR("pi"));
    DigiKeyboard.sendKeyStroke(KEY_ENTER);  
    DigiKeyboard.delay(250);
    DigiKeyboard.write(PSTR("raspberry"));
    DigiKeyboard.sendKeyStroke(KEY_ENTER);  
}

void shutoff(){
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(250);
    DigiKeyboard.write(PSTR(" sudo shutdown -h now"));
    DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

  void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    login();
    shutoff();
  }
  
  
  void loop() {
    digitalWrite(0,HIGH);
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(1000);
    digitalWrite(0,LOW);
    DigiKeyboard.delay(1000);
  }
