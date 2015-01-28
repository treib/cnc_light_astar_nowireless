#include "Arduino.h"
#include "RGBColor.h"

RGBColor::RGBColor(rgbcolor_notify_func_ptr handler_func)
{
  _handler = handler_func;
  
  _desired_intensity = 0.0; // off
  
  _actual_red = _actual_green = _actual_blue = 0.0;
  _actual_intensity = 0.0;

  _current_red = _current_green = _current_blue = 0;
  _dimming_in_progress = false;
  _dimDelay = DEFAULT_DIM_DELAY;
  _updateBulb_last_dimTime = 0;
}


// for ramp_speed: 0=immediate, 255=default, or 1-254 with larger being slower
void RGBColor::GoDirect(float intensity, float c_red, float c_green, float c_blue, int ramp_speed)
{
  _desired_intensity = intensity;
  _desired_red = c_red;
  _desired_green = c_green;
  _desired_blue = c_blue;
  _dimming_in_progress = (ramp_speed != 0);
  if ( ramp_speed == 255 )
    _dimDelay = DEFAULT_DIM_DELAY;
  else
    _dimDelay = ramp_speed;
  Update();
}


void RGBColor::Update()
{
  // see if it's time to update
  if ( (millis() - _updateBulb_last_dimTime) > _dimDelay )
  {
    _updateBulb_last_dimTime = millis();
    
    // first, converge actual lamp values to the desired values
    _actual_red = _desired_red;
    _actual_green = _desired_green;
    _actual_blue = _desired_blue;
  
    if ( _dimming_in_progress )
      convergeFloatToTarget(&_actual_intensity, _desired_intensity, 1.0/255.0);
    else
      _actual_intensity = _desired_intensity;
  
    // enforce a lower limit for non-zero intensities
    if ( _desired_intensity > 0 && _actual_intensity > 0 && _actual_intensity < MINIMUM_INTENSITY )
      _actual_intensity = MINIMUM_INTENSITY;
  
    // now load the actual values into the PWM
    boolean didUpdate = false;
  
    // calculate new color values
    int red = _actual_intensity * _actual_red * 255;
    if (red < 0) red = 0;
    if (red > 255) red = 255;
    int green = _actual_intensity * _actual_green * 255;
    if (green < 0) green = 0;
    if (green > 255) green = 255;
    int blue = _actual_intensity * _actual_blue * 255;
    if (blue < 0) blue = 0;
    if (blue > 255) blue = 255;
  
    if (red != _current_red || green != _current_green || blue != _current_blue)
    {
      didUpdate = true;
    }
    _current_red = red;
    _current_green = green;
    _current_blue = blue;
  
    if (didUpdate && _handler)
    {
      (*_handler)(this);
    }
  }
}


rgb_color RGBColor::GetRGBColorStruct()
{
  rgb_color color;
  color.red = _current_red;
  color.green = _current_green;
  color.blue = _current_blue;
  return color;
}


void RGBColor::convergeFloatToTarget(float *current, float desired, float speed)
{
  float currentVal = *current;
  if ( currentVal < desired )
  {
    currentVal += speed;
    if ( currentVal > desired ) currentVal = desired;
  }
  else if ( currentVal > desired )
  {
    currentVal -= speed;
    if ( currentVal < desired ) currentVal = desired;
  }
  *current = currentVal;
}



