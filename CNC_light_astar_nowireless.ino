
#include <Arduino.h>
#include "printf.h"
#include "Button.h"
#include "Relay.h"
#include <PololuLedStrip.h>
#include "RGBColor.h"


// pin assignments
#define PIN_LEDS 0
#define PIN_CNC_PWR_SENSE 1
#define PIN_SPIN_SENSE 2

#define PIN_SPIN_ENABLE 3
#define PIN_SPIN_ON_OFF 4
#define PIN_SPIN_INDICATOR 5

#define PIN_DUST_ENABLE 6

#define PIN_VAC_RELAY 7
#define PIN_SPIN_RELAY 8
#define PIN_GND1 9


// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<PIN_LEDS> ledStrip;
#define LED_COUNT 2
rgb_color colors[LED_COUNT];
RGBColor color(SendNewColor);

// there are two relay outputs, one for spindle power, and other for a dust vac
Relay spin_relay(PIN_SPIN_RELAY, RelayActuated);
Relay vac_relay(PIN_VAC_RELAY, RelayActuated);

// internal state
boolean cnc_power_is_on = false;
boolean spindle_is_on = false;
unsigned long light_notify_time = 0;
boolean notify_color_is_on = false;

bool debug = false;


void setup()
{
  // Start up the serial port, for communication with the PC.
  Serial.begin(115200);
  printf_begin();
  
  pinMode(PIN_CNC_PWR_SENSE, INPUT);
  pinMode(PIN_SPIN_SENSE, INPUT);
  pinMode(PIN_GND1, OUTPUT);
  digitalWrite(PIN_GND1, false);
  
  // set up some timeouts for the relays - so they are not left on forever
  spin_relay.delayed_off_s = 1;
  spin_relay.timeout_s = 12*60*60;   // 12 hours
  vac_relay.delayed_off_s = 2;
  vac_relay.timeout_s = 12*60*60;  // 12 hours
  
  // go to default color on LEDs
  showNoColor();
}


void loop()
{
  unsigned long currentTime = millis();
  
  spin_relay.Update();
  vac_relay.Update();
  color.Update();
  
  // handle serial commands coming to us from the USB port
  if (Serial.available())
  {
    char c = Serial.read();
    if ( c == '?' ) // status request
    {
      printf("cnc_power:%d\r\n",cnc_power_is_on);
    }
  }
  
  // handle setting the color of the LED lights.
  //  if the CNC power turns off, turn lights off.
  //  otherwise, use color to indicate some spindle power status
  boolean cncPowerActuallyOn = digitalRead(PIN_CNC_PWR_SENSE);
  boolean spindleActuallyOn = digitalRead(PIN_SPIN_SENSE);
  
  if ( ! cncPowerActuallyOn )
  {
    if ( cnc_power_is_on )
    {
      cnc_power_is_on = cncPowerActuallyOn;
      showNoColor();
      printf("cnc_power:%d\r\n",cnc_power_is_on);
    }
  }
  else
  {
    if ( ! cnc_power_is_on )
    {
      cnc_power_is_on = cncPowerActuallyOn;
      showNormalColor();
      printf("cnc_power:%d\r\n",cnc_power_is_on);
    }
    
    // further, set colors depending on state
    // check if spindle is going on/off
    if ( !spindle_is_on && spindleActuallyOn )
    {
      spindle_is_on = spindleActuallyOn;
      showWarningColor();
      spin_relay.Set(spindle_is_on);
      vac_relay.Set(spindle_is_on);
      if ( debug ) Serial.println("@:Spindle ON");
      printf("cnc_spin:%d\r\n",spindle_is_on);
    }
    else if ( spindle_is_on && !spindleActuallyOn )
    {
      spindle_is_on = spindleActuallyOn;
      showAllClearColor();
      spin_relay.Set(spindle_is_on);
      vac_relay.Set(spindle_is_on);
      if ( debug ) Serial.println("@:Spindle OFF");
      printf("cnc_spin:%d\r\n",spindle_is_on);
    }
    
    // turn off notify color if has been long enough
    if ( millis() - light_notify_time > 3000 && notify_color_is_on )
    {
      showNormalColor();
      Serial.println("@:going back to normal");
    }
  }
  
}


void RelayActuated(Relay *r)
{
}


void SendNewColor(RGBColor *color)
{
  rgb_color newColor = color->GetRGBColorStruct();
  for(uint16_t i = 0; i < LED_COUNT; i++)
      colors[i] = newColor;
  ledStrip.write(colors, LED_COUNT);  
  //printf("@:showing color: red=%d, green=%d, blue=%d\r\n",color.red, color.green, color.blue);
}


void showNoColor()
{
  color.GoDirect(0.0, 1.0, 1.0, 0.6, DEFAULT_DIM_DELAY);
  notify_color_is_on = false;
}

void showNormalColor()
{
  color.GoDirect(1.0, 1.0, 1.0, 0.6, DEFAULT_DIM_DELAY);
  notify_color_is_on = false;
}

void showWarningColor()
{
  color.GoDirect(1.0, 1.0, 0.0, 0.0, 1);
  notify_color_is_on = true;
  light_notify_time = millis();
}

void showAllClearColor()
{
  color.GoDirect(1.0, 0.0, 1.0, 0.0, 1);
  notify_color_is_on = true;
  light_notify_time = millis();
}

