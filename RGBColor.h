#ifndef RGBColor_h
#define RGBColor_h

#include "Arduino.h"
#include <PololuLedStrip.h>

// some light defaults
#define MINIMUM_INTENSITY 8.0/255.0
#define DEFAULT_DIM_DELAY 3
#define OFF_RAMP_SPEED 9

class RGBColor;
typedef void (*rgbcolor_notify_func_ptr)(RGBColor *);


class RGBColor
{
  public:
    // the handler is called when the color changes to a new value (so it can be sent to the lamp if needed)
    RGBColor(rgbcolor_notify_func_ptr handler_func);
    
    void Update();
    
    // for ramp_speed: 0=immediate, 255=default, or 1-254 with larger being slower
    void GoDirect(float intensity, float c_red, float c_green, float c_blue, int ramp_speed);
    
    // get the current (instantaneous) color values, ready to be loaded into a PWM or similar
    rgb_color GetRGBColorStruct();
    
  private:
    rgbcolor_notify_func_ptr _handler;
    
    // current desired color/intensity
    float _desired_red;  // 0-1, amount of color
    float _desired_green;  // 0-1, amount of color
    float _desired_blue;  // 0-1, amount of color
    float _desired_intensity; // 0-1, amount of brightness
    boolean _dimming_in_progress;  // true when we are doing a smooth transition
    
    // the transitional values - due to dimming, this is what we are actually displaying right now
    float _actual_red;  // 0-1, amount of color
    float _actual_green;  // 0-1, amount of color
    float _actual_blue;  // 0-1, amount of color
    float _actual_intensity; // 0-1, amount of brightness
    
    // what is actually loaded into PWM
    uint8_t _current_red;
    uint8_t _current_green;
    uint8_t _current_blue;
    
    // used for dimming
    unsigned long _dimDelay;    // the throttles how fast fading happens while holding down button
    unsigned long _updateBulb_last_dimTime;
    
    void convergeFloatToTarget(float *current, float desired, float speed);
};

#endif

