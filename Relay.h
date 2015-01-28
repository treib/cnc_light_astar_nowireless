#ifndef Relay_h
#define Relay_h

#include "Arduino.h"

class Relay;
typedef void (*relay_notify_func_ptr)(Relay *);

class Relay
{
  public:
    Relay(uint8_t pin, relay_notify_func_ptr handler_func);  // handler is called any time the relay actually (physically) changes state
    void Update();
    int Actual();  // return the actual value at this moment
    void Set(bool);
    unsigned long timeout_s;    // operational timeout - turn off this relay after this many seconds of being on (max is 24 hours)
    unsigned long delayed_off_s;    // delayed turn off - when relay is requested to turn off, will actually continue to run for the specified seconds first (max is 24 hours)

  private:
    uint8_t _pin;
    unsigned long _last_relay_actuate_time;
    unsigned long _last_user_changed_time;
    bool _desired;
    bool _actual;
    relay_notify_func_ptr _handler;
};

#endif
