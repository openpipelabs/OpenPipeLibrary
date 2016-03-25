/*  OpenPipe Breakout samples example
 *
 *  Play sampled sounds (44100 Hz @ 8bit) using PWM output.
 *  Samples are defined in samples.h file, generated using the provided samples.py script
 *
 *  Connect the OpenPipe Breakout wires to Arduino as follows:
 *  YELLOW-> A5 (SCL)
 *  GREEN-> A4 (SDA)
 *  BLACK -> A3 (GND) (Grey in old openpipes)
 *  RED -> A2 (VCC) (White in old openpipes)
 *
 *  Connect a speaker to PINs 11 & 12
 *
 *  © OpenPipe Labs. 2016
 *  www.openpipe.cc
 *
 *  This example code is in the public domain.
 */

#include <Wire.h> // Wire library for comunicating with OpenPipe
#include <OpenPipe.h> // OpenPipe Library
#include "samples.h"

// SELECT HERE WICH INSTRUMENT TO USE
#define GAITA_GALEGA
//#define GAITA_ASTURIANA
//#define GHB
//#define UILLEANN
//#define SACKPIPA

// DISABLE DRONE COMMENTING THE FOLLOWING LINE
//#define ENABLE_DRONE

// THE FOLLOWING LINES ASSOCAITES FINGERINGS AND SOUND SAMPLES FOR EVERY INSTRUMENT
#ifdef GAITA_GALEGA
  #define FINGERING FINGERING_GAITA_GALEGA
  #define INSTRUMENT INSTRUMENT_GAITA_GALEGA
#endif

#ifdef GAITA_ASTURIANA
  #define FINGERING FINGERING_GAITA_ASTURIANA
  #define INSTRUMENT INSTRUMENT_GAITA_ASTURIANA
#endif

#ifdef GHB
  #define FINGERING FINGERING_GREAT_HIGHLAND_BAGPIPE  
  #define INSTRUMENT INSTRUMENT_GHB
#endif

#ifdef UILLEANN
  #define FINGERING FINGERING_UILLEANN_PIPE
  #define INSTRUMENT INSTRUMENT_UILLEANN
#endif

#ifdef SACKPIPA
  #define FINGERING FINGERING_SACKPIPA
  #define INSTRUMENT INSTRUMENT_SACKPIPA
#endif

// There is also INSTRUMENT_SINUSOIDS available with notes defined from 48 to 90


#define SAMPLE_RATE 44100

// Global variables
unsigned long fingers, previous_fingers;
char note;
char playing;
unsigned char previous_sample, sample, drone_sample;
unsigned char sample_index, drone_index;
unsigned long int drone_sample_length;
sample_t* samples_table;


void setup(){

  // Serial port setup
  Serial.begin(115200);
  Serial.println("OpenPipe SAMPLES");
  
  // OpenPipe setup
  OpenPipe.power(A2, A3); // VCC PIN in A2 and GND PIN in A3
  OpenPipe.config();
  OpenPipe.setFingering(FINGERING);

  // Speaker setup
  pinMode(12,OUTPUT);
  digitalWrite(12, LOW); //SPEAKER GND

  // Configure PWM for sound generation
  startPlayback();

  
  // Variables initialization
  fingers=0;
  previous_fingers=0xFF;
  playing=0;

  // Initialize samples table for defined instrument
  samples_table=INSTRUMENT;
#ifdef ENABLE_DRONE
  drone_sample=note_to_sample(OpenPipe.drone_note);
  drone_sample_length=samples_table[drone_sample].len;
#endif

}

void loop(){
  
  // Read OpenPipe fingers
  fingers=OpenPipe.readFingers();

  // If fingers have changed...
  if (fingers!=previous_fingers){
    previous_fingers=fingers;
    
    // Print fingers for debug
    OpenPipe.printFingers();
    
    // Check the low right thumb sensor
    if (OpenPipe.isON()){
      playing=1;
    
      //Find which sample to play
      note=OpenPipe.note;
      sample=note_to_sample(note);
      Serial.print(" NOTE: ");
      Serial.print(note, DEC);
      Serial.print(" SAMPLE: ");
      Serial.print(sample);
      Serial.println();
    }else{
      sample=0xFF;
      Serial.println(" SILENCE");
    }      
  }
}

// Search sound sample index based on MIDI note
// Return sample index if found, 0xFF otherwise
int note_to_sample(int note){
  int i=0;
  while(samples_table[i].note!=0xFF){
    if (samples_table[i].note==note){
      return i;
    }
    i++;
  }
  return 0xFF;
}


///////////////////////////////////////////////////////////////
// PWM AUDIO FUNCTIONS
///////////////////////////////////////////////////////////////


// configure PWM for sound generation
void startPlayback()
{
  pinMode(11, OUTPUT);

  // Set up Timer 2 to do pulse width modulation on the speaker
  // pin.

  // Use internal clock (datasheet p.160)
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));

  // Set fast PWM mode  (p.157)
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);

  // Do non-inverting PWM on pin OC2A (p.155)
  // On the Arduino this is pin 11.
  TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
  TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));

  // No prescaler (p.158)
  TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

  // Set initial pulse width to the first sample.
  OCR2A = 127;

  // Set up Timer 1 to send a sample every interrupt.
  cli();

  // Set CTC mode (Clear Timer on Compare Match) (p.133)
  // Have to set OCR1A *after*, otherwise it gets reset to 0!
  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

  // No prescaler (p.134)
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

  // Set the compare register (OCR1A).
  // OCR1A is a 16-bit register, so we have to do this with
  // interrupts disabled to be safe.
  OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000

  // Enable interrupt when TCNT1 == OCR1A (p.136)
  TIMSK1 |= _BV(OCIE1A);

  sei();
}

// stop PWM sound
void stopPlayback()
{
  // Disable playback per-sample interrupt.
  TIMSK1 &= ~_BV(OCIE1A);

  // Disable the per-sample timer completely.
  TCCR1B &= ~_BV(CS10);

  // Disable the PWM timer.
  TCCR2B &= ~_BV(CS10);

  digitalWrite(11, LOW);
}

/* PWM AUDIO CODE : This is called at SAMPLE_RATE Hz to load the next sample. */
ISR(TIMER1_COMPA_vect) {
  
  // STOP SOUND
  if (!(OpenPipe.isON())){
    OCR2A=127;
    return;
  }
  
  // PLAY PREVIOUS SAMPLE IF THE CURRENT ONE IS NOT FOUND
  if (sample==0xFF){
    //OCR2A=0;
    //return;
    sample=previous_sample;
  }
  
  // WAIT FOR THE SAMPLE TO FINISH IN ORDER TO AVOID 'CLICKS'
  if (previous_sample!=sample && sample_index==0){
    previous_sample=sample;
    //sample_index=0;
  }
  
  // LOOP SAMPLE
  if (sample_index==samples_table[previous_sample].len){
    sample_index=0;
  }else{
    sample_index++;
  }
  
#ifdef ENABLE_DRONE

  // LOOP DRONE SAMPLE
  if (drone_index==drone_sample_length){
    drone_index=0;
  }else{
    drone_index++;
  }
  
  // MIX NOTE AND DRONE SAMPLES
  int16_t out;
  out=0;
  out=pgm_read_byte_near(((uint8_t*)samples_table[drone_sample].sample) + drone_index)*2;
  out+=pgm_read_byte_near(((uint8_t*)samples_table[previous_sample].sample) + sample_index)*8;
  out=out>>4;
  OCR2A=out;
#else
  // UPDATE PWM
  OCR2A=pgm_read_byte_near(((uint8_t*)samples_table[previous_sample].sample) + sample_index);
#endif

}

