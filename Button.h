#ifndef Button_h
#define Button_h

#include "Arduino.h"

class Button;
typedef void (*notify_func_ptr)(Button *);
enum button_type_e { button_type_toggle, button_type_switch };

// times are in ms
#define BUTTON_DEBOUNCE_TIME 25
#define BUTTON_PRESS_HOLDOFF 175

class Button
{
  public:
    Button(uint8_t pin, button_type_e type, uint8_t led_pin, notify_func_ptr handler_func);
    void Update();
    button_type_e Type();
    void Press(bool send_notification);
    
    int value;	// the underlying state/value of this button.  depends on the button type.
    int condition;	// current real (debounced) condition of the button: BTN_PRESSED or BTN_NOT_PRESSED
    int autoPressed;	// if true, then this button was virtually pressed
    int clockOffset;    // the offset from millis() to use as the timebase for the blinking of this button
    
  private:
	int _prev;		// previously read condition - for looking for changes
	unsigned long _last_bounce_time;	// time of last button change (bounce)
	unsigned long _last_condition_change_time;	// used to implement a holdoff after a button changes condition
	uint8_t _pin;		// pin to read button state on - HIGH is not pressed, LOW is pressed
	uint8_t _led_pin;	// pin for an LED associated with this button. 255=no LED
        button_type_e _type;		// what type of button - BTN_TOGGLE, BTN_SWITCH
        bool _button_blink_state;
        notify_func_ptr _handler;
};

#endif
