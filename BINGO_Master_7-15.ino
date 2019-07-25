
// BINGO_Master_7-15-2019

//----------- Libraries and Global Definitions ------------

//The libraries for the keypad, display, Bluetooth, and I2C provide commands to make coding much easier, and to take care of all the little details. 

//For Keypad

#include <Keypad.h>

 const byte ROWS = 4;
 const byte COLS = 3;

  char keys[ROWS][COLS] = { //Defines the keypad matrix
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'} };

  byte rowPins[ROWS] = {9, 4, 5, 7}; //Defines the Nano digital pins that connect to keypad pins (4, 5, 6, 7)
  byte colPins[COLS] = {8, 10, 6};   //Defines the Nano digital pins that connect to keypad pins (1, 2, 3)
  //See Instructable Step 8 for more detail.

 Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); //The sketch scans Nano D4-D10 pins, finds a connected pair, 
                                                                           //and uses this mapping to determine which key in the matrix was pressed.  
//For Display:

#include <Adafruit_LEDBackpack.h>
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4(); //Class definition of display commands

//For Bluetooth:

#include "SoftwareSerial.h"
SoftwareSerial BTSerial(2, 3); //Rx,Tx, Bluetooth pin assignmentment {D2. D3)

//For I2C:

#include <Wire.h> //Needed by the display Backpack, which handles all the I2C communication


//----------------Setup------------------

void setup() {

    //For AlphaNumeric Display:

  alpha4.begin(0x70); //Begins I2C communication with the display module; 0X70 is the module address.
  BlankDisplay(); //My function that puts all bars in the display at power-up

  //For Bluetooth:

  BTSerial.begin(9600); //Begins Bluetooth serial communication.

  //For Nano:

  Serial.begin(9600); //When using the Arduino IDE serial monitor.

  /* Nano Pins: D0,D1 (Tx,Rx) : Hardware serial communication with the Arduino IDE serial monitor for debugging purposes.
                D2,D3 (Tx,Rx) : Software serial communication with Bluetooth module.
                D4-D10 : Matrix readout of keyboard.
                A4,A5 (SDA,SCL): Required pins for I2C communication with the display Backpack.
     None of these pins require a pinMode() specification because they either are default input or otherwise set by a library. */

}

//--------------MyFunctions---------------

void BlankDisplay() {

  int i; //Display digit number (0-3)
  char Character = '-';
  for (i = 0; i < 4; i++) {
    alpha4.writeDigitAscii(i, Character); //Sends the Character ASCII number to digits 0-3 registers
    alpha4.writeDisplay(); //Lights up the LED segments that comprise the commanded character
  } 
}


//----------------Loop---------------------

void loop() {

  int i; //Display's digit number (0-3)
  char key; //Result of keypad press
  char Key[4]; //Key array

StartKeypad:

  i = 0; //First digit

  do {  //Loop to get keypad input and display all four characters

    //Wait for key to be  pressed

    do {  //Loop to get pressed keypad/character
      key = keypad.getKey();
    }
    while (key == NO_KEY);

    //When first key is pressed, clear the display

    if (i == 0) {
      alpha4.clear();
      alpha4.writeDisplay();
    }

    //If a mistake is made, press "#" to clear the display and start over

    if (key == '#') { //Clear command
      alpha4.clear();
      alpha4.writeDisplay();
      BTSerial.write('#'); //Command sent to the Slave to clear its display
      BTSerial.write(' '); //Found to be necessary, otherwise require two key presses
      goto StartKeypad;
    }
    else Key[i] = key;

    //First keypad press gets the BINGO alphabetic character and the hyphen character (B- , N- , etc.)

    if (i == 0) {

      if (Key[0] == '1')  Key[0] = 'B';
      if (Key[0] == '4')  Key[0] = 'I';
      if (Key[0] == '7')  Key[0] = 'N';
      if (Key[0] == '*')  Key[0] = 'G';
      if (Key[0] == '0')  Key[0] = 'O';
      alpha4.writeDigitAscii(0, Key[0]); //Load first digit register (0) with the determined alphabetic ASCII number

      i = 1;
      Key[1] = '-'; //Second character left "blank"
      alpha4.writeDigitAscii(1, Key[1]); //Send "blank" to second digit register

      alpha4.writeDisplay(); //Light-up the first two characters

    }

    //Then light-up the third and fourth digits with the second and third key press number (01 - 75)

    if (i > 1) {
      alpha4.writeDigitAscii(i, Key[i]);
      alpha4.writeDisplay();
    }

    i = i + 1; //

  } //Loop back to get and display next digit's number

  while (i < 4); //Loop to display last two characters


  // Transmit all four characters to the Slave using Bluetooth

  for (i = 0; i < 5; i++) {  //Note: Not i < 4; required the extra null transmission to let the Slave get the last character
    BTSerial.write(Key[i]); }

} //End main loop
