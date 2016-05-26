/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 * Edited by Mitra to add new controller SANYO
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#ifndef SteeringWheelControl_h
#define SteeringWheelControl_h

#include <Arduino.h>

const int SWC_NO_ERROR            =    0;
const int SWC_NOT_READY           =    1;
const int SWC_DECODED             =    2;
const int SWC_HASHED              =    3;
const int SWC_CMD_UNSUPPORTED     =  100;
const int SWC_CMD_VOLUP           =  101;
const int SWC_CMD_VOLDN           =  102;
const int SWC_CMD_MUTE            =  103;
const int SWC_CMD_PSTUP           =  104;
const int SWC_CMD_PSTDN           =  105;
const int SWC_CMD_SOURCE          =  106;
const int SWC_CMD_TRKUP           =  107;
const int SWC_CMD_TRKDN           =  108;
const int SWC_CMD_POWER           =  109;
const int SWC_CMD_PLAY            =  110;
const int SWC_CMD_BAND            =  111;
const int SWC_STATE_IDLE          =  202;
const int SWC_STATE_MARK          =  203;
const int SWC_STATE_SPACE         =  204;
const int SWC_STATE_STOP          =  205;
const int SWC_ERR_UNSPECIFIED     = 5000;
const int SWC_ERR_BUFFER_OVERFLOW = 5001;
const int SWC_ERR_UNSUPPORTED_CMD = 5002;
const int SWC_ERR_RAWLEN_TOOSHORT = 5003;

//// Some useful constants
#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF 71 // Length of raw duration buffer

//-----------------------------------------------------------------------------
// information for the interrupt handler
typedef struct {
  uint8_t  recvpin;           // pin for IR data from detector
  uint8_t  rcvstate;          // state machine
  unsigned int timer;     // state timer, counts 50uS ticks.
  unsigned int rawbuf[RAWBUF]; // raw data
  uint8_t  rawlen;         // counter of entries in rawbuf
  int      error;
} irparams_t;

//-----------------------------------------------------------------------------
// main class for receiving IR
class swc_recv
{
public:
                swc_recv(int recvpin);
  void          init();
  int           getStatus();
  void          resume();
  unsigned long getIdleTime(); //milliseconds of idle time
  byte          value;         // Decoded value (enum values SWC_xxxx)
  unsigned long repeats;       //# of time current value has been repeated
  int           error;
private:
  long          decodeHash();
  int           compare(unsigned int oldval, unsigned int newval);
  void          printRawValue();
  unsigned long rawValue;      //the raw value received (hash of code)
  unsigned long lastRawValue;  //the last raw value received
	unsigned long timestamp;     //timestamp (millis() value) of current decode
	unsigned long lastTimestamp; //timestamp (millis() value) of last decode
};


// receiver states

#define TOLERANCE 25  // percent tolerance in measurements
#define LTOL (1.0 - TOLERANCE/100.) 
#define UTOL (1.0 + TOLERANCE/100.) 

#define _GAP 40000 // Minimum gap between transmissions (41 ms on alpine protocol)
#define GAP_TICKS (_GAP/USECPERTICK) //400

#define TICKS_LOW(us) (int) (((us)*LTOL/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*UTOL/USECPERTICK + 1))

#endif
