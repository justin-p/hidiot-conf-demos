// DigiDuck code courtesy of uslurper. https://github.com/uslurper
// This program currently does not allow the BREAK key, since I couldn't find it in the USB HID spec.

#include "DigiKeyboard.h"

#define B1 0
#define B2 2
#define LED 1

#define KEY_TAB 43
#define KEY_DOWN 81
#define KEY_DELETE 42
#define KEY_PRINTSCREEN 70
#define KEY_SCROLLLOCK 71
#define KEY_INSERT 73
#define KEY_PAUSE 72
#define KEY_HOME 74
#define KEY_PAGEUP 75
#define KEY_END 77
#define KEY_PAGEDOWN 78
#define KEY_RIGHTARROW 79
#define KEY_RIGHT 79
#define KEY_DOWNARROW 81
#define KEY_LEFTARROW 80
#define KEY_UP 82
#define KEY_UPARROW 82
#define KEY_NUMLOCK 83
#define KEY_CAPSLOCK 57
#define KEY_MENU 118

int triggered = 0;

void payload() {

  // Author: Cody Theodore
  // Title: OSX Youtube Blast
  // This payload will open terminal, crank up the Macs volume all the way, then open a youtube video of
  // your choice by replacing the link.
  DigiKeyboard.delay(1000);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_SPACE, MOD_GUI_LEFT);
  DigiKeyboard.println("terminal");
  DigiKeyboard.delay(500);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(4000);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.println("osascript -e 'set volume 7'");
  DigiKeyboard.delay(500);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(500);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.println("open https://www.youtube.com/watch?v=dQw4w9WgXcQ");
  DigiKeyboard.delay(500);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

void checkButton(){
    if (digitalRead(B1)==LOW || digitalRead(B2) == LOW){
      triggered = 1;
    }
}

void setup(){
  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {
   if (triggered == 1){
      digitalWrite(LED,HIGH);
      DigiKeyboard.update();
      DigiKeyboard.sendKeyStroke(0);
      payload();
      triggered = 0;
      digitalWrite(LED, LOW);
    } else {
      checkButton();
    }
}
