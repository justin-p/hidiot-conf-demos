  #include "DigiKeyboard.h"
  #define LED 0

  void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(250);
    DigiKeyboard.write(PSTR(" sudo shutdown -h now"));
    DigiKeyboard.sendKeyStroke(KEY_ENTER);
  }
  
  
  void loop() {
    digitalWrite(0,HIGH);
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(1000);
    digitalWrite(0,LOW);
    DigiKeyboard.delay(1000);
  }
