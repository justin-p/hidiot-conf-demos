#include <avr/pgmspace.h>

#define UNIT 250 // Lets make time units 250 milliseconds long
#define LED 1    // Our LED is PB1

const char message[] PROGMEM = "HITBAMS 2017 is awesome"; // .... .. - -... .- -- ...   ..--- ----- .---- --... 
int c;

void on(){        // Turn the LED on
  digitalWrite(LED, HIGH);
}

void off(){       // Turn the LED off
  digitalWrite(LED, LOW);
}

void send(char c)
{
    switch (c)
    {
        case '-':
            on();
            delay(UNIT*3);
            off();
            break;
        case '.':
            on();
            delay(UNIT);
            off();
            break;
    case ' ':
        delay(UNIT*4);
        break;
    }
    delay(UNIT);
}

void sendc(PGM_P s)
{
    int c;
    for (c=pgm_read_byte(s);c;c=pgm_read_byte(++s)) 
    {
        send(c);
    }
}

void sendChar(int c){
  switch(toupper(c)){
    case ' ': sendc(PSTR(" ")); break;
    case 'A': sendc(PSTR(".-")); break;
    case 'B': sendc(PSTR("-...")); break;
    case 'C': sendc(PSTR("-.-.")); break;
    case 'D': sendc(PSTR("-..")); break;
    case 'E': sendc(PSTR(".")); break;
    case 'F': sendc(PSTR("..-.")); break;
    case 'G': sendc(PSTR("--.")); break;
    case 'H': sendc(PSTR("....")); break;
    case 'I': sendc(PSTR("..")); break;
    case 'J': sendc(PSTR(".---")); break;
    case 'K': sendc(PSTR("-.-")); break;
    case 'L': sendc(PSTR(".-..")); break;
    case 'M': sendc(PSTR("--")); break;
    case 'N': sendc(PSTR("-.")); break;
    case 'O': sendc(PSTR("---")); break;
    case 'P': sendc(PSTR(".--.")); break;
    case 'Q': sendc(PSTR("--.-")); break;
    case 'R': sendc(PSTR(".-.")); break;
    case 'S': sendc(PSTR("...")); break;
    case 'T': sendc(PSTR("-")); break;
    case 'U': sendc(PSTR("..-")); break;
    case 'V': sendc(PSTR("...-")); break;
    case 'W': sendc(PSTR(".--")); break;
    case 'X': sendc(PSTR("-..-")); break;
    case 'Y': sendc(PSTR("-.--")); break;
    case 'Z': sendc(PSTR("--..")); break;
    case '1': sendc(PSTR(".----")); break;
    case '2': sendc(PSTR("..---")); break;
    case '3': sendc(PSTR("...--")); break;
    case '4': sendc(PSTR("....-")); break;
    case '5': sendc(PSTR(".....")); break;
    case '6': sendc(PSTR("-....")); break;
    case '7': sendc(PSTR("--...")); break;
    case '8': sendc(PSTR("---..")); break;
    case '9': sendc(PSTR("----.")); break;
    case '0': sendc(PSTR("-----")); break;
  }
  send(' ');
}

void setup() {                
  // initialize the digital pin as an output.
  pinMode(LED, OUTPUT); // Tells the HIDIOT we want to use the pin to output a signal
}

void loop() {
  // put your main code here, to run repeatedly:
  PGM_P ptr = message;
  for (c=pgm_read_byte(ptr); c; c=pgm_read_byte(++ptr)){
    sendChar(c);
  }
  delay(UNIT*21);
}
