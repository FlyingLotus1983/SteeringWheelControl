/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#ifndef SteeringWheelControlInt_h
#define SteeringWheelControlInt_h

#include <Arduino.h>

// define which timer to use
//
// Uncomment the timer you wish to use on your board.  If you
// are using another library which uses timer2, you have options
// to switch IRremote to use a different timer.

// Arduino Mega
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  //#define IR_USE_TIMER1   // tx = pin 11
  //#define IR_USE_TIMER2     // tx = pin 9
  //#define IR_USE_TIMER3   // tx = pin 5
  //#define IR_USE_TIMER4   // tx = pin 6
  #define IR_USE_TIMER5   // tx = pin 46

// Arduino Duemilanove, Diecimila, LilyPad, Mini, Fio, etc
#else
  //#define IR_USE_TIMER1   // tx = pin 9
  #define IR_USE_TIMER2     // tx = pin 3
#endif

#ifdef F_CPU
#define SYSCLOCK F_CPU     // main Arduino clock
#else
#define SYSCLOCK 16000000  // main Arduino clock
#endif

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


//--------------------------------------------------------------------------------
// defines for timer2 (8 bits)
#if defined(IR_USE_TIMER2)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR2A |= _BV(COM2B1))
#define TIMER_DISABLE_PWM    (TCCR2A &= ~(_BV(COM2B1)))
#define TIMER_ENABLE_INTR    (TIMSK2 = _BV(OCIE2A))
#define TIMER_DISABLE_INTR   (TIMSK2 = 0)
#define TIMER_INTR_NAME      TIMER2_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR2A = _BV(WGM20); \
  TCCR2B = _BV(WGM22) | _BV(CS20); \
  OCR2A = pwmval; \
  OCR2B = pwmval / 3; \
})
#define TIMER_COUNT_TOP      (SYSCLOCK * USECPERTICK / 1000000)
#if (TIMER_COUNT_TOP < 256)
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR2A = _BV(WGM21); \
  TCCR2B = _BV(CS20); \
  OCR2A = TIMER_COUNT_TOP; \
  TCNT2 = 0; \
})
#else
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR2A = _BV(WGM21); \
  TCCR2B = _BV(CS21); \
  OCR2A = TIMER_COUNT_TOP / 8; \
  TCNT2 = 0; \
})
#endif

//--------------------------------------------------------------------------------
// defines for timer1 (16 bits)
#elif defined(IR_USE_TIMER1)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR1A |= _BV(COM1A1))
#define TIMER_DISABLE_PWM    (TCCR1A &= ~(_BV(COM1A1)))
#if defined(__AVR_ATmega8P__) || defined(__AVR_ATmega8__)
  #define TIMER_ENABLE_INTR    (TIMSK = _BV(OCIE1A))
  #define TIMER_DISABLE_INTR   (TIMSK = 0)
#else
  #define TIMER_ENABLE_INTR    (TIMSK1 = _BV(OCIE1A))
  #define TIMER_DISABLE_INTR   (TIMSK1 = 0)
#endif
#define TIMER_INTR_NAME      TIMER1_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR1A = _BV(WGM11); \
  TCCR1B = _BV(WGM13) | _BV(CS10); \
  ICR1 = pwmval; \
  OCR1A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR1A = 0; \
  TCCR1B = _BV(WGM12) | _BV(CS10); \
  OCR1A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT1 = 0; \
})


//--------------------------------------------------------------------------------
// defines for timer3 (16 bits)
#elif defined(IR_USE_TIMER3)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR3A |= _BV(COM3A1))
#define TIMER_DISABLE_PWM    (TCCR3A &= ~(_BV(COM3A1)))
#define TIMER_ENABLE_INTR    (TIMSK3 = _BV(OCIE3A))
#define TIMER_DISABLE_INTR   (TIMSK3 = 0)
#define TIMER_INTR_NAME      TIMER3_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR3A = _BV(WGM31); \
  TCCR3B = _BV(WGM33) | _BV(CS30); \
  ICR3 = pwmval; \
  OCR3A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR3A = 0; \
  TCCR3B = _BV(WGM32) | _BV(CS30); \
  OCR3A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT3 = 0; \
})


//--------------------------------------------------------------------------------
// defines for timer4 (10 bits, high speed option)
#elif defined(IR_USE_TIMER4_HS)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM    (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR    (TIMSK4 = _BV(TOIE4))
#define TIMER_DISABLE_INTR   (TIMSK4 = 0)
#define TIMER_INTR_NAME      TIMER4_OVF_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR4A = (1<<PWM4A); \
  TCCR4B = _BV(CS40); \
  TCCR4C = 0; \
  TCCR4D = (1<<WGM40); \
  TCCR4E = 0; \
  TC4H = pwmval >> 8; \
  OCR4C = pwmval; \
  TC4H = (pwmval / 3) >> 8; \
  OCR4A = (pwmval / 3) & 255; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR4A = 0; \
  TCCR4B = _BV(CS40); \
  TCCR4C = 0; \
  TCCR4D = 0; \
  TCCR4E = 0; \
  TC4H = (SYSCLOCK * USECPERTICK / 1000000) >> 8; \
  OCR4C = (SYSCLOCK * USECPERTICK / 1000000) & 255; \
  TC4H = 0; \
  TCNT4 = 0; \
})


//--------------------------------------------------------------------------------
// defines for timer4 (16 bits)
#elif defined(IR_USE_TIMER4)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM    (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR    (TIMSK4 = _BV(OCIE4A))
#define TIMER_DISABLE_INTR   (TIMSK4 = 0)
#define TIMER_INTR_NAME      TIMER4_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR4A = _BV(WGM41); \
  TCCR4B = _BV(WGM43) | _BV(CS40); \
  ICR4 = pwmval; \
  OCR4A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR4A = 0; \
  TCCR4B = _BV(WGM42) | _BV(CS40); \
  OCR4A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT4 = 0; \
})


//--------------------------------------------------------------------------------
// defines for timer5 (16 bits)
#elif defined(IR_USE_TIMER5)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR5A |= _BV(COM5A1))
#define TIMER_DISABLE_PWM    (TCCR5A &= ~(_BV(COM5A1)))
#define TIMER_ENABLE_INTR    (TIMSK5 = _BV(OCIE5A))
#define TIMER_DISABLE_INTR   (TIMSK5 = 0)
#define TIMER_INTR_NAME      TIMER5_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR5A = _BV(WGM51); \
  TCCR5B = _BV(WGM53) | _BV(CS50); \
  ICR5 = pwmval; \
  OCR5A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR5A = 0; \
  TCCR5B = _BV(WGM52) | _BV(CS50); \
  OCR5A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT5 = 0; \
})


#else // unknown timer
#error "Internal code configuration error, no known IR_USE_TIMER# defined\n"
#endif

#endif
