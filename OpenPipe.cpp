 // Please read OpenPipe.h for information about the liscence

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>       // I2C LIBRARY FOR MPR121
#include "mpr121.h"     // MPR121 register definitions
#include "OpenPipe.h"

OpenPipeClass OpenPipe;

OpenPipeClass::OpenPipeClass(){

}

void OpenPipeClass::config(void){

  Wire.begin();
  mpr121QuickConfig();

}

void OpenPipeClass::power(char vcc_pin, char gnd_pin){
  
  pinMode(gnd_pin, OUTPUT);
  pinMode(vcc_pin, OUTPUT);

  digitalWrite(gnd_pin, LOW);
  digitalWrite(vcc_pin, LOW);
  delay(500);
  digitalWrite(vcc_pin, HIGH);

}

void OpenPipeClass::setFingering(int fing){

  fingering=fing;
  fingering_table=fingerings[fing].table;
  drone_note=fingering_table[2];

}

// Reads MPR121 electrodes and sort them in openpipe board order
// Updates fingers,control and note variables
int  OpenPipeClass::readFingers(){

  char buffer[8];
  int i=0;
  unsigned int tmp;
  

  // READ MPR121
  //Serial.print("1");
  Wire.beginTransmission(0x5A);
  Wire.write((uint8_t)0);
  Wire.requestFrom(0x5A, 2);
  // TODO: UNBLOCK HERE
  while(Wire.available()){ 
    buffer[i] = Wire.read();
    i++;
    if (i>18) break;
  }
  Wire.endTransmission();
  //Serial.print("2");

  // SORT MPR121 ELECTRODES
  fingers=   ((buffer[0]&(1<<0))>>0) | 
    ((buffer[0]&(1<<1))>>0) |
    ((buffer[0]&(1<<2))>>0) |
    ((buffer[0]&(1<<4))>>1) |
    ((buffer[0]&(1<<7))>>3) |
    ((buffer[0]&(1<<6))>>1) |
    ((buffer[1]&(1<<2))<<4) |
    ((buffer[1]&(1<<1))<<6) |
    ((buffer[1]&(1<<3))<<5);

  // READ RIGHT THUMB CONTROL ELECTRODES
  control=(buffer[0]&(1<<5))>>5;
  control|=(buffer[0]&(1<<3))>>2;
  control|=(buffer[1]&(1<<0))<<2;

  note=fingersToNote();

  return fingers|control<<10;

}

char OpenPipeClass::isON(void){

  return control&(1);

}

// Search note in fingering table based on fingers position
// Returns MIDI note if found, 0xFF otherwise
unsigned char OpenPipeClass::fingersToNote(void){

  int i;
  int note=0;
  unsigned long tmp;
  unsigned long longfingers;
  int base;

  base=fingering_table[0];
  i=2;
  while(fingering_table[i]!=0xFFFFFFFF){
    tmp=fingering_table[i];
    // Seach fingering word (1 on MSB)
    if (tmp&(0x80000000)){
      // Clean MSB
      tmp&=~(0x80000000);  
      // Fingering and mask matches? 
      if ( (fingers&(tmp&0xFFFF))==(tmp>>16)){
        //Serial.print("POSITION FOUND");
        // Jump over following fingering positions (if they exist)
        while (fingering_table[i]&(0x80000000)){
          //Serial.print(".");
          i++;
        }
        // Read note
        note=(fingering_table[i]>>24) & 0xFF;
        //Serial.print("NOTE");
        //Serial.println(note, DEC);
        return base+note;
      }
    }
    i++;
  }
  // FINGERING NOT FOUND
  return 0xFF;
}

void OpenPipeClass::printFingers(void){

    for (int i=8; i>=0; i--){
      if (fingers&(1<<i)) Serial.print("*");
      else Serial.print("O");
    }
    Serial.print(" ");
    for (int i=2; i>=0; i--){
      if (control&(1<<i)) Serial.print("*");
      else Serial.print("O");
    }

}

// read one MPR121 register at 'address'
// return the register content
char OpenPipeClass::mpr121Read(unsigned char address)
{

  char data;

  Wire.beginTransmission(0x5A);  // begin communication with the MPR121 on I2C address 0x5A
  Wire.write(address);            // sets the register pointer
  Wire.requestFrom(0x5A, 1);     // request for the MPR121 to send you a single byte

  // check to see if we've received the byte over I2C
  if(1 <= Wire.available())
  {
    data = Wire.read();
  }

  Wire.endTransmission();        // ends communication

  return data;  // return the received data

}

// write one MPR121 register with 'data' at 'address'
void OpenPipeClass::mpr121Write(unsigned char address, unsigned char data)
{

  Wire.beginTransmission(0x5A);  // begin communication with the MPR121 on I2C address 0x5A 
  Wire.write(address);            // sets the register pointer
  Wire.write(data);               // sends data to be stored
  Wire.endTransmission();        // ends communication

}

// configure all MPR121 registers as described in AN3944
void OpenPipeClass::mpr121QuickConfig(void)
{
  
  // Section A
  // This group controls filtering when data is > baseline.
  mpr121Write(MHD_R, 0x01);
  mpr121Write(NHD_R, 0x01);
  mpr121Write(NCL_R, 0x00);
  mpr121Write(FDL_R, 0x00);

  // Section B
  // This group controls filtering when data is < baseline.
  mpr121Write(MHD_F, 0x01);
  mpr121Write(NHD_F, 0x01);
  mpr121Write(NCL_F, 0xFF);
  mpr121Write(FDL_F, 0x02);

  // Section C
  // This group sets touch and release thresholds for each electrode
  mpr121Write(ELE0_T, TOU_THRESH);
  mpr121Write(ELE0_R, REL_THRESH);
  mpr121Write(ELE1_T, TOU_THRESH);
  mpr121Write(ELE1_R, REL_THRESH);
  mpr121Write(ELE2_T, TOU_THRESH);
  mpr121Write(ELE2_R, REL_THRESH);
  mpr121Write(ELE3_T, TOU_THRESH);
  mpr121Write(ELE3_R, REL_THRESH);
  mpr121Write(ELE4_T, TOU_THRESH);
  mpr121Write(ELE4_R, REL_THRESH);
  mpr121Write(ELE5_T, TOU_THRESH);
  mpr121Write(ELE5_R, REL_THRESH);

  mpr121Write(ELE6_T, TOU_THRESH);
  mpr121Write(ELE6_R, REL_THRESH);
  mpr121Write(ELE7_T, TOU_THRESH);
  mpr121Write(ELE7_R, REL_THRESH);
  mpr121Write(ELE8_T, TOU_THRESH);
  mpr121Write(ELE8_R, REL_THRESH);
  mpr121Write(ELE9_T, TOU_THRESH);
  mpr121Write(ELE9_R, REL_THRESH);
  mpr121Write(ELE10_T, TOU_THRESH);
  mpr121Write(ELE10_R, REL_THRESH);
  mpr121Write(ELE11_T, TOU_THRESH);
  mpr121Write(ELE11_R, REL_THRESH);

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  //mpr121Write(FIL_CFG, 0x04);
  mpr121Write(FIL_CFG, 0x00);

  // Section E
  // Electrode Configuration
  // Enable 6 Electrodes and set to run mode
  // Set ELE_CFG to 0x00 to return to standby mode
  mpr121Write(ELE_CFG, 0x0C); // Enables all 12 Electrodes

  // Section F
  // Enable Auto Config and auto Reconfig @3.3V
  mpr121Write(ATO_CFG0, 0x0B);
  mpr121Write(ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256
  mpr121Write(ATO_CFGL, 0x83);  // LSL = 0.65*USL
  mpr121Write(ATO_CFGT, 0xB5);  // Target = 0.9*USL

}

