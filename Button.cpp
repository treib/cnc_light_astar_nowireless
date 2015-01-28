#include "Arduino.h"
#include "Button.h"


Button::Button(uint8_t pin, button_type_e type, uint8_t led_pin, notify_func_ptr handler_func)
{
  _pin = pin;
  if ( _pin >= 0 )
    pinMode(_pin, INPUT_PULLUP);
    
  _led_pin = led_pin;
  if ( _led_pin >= 0 )
  {
    pinMode(_led_pin, OUTPUT);
    digitalWrite(_led_pin, LOW);
  }
  
  _type = type;
  
  value = 0;
  condition = 0;
  autoPressed = 0;
  
  _prev = 0;
  _last_bounce_time = 0;
  _last_condition_change_time = 0;
  _button_blink_state = false;
  
  _handler = handler_func;
}

void Button::Update()
{
  unsigned long currentTime = millis();
  int b_now = ! digitalRead(_pin);  // adjusted so 0=off/not pressed, 1=on/pressed
  
  if ( b_now != _prev )
    _last_bounce_time = currentTime;
  if ( b_now != condition 
      && (currentTime - _last_bounce_time) > BUTTON_DEBOUNCE_TIME 
      && (currentTime - _last_condition_change_time) > BUTTON_PRESS_HOLDOFF )
  {
    condition = b_now;
    _last_condition_change_time = currentTime;
    
    if ( _type == button_type_toggle && condition == 1 )
    {
      Press(true);
    }
    else if ( _type == button_type_switch )
    {
      value = (condition == 1);
      if ( _handler )
        (*_handler)(this);
    }
  }
  _prev = b_now;
  
  // set LED to reflect state
  if ( _led_pin >= 0 )
  {
    if ( ! autoPressed )
      digitalWrite(_led_pin,(value == 0 ? LOW : HIGH));
    else
      analogWrite(_led_pin,(_button_blink_state ? 255 : 32));
  }
  
  // run blinker
  _button_blink_state = (((currentTime+clockOffset) % 1000) > 499 );
  
  // prevent loop-around bugs with time values
  if ( (currentTime - _last_condition_change_time) > BUTTON_PRESS_HOLDOFF )
    _last_condition_change_time = currentTime - BUTTON_PRESS_HOLDOFF - 1;
  if ( (currentTime - _last_bounce_time) > BUTTON_DEBOUNCE_TIME )
    _last_bounce_time = currentTime - BUTTON_DEBOUNCE_TIME - 1;
}

button_type_e Button::Type()
{
  return _type;
}

void Button::Press(bool send_notification)
{
  value = ! value;
  autoPressed = 0;
  if ( send_notification && _handler )
    (*_handler)(this);
}



