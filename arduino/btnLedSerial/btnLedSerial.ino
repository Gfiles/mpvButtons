/*
  String to Integer conversion

  Reads a serial input string until it sees a newline, then converts the string
  to a number if the characters are digits.

  The circuit:
  - No external components needed.

  created 29 Nov 2010
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/StringToInt
*/

String inString = "";  // string to hold input

// how many Buttons with LED
#define numBtns 2

// Variables will change:
int btnState[numBtns];
int lastBtnState[numBtns];
int btnPin[numBtns] = {2, 10};
int ledPin[numBtns] = {3, 11};
int gndPin[numBtns*2] = {4, 5, 9, 12};

int reading[numBtns];
int pinNum;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[numBtns];  // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(9600);
  // default settings
  
  //Setup Buttons and LEDS
  for (int i = 0; i < numBtns; i++) {
    pinMode(btnPin[i], INPUT_PULLUP);
    pinMode(ledPin[i], OUTPUT);
    digitalWrite(ledPin[i], HIGH);
    lastBtnState[i] = HIGH;
  }
  
  for (int i = 0; i < numBtns*2; i++) {
    pinMode(gndPin[i], OUTPUT);
    digitalWrite(gndPin[i], LOW);
  }
}

void loop() {
  
  // read the state of the switch into a local variable:
  for (int i = 0; i < numBtns; i++) {
    reading[i] = digitalRead(btnPin[i]);
    //Serial.println(reading[i]);
  // check to see if you just pressed the button
  // (i.e. the input went from HIGH to LOW), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  
    if (reading[i] != lastBtnState[i]) {
      // reset the debouncing timer
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:
  
      // if the button state has changed:
      if (reading[i] != btnState[i]) {
        btnState[i] = reading[i];
  
        // only toggle the LED if the new button state is HIGH
        if (btnState[i] == LOW) {
          Serial.println(i);
          if (digitalRead(ledPin[i]) == 1){
            digitalWrite(ledPin[i], 0);
          }
        }
      }
    }

    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastBtnState[i] = reading[i];
  }  
  if (Serial.available() > 0) {
    int inChar = Serial.read();
       //wait for data available
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    // if you get a newline, print the string, then the string's value:
    if (inChar == '\n'){
      if (inString.length() == 2) {
        //Serial.print("Value:");
        //Serial.println(inString.toInt());
        String p1 = "";
        String p2 = "";
        p1 += inString[0];
        p2 += inString[1];
        
        Serial.print("String: ");
        Serial.print(ledPin[p1.toInt()]);
        Serial.print(" - ");
        Serial.println(p2.toInt());
        // clear the string for new input:
        inString = "";
        digitalWrite(ledPin[p1.toInt()], p2.toInt());
      }
      else {Serial.print("Unkown command");}
    /*
    if (teststr.length() == 2) {
      digitalWrite(ledPin[toInt(teststr[0])], toInt(teststr[1]));
      Serial.print(ledPin[toInt(teststr[0])]);
      Serial.print(" - ");
      Serial.println(toInt(teststr[1]));
    } else {
    }
    */
  }
  }
}
