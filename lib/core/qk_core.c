/*!
 * \file qk_core.c
 *
 * \author mribeiro
 *  
 * This file is part of QkProgram
 */

#include "qk_system.h"

qk_core _qk_core;
/******************************************************************************/
static void handle_board_detection();
static void handle_input_changed();
/******************************************************************************/

void qk_core_init()
{
  memset(&_qk_core, 0, sizeof(qk_core));
  _qk_core.currentState = QK_STATE_IDLE;
  _qk_core.info.baudRate = _HAL_UART_BAUD_DEFAULT_LOW;
#if defined( QK_IS_DEVICE )
  _qk_core.sampling.frequency = 0; // invalid
  _qk_core.sampling.mode = QK_SAMPLING_MODE_CONTINUOUS;
  _qk_core.sampling.triggerClock = QK_SAMPLING_TRIGGER_CLOCK_10SEC;
  _qk_core.sampling.triggerScaler = 1;
  _qk_core.sampling.N = 10;

  qk_sampling_set_frequency(_QK_DEFAULT_SAMPFREQ);
#endif

#if defined( _QK_FEAT_EEPROM_ )
  qk_restore();
#endif

  handle_board_detection();
  handle_input_changed();
}

bool qk_clock_set_mode(qk_clock_mode mode)
{
  bool changed = false;
  switch(mode)
  {
  case QK_CLOCK_MODE_NORMAL:
    changed = hal_clock_setFrequency(HAL_CLOCK_FREQ_NORMAL);
    break;
  case QK_CLOCK_MODE_FASTER:
    changed = hal_clock_setFrequency(HAL_CLOCK_FREQ_FASTER);
    break;
  case QK_CLOCK_MODE_FAST:
    changed = hal_clock_setFrequency(HAL_CLOCK_FREQ_FAST);
    break;
  case QK_CLOCK_MODE_SLOW:
    changed = hal_clock_setFrequency(HAL_CLOCK_FREQ_SLOW);
    break;
  case QK_CLOCK_MODE_SLOWER:
    changed = hal_clock_setFrequency(HAL_CLOCK_FREQ_SLOWER);
    break;
  default: ;
  }
  if(changed) {
    _qk_core.clockMode = mode;
  }
  return changed;
}

void qk_run()
{
  uint32_t i, count;
  /*************************************
   * PROCESSING
   ************************************/

  if(_hal_gpio.flags.inputChanged == 1)
  {
    handle_input_changed();
    _hal_gpio.flags.inputChanged = 0;
  }

  //TODO Status notifications
  //TODO Events

  for(i = 0; i < QK_PROTOCOL_STRUCT_COUNT; i++)
  {
    if(_qk_protocol[i].callback.processBytes != 0)
      _qk_protocol[i].callback.processBytes();

    if(_qk_protocol[i].flags.reg & QK_PROTOCOL_FLAGMASK_NEWPACKET)
    {
      if(_qk_protocol[i].callback.processPacket != 0)
        _qk_protocol[i].callback.processPacket();

      _qk_protocol[i].flags.reg &= ~QK_PROTOCOL_FLAGMASK_NEWPACKET;
    }
  }

#if defined( QK_IS_DEVICE )
  count = _QK_MAX_FIRED_EVENTS;
  qk_cb *pendingEvents = qk_pendingEvents();
  while(qk_cb_available(pendingEvents) > 0 && count--)
  {
    qk_event firedEvent;
    qk_cb_read(pendingEvents, (void*) &firedEvent);
    //qk_event *firedEvent = (qk_event*)qk_cb_pick(pendingEvents);
    _qk_protocol_send_event(&firedEvent, _protocol_board);
  }
#endif

  _qk_handle_state_change();

  /*************************************
   * STATE MACHINE
   ************************************/
  switch(_qk_core.currentState)
  {
  case QK_STATE_SLEEP:
    break;
  case QK_STATE_IDLE:
    hal_timer_stop(HAL_TIMER_ID_2);
    break;
#if defined(QK_IS_DEVICE)
  case QK_STATE_START:
    if(_qk_device->callbacks.start != 0)
      _qk_device->callbacks.start();
    hal_timer_reset(HAL_TIMER_ID_2);
    hal_timer_start(HAL_TIMER_ID_2);
    _qk_core.currentState = QK_STATE_RUNNING;
    break;
  case QK_STATE_RUNNING:
    if(_hal_timer_2->flags.timeout == 1)
    {
      if(_qk_device->callbacks.sample != 0)
        _qk_device->callbacks.sample();
      _qk_protocol_send_code(QK_PACKET_CODE_DATA, _protocol_board);
      _hal_timer_2->flags.timeout = 0;
    }
    break;
  case QK_STATE_STANDBY:
    break;
  case QK_STATE_STOP:
    if(_qk_device->callbacks.stop != 0)
      _qk_device->callbacks.stop();
    _qk_core.currentState = QK_STATE_IDLE;
    break;
#endif
  default:
    _qk_core.currentState = QK_STATE_IDLE;
  }

  /*************************************
   * POWER MANAGEMENT
   ************************************/
#ifdef _QK_FEAT_POWER_MANAGEMENT
  hal_power_EM1();
#endif
}

void qk_loop()
{
  while(1) { qk_run(); }
}

void _qk_request_state_change(qk_state state)
{
  _qk_core.change_to_state = state;
  _qk_core.flags.reg_internal |= QK_FLAGMASK_INTERNAL_RQSTATECHANGE;
}
void _qk_handle_state_change()
{
  if(_qk_core.flags.reg_internal & QK_FLAGMASK_INTERNAL_RQSTATECHANGE)
  {
    _qk_core.currentState = _qk_core.change_to_state;
    _qk_core.flags.reg_internal &= ~QK_FLAGMASK_INTERNAL_RQSTATECHANGE;
  }
}

void qk_core_set_baudrate(uint32_t baud)
{
  hal_uart_setBaudRate(HAL_UART_ID_1, baud);
  _qk_core.info.baudRate = baud;
}

#ifdef QK_IS_DEVICE
void qk_sampling_set_mode(qk_sampling_mode mode)
{
  _qk_core.sampling.mode = mode;
}

void qk_sampling_set_trigger(qk_sampling_trigger_clock clock, uint8_t scaler)
{
  _qk_core.sampling.triggerClock = clock;
  _qk_core.sampling.triggerScaler = scaler;
}

void qk_sampling_set_N(uint32_t N)
{
  _qk_core.sampling.N = N;
}

void qk_sampling_set_frequency(uint32_t sampFreq)
{
  if(sampFreq == 0 || sampFreq == _qk_core.sampling.frequency)
  {
    return;
  }
  hal_timer_setFrequency(HAL_TIMER_ID_2, sampFreq);
  _qk_core.sampling.frequency = sampFreq;
  _qk_core.sampling.period = (uint32_t)(1000000.0/(float)sampFreq); // usec
}
void qk_sampling_set_period(uint32_t usec)
{
  uint32_t msec;
  if(usec >= 1000) {
    msec = usec*1000;
    hal_timer_setPeriod(HAL_TIMER_ID_2, msec, HAL_TIMER_SCALE_MSEC);
  }
  else {
    hal_timer_setPeriod(HAL_TIMER_ID_2, usec, HAL_TIMER_SCALE_USEC);
  }
  _qk_core.sampling.period = usec;
  _qk_core.sampling.frequency = (uint32_t)(1000000.0/(float)usec);

}

#endif /*QK_IS_DEVICE*/


static void handle_board_detection()
{
  bool detected = !hal_getDET(); // DET pin is pulled-up

  if(flag(_qk_core.flags.reg_status, QK_FLAGMASK_STATUS_DET) == detected) {
    return;
  }

  if(detected)
  {
    hal_blinkLED(2, 50);
    _qk_core.flags.reg_status |= QK_FLAGMASK_STATUS_DET;

    hal_uart_setBaudRate(HAL_UART_ID_1, _qk_core.info.baudRate);

    if(_qk_core.callbacks.boardAttached != 0) {
      _qk_core.callbacks.boardAttached();
    }

#if defined( QK_IS_DEVICE )
//start
#endif
  }
  else
  {
    hal_blinkLED(1, 50);
    _qk_core.flags.reg_status &= ~QK_FLAGMASK_STATUS_DET;

    hal_uart_setBaudRate(HAL_UART_ID_1, _HAL_UART_BAUD_DEFAULT_LOW);

    if(_qk_core.callbacks.boardRemoved != 0)
      _qk_core.callbacks.boardRemoved();

#if defined( QK_IS_DEVICE )
//stop
#endif
  }
}

static void handle_input_changed()
{
  handle_board_detection();

#if defined( QK_IS_MODULE )
  uint8_t count;
  bool buttonPressed = !_getPB(); // PB pin is pulled-up

  if(buttonPressed) {
    delay_ms(250);
    if(_getPB() == 1) { // quick
      //if(_qk.flags.reg & QK_FLAGMASK_DET) {
        //TODO startDevice
      //}
      //else {
        //TODO stopDevice
      //}
    }
    else { // hold
      for(count = 0; count < QK_GOTOPD_TIMEOUT; count++) {
        delay_ms(100);
        if(_getPB()) break;
      }
      if(count == QK_GOTOPD_TIMEOUT) {
        _blinkLED(1, 100);
        while(_getPB() == 0) {
          delay_ms(100);
        }
        //TODO goto power down (deep sleep)
      }
    }
  }
#endif /*QK_IS_MODULE*/

  if(_qk_core.callbacks.inputChanged != 0) {
    _qk_core.callbacks.inputChanged();
  }
}