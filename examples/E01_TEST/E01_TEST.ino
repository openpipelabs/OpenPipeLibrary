/*  OpenPipe Breakout test
 *
 *  Reads OpenPipe fingers and outputs the values to the serial port
 *  Connect the OpenPipe Breakout wires to Arduino as follows:
 *  YELLOW-> A5 (SCL)
 *  GREEN-> A4 (SDA)
 *  BLACK -> A3 (GND) (Grey in old openpipes)
 *  RED -> A2 (VCC) (White in old openpipes)
 *
 *  © OpenPipe Labs. 2016
 *  www.openpipe.cc
 *
 *  This example code is in the public domain.
 */

#include <Wire.h> // Wire library for comunicating with OpenPipe
#include <OpenPipe.h> // OpenPipe Library

// global variables
unsigned long fingers, previous_fingers;

void setup(){

  // Serial port setup
  Serial.begin(115200);
  Serial.println("OpenPipe TEST");
  
  // OpenPipe setup
  OpenPipe.power(A2, A3); // VCC PIN in A2 and GND PIN in A3
  OpenPipe.config();
  OpenPipe.setFingering (FINGERING_GAITA_GALEGA);
  
  // variables initialization
  fingers=0;
  previous_fingers=0xFF;

}

void loop(){

  // Read OpenPipe fingers
  fingers=OpenPipe.readFingers();

  // If fingers changed print the current fingering
  if (fingers!=previous_fingers){
    previous_fingers=fingers;
    OpenPipe.printFingers();
    Serial.println();
  }
}
