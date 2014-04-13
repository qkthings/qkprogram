/*!
 * \file hal_timer.c
 *
 * \author mribeiro
 *  
 * This file is part of QkProgram
 */

#include "../sys/qk_system.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define _TOP      16
#define _PRE      1024
#define _PRE_BITS ((1 << CS12) | (1 << CS10))

//-----------------------------------------------------------------------------
void hal_timer_init()
{
  OCR1A = _TOP * 1000; // ~1s
  TCCR1B |= (1 << WGM12);  // (mode 4)
  TIMSK1 |= (1 << OCIE1A); // Set compare match to TIMER1_COMPA_vect
}

void hal_timer_reinit()
{

}
void hal_timer_start(hal_timer_id_t tmr)
{
  if(tmr == HAL_TIMER_ID_2)
  {
    TCCR1B |= _PRE_BITS;
  }
}
void hal_timer_stop(hal_timer_id_t tmr)
{
  if(tmr == HAL_TIMER_ID_2)
  {
    TCCR1B &= ~((1 << CS12) | (1 << CS11)  | (1 << CS10));
  }
}
void hal_timer_reset(hal_timer_id_t tmr)
{
  if(tmr == HAL_TIMER_ID_2)
  {
    TCNT1 = 0;
  }
}
void hal_timer_setPeriod(hal_timer_id_t tmr, uint32_t per, hal_timer_scale_t s)
{
  if(tmr == HAL_TIMER_ID_2)
  {
    OCR1A = _TOP * per;   //1/fosc * pre * top = 1ms
    TCCR1B |= (1 << WGM12);  // (mode 4)
    TIMSK1 |= (1 << OCIE1A); // Set compare match to TIMER1_COMPA_vect
  }
}
void hal_timer_setFrequency(hal_timer_id_t tmr, uint32_t freq)
{
  if(freq == 0)
    return;

  if(tmr == HAL_TIMER_ID_2)
    hal_timer_setPeriod(HAL_TIMER_ID_2, 1000.0/(float)freq, HAL_TIMER_SCALE_MSEC);
}
void hal_timer_setCallback(hal_timer_id_t tmr, void (*c)(void))
{
  _hal_timer[tmr].callbacks.timeout = c;
}

//-----------------------------------------------------------------------------
void delay_setBusyWaiting(bool en)
{
  // not implemented: always busy waiting
}

void delay_ms(uint16_t value)
{
  _delay_ms(value);
}

void delay_us(uint16_t value)
{
  _delay_us(value);
}

//-----------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
  if(_hal_timer_2->flags.timeout == 1)
    _hal_timer_2->flags.overlap = 1;
  _hal_timer_2->flags.timeout = 1;

//  if(_hal_timer_2->callbacks.timeout != 0){
//    //_hal_timer_2->callbacks.timeout(); //FIXME not working (segfault?): shouldn't reach this!
//  }
}