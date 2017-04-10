#include <DigisparkOLED.h>
#include <Wire.h>
// ============================================================================

#include "augen.h"


void setup() {
  // put your setup code here, to run once:

  oled.begin();

}

void loop() {
  
  // put your main code here, to run repeatedly:
  oled.fill(0xFF); //fill screen with color
  delay(1000);
  oled.clear(); //all black
  delay(3000);
  //usage: oled.setCursor(X IN PIXELS, Y IN ROWS OF 8 PIXELS STARTING WITH 0);
  oled.setCursor(36, 0); //top left
  oled.setFont(FONT8X16);
  oled.print(F("RAW HEX")); //wrap strings in F() to save RAM!
  delay(1000);
  oled.setFont(FONT6X8);
  oled.setCursor(0, 3); //two rows down because the 8x16 font takes two rows of 8
  oled.println(F("  HIDIOTS ON OLED!")); //println will move the cursor 8 or 16 pixels down (based on the front) and back to X=0
  oled.println(F(""));
  delay(1000);
  oled.print(F("Shall we try a pic?")); //lines auto wrap
  delay(5000);
  oled.fill(0xFF); //fill screen with color
  delay(1000);
  oled.clear(); //all black
  delay(1000);
  //usage oled.bitmap(START X IN PIXELS, START Y IN ROWS OF 8 PIXELS, END X IN PIXELS, END Y IN ROWS OF 8 PIXELS, IMAGE ARRAY);
  oled.bitmap(0, 0, 128, 8, Augen);
  delay(10000);
}
