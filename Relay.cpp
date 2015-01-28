#include "Arduino.h"
#include "Relay.h"


Relay::Relay(uint8_t pin, relay_notify_func_ptr handler_func)
{
  _pin = pin;
  if ( _pin >= 0 )
    pinMode(_pin, OUTPUT);
    
  _desired = false;
  _actual = false;
  _last_relay_actuate_time = 0;
  _last_user_changed_time = 0;
  timeout_s = 0;  // no timeout
  delayed_off_s = 0; // no delayed off
  
  _handler = handler_func;
}

void Relay::Update()
{
  unsigned long currentTime = millis();
  
  if ( _desired != _actual && (currentTime - _last_relay_actuate_time) >= 500 )
  {
    if ( delayed_off_s <= 0 || !_actual || ( _actual && (currentTime - _last_user_changed_time) >= delayed_off_s*1000 ) )
    {
      digitalWrite(_pin, (_desired==0 ? LOW : HIGH));
      _actual = _desired;
      _last_relay_actuate_time = currentTime;
      if ( _handler ) (*_handler)(this);
    }
  }
  
  if ( _actual && timeout_s > 0 && (currentTime - _last_relay_actuate_time) >= timeout_s*1000 )
  {
    // relay has been on long enough to reach the timeout.  turn it off, and notify.
    _desired = false;
    if ( _handler ) (*_handler)(this);
  }
  
  // peg to max of 24 hours so we never have strange time compares when the current time wraps around
  unsigned long oneDay = (unsigned long)1000*60*60*24;
  if ( (currentTime - _last_relay_actuate_time) > oneDay ) _last_relay_actuate_time = currentTime - oneDay - 1;
  if ( (currentTime - _last_user_changed_time) > oneDay ) _last_user_changed_time = currentTime - oneDay - 1;
}

void Relay::Set(bool val)
{
  if ( _desired != val )
  {
    _last_user_changed_time = millis();
    _desired = val;
  }
}

int Relay::Actual()
{
  return _actual;
}

