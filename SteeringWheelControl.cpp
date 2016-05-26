/*
 * IRremote
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz> 
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#include "SteeringWheelControl.h"
#include "SteeringWheelControlInt.h"

// Provides ISR
#include <avr/interrupt.h>

volatile irparams_t irparams;

//constructor
swc_recv::swc_recv(int recvpin)
{
  lastRawValue = 0;
  repeats = 0;
  irparams.recvpin = recvpin;
  irparams.error = SWC_NO_ERROR;
  error = SWC_NO_ERROR;
}

// initialization
void swc_recv::init() 
{
  timestamp = millis();

  cli();
  // setup pulse clock timer interrupt
  //Prescale /8 (16M/8 = 0.5 microseconds per tick)
  // Therefore, the timer interval can range from 0.5 to 128 microseconds
  // depending on the reset value (255 to 0)
  TIMER_CONFIG_NORMAL();

  //Timer2 Overflow Interrupt Enable
  TIMER_ENABLE_INTR;

  TIMER_RESET;

  sei();  // enable interrupts

  // initialize state machine variables
  irparams.rcvstate = SWC_STATE_IDLE;
  irparams.rawlen = 0;

  // set pin modes
  pinMode(irparams.recvpin, INPUT);
}

// TIMERx interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts
ISR(TIMER_INTR_NAME)
{
  TIMER_RESET;

  uint8_t irdata = (uint8_t)digitalRead(irparams.recvpin);

  irparams.timer++; // One more 50us tick
  if (irparams.rawlen >= RAWBUF) 
  {
    irparams.error = SWC_ERR_BUFFER_OVERFLOW; // Buffer overflow
    irparams.rcvstate = SWC_STATE_STOP;
  }
  switch(irparams.rcvstate) 
  {
  case SWC_STATE_IDLE: // In the middle of a gap
    if (irdata == 1)
    {
      if (irparams.timer < GAP_TICKS) 
      {
        // Not big enough to be a gap.
        irparams.timer = 0;
      } 
      else 
      {
        // gap just ended, record duration and start recording transmission
        irparams.rawlen = 0;
        irparams.rawbuf[irparams.rawlen++] = irparams.timer; //timer is an unsigned int (U16)
        irparams.timer = 0;
        irparams.rcvstate = SWC_STATE_MARK;
      }
    }
    break;
  case SWC_STATE_MARK: // timing MARK
    if (irdata == 0)
    {   
      // MARK ended, record time
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = SWC_STATE_SPACE;
    }
    break;
  case SWC_STATE_SPACE: // timing SPACE
    if (irdata == 1)
    { 
      // SPACE just ended, record it
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = SWC_STATE_MARK;
    } 
    else 
    { 
      //if (irparams.timer > GAP_TICKS) 
      if (irparams.timer > 700) 
      {
        //Serial.print("Big space, indicates gap between codes....Rawlen==");
        //Serial.println(irparams.rawlen,DEC);
        irparams.rcvstate = SWC_STATE_STOP;
        // Don't reset timer; keep counting space width
      } 
    }
    break;
  case SWC_STATE_STOP: // waiting, measuring gap
    if (irdata == 1)
    { 
      irparams.timer = 0;  // reset gap timer
    }
    break;
  }
}

void swc_recv::resume() 
{
  irparams.rcvstate = SWC_STATE_IDLE;
  irparams.error = SWC_NO_ERROR;
  irparams.rawlen = 0;
  error = SWC_NO_ERROR;
}


// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int swc_recv::getStatus() 
{
  if (irparams.rcvstate != SWC_STATE_STOP) 
  {
    return SWC_NOT_READY;
  }
  if (irparams.error != SWC_NO_ERROR)
  {
    return irparams.error;
  }

  if (decodeHash() == SWC_HASHED) 
  {
    //Serial.print("Decoded deh hash!: ");
    //Serial.println(rawValue,HEX);
    lastTimestamp = timestamp;
    timestamp = millis();
    lastRawValue = rawValue;
    
    if (rawValue == lastRawValue)
    {
      repeats++;
    }
    else
    {
      repeats == 0;
    }

         if (rawValue == 0x16F8BA45) { value = SWC_CMD_VOLUP; }
    else if (rawValue == 0x8D877B8B) { value = SWC_CMD_VOLDN; }
    else if (rawValue == 0xD1068AA9) { value = SWC_CMD_MUTE;  }
    else if (rawValue == 0xB7237849) { value = SWC_CMD_PSTUP; }
    else if (rawValue == 0x1F67D00F) { value = SWC_CMD_PSTDN; }
    else if (rawValue == 0x16635CC5) { value = SWC_CMD_SOURCE;}
    else if (rawValue == 0xE395A06D) { value = SWC_CMD_TRKUP; }
    else if (rawValue == 0x781BCC6B) { value = SWC_CMD_TRKDN; }
    else if (rawValue == 0xCF8E4B2F) { value = SWC_CMD_POWER; }
    else if (rawValue == 0xDA354EEB) { value = SWC_CMD_PLAY;  }
    else if (rawValue == 0xDA82A3F3) { value = SWC_CMD_BAND;  }
    else
    {
      value = SWC_CMD_UNSUPPORTED;
      return SWC_ERR_UNSUPPORTED_CMD;
    }
    return SWC_DECODED;
  }

  //Serial.println("Attempting SWC decode");

  // Throw away and start over
  resume();
  return SWC_NOT_READY;
}

int MATCH(int measured, int desired) 
{
	return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}


/* -----------------------------------------------------------------------
 * decode - decode an arbitrary IR code.
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int swc_recv::compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } 
  else if (oldval < newval * .8) {
    return 2;
  } 
  else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
long swc_recv::decodeHash() 
{
  // Require at least 6 samples to prevent triggering on noise
  if (irparams.rawlen < 6) 
  {
    //Serial.print("Rawlen too short to decode hash: ");
    //Serial.println(irparams.rawlen,DEC);
    irparams.error = SWC_ERR_RAWLEN_TOOSHORT;
    return SWC_ERR_RAWLEN_TOOSHORT;
  }
  long hash = FNV_BASIS_32;
  for (int i = 1; i+2 < irparams.rawlen; i++) 
  {
    int value =  compare(irparams.rawbuf[i], irparams.rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  rawValue = hash;
  return SWC_HASHED;
}

unsigned long swc_recv::getIdleTime()
{
	return millis() - timestamp;
}

void swc_recv::printRawValue()
{
  for (int i=0;i < (irparams.rawlen);i++)
  {
    Serial.print(irparams.rawbuf[i],DEC);
    Serial.print(",");
  }
  Serial.print("\n");
}