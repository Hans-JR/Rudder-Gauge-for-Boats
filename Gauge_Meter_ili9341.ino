/*
 Based on TFT_eSPI example analogue meter using a ILI9341 TFT LCD screen (320 x 240)

 Needs Font 2 and 4

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.
 
 This version is only for ESP32!!!

 Connection Display:
 Vin  > 5V
 GND  > GND
 CS   > 5
 RST  > 4
 DC   > 17 (TX2)
 MOSI > 23
 SCK  > 18
 LED  > 3.3V
 MISO > 19

 Rudder input: 35 

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 
Updated by Bodmer for variable meter size
Modified by Hans-JR. 
 */

// Define meter size as 1 for tft.rotation(0) or 1.3333 for tft.rotation(1)
#define M_SIZE 1.3333

#define ADC_Calibration_Value1 250.0 // For resistor measure 5 Volt and 180 Ohm equals 100% plus 1K resistor.
// A 3.3V Zener Diode may be connected to input pin (Cathode to pin) and a 1k resistor from +5V to pin

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#define TFT_GREY 0x5AEB

float ltx = 0;                                  // Saved x coord of bottom of needle
uint16_t osx = M_SIZE*120, osy = M_SIZE*120;    // Saved x & y coords
uint32_t updateTime = 0;                        // time for next update

int old_analog =  -999;                         // Value last displayed

int value[6] = {0, 0, 0, 0, 0, 0};
int old_value[6] = { -1, -1, -1, -1, -1, -1};
int d = 0;


float potValue = 0;

// Rudder position measure is connected to GPIO 35 (Analog ADC1_CH7)
const int ADCpin2 = 35;

void setup(void) {
  tft.init();
  tft.setRotation(1);
  Serial.begin(57600);        // For debug
  tft.fillScreen(TFT_BLACK);

  analogMeter();              // Draw analogue meter

  updateTime = millis();      // Next update time
}


// ReadVoltage is used to improve the linearity of the ESP32 ADC see: https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function

double ReadVoltage(byte pin) {
  double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if (reading < 1 || reading > 4095) return 0;
  return (-0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089) * 1000;
} 

void loop() {


  potValue = ((potValue * 15) + (ReadVoltage(ADCpin2) * ADC_Calibration_Value1 / 4096)) / 16; // This implements a low pass filter to eliminate spike for ADC readings

  Serial.print("Final Value: ");
  Serial.println(potValue);            // filtered value 
  if (potValue>100) potValue=100;
  
  plotNeedle(potValue, 0);
}


// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter()
{

  // Meter outline

  tft.fillRect(0, 0, M_SIZE*239, M_SIZE*180, TFT_BLUE); // outher frame
  tft.fillRect(10, 10, M_SIZE*225, M_SIZE*165, TFT_WHITE); // background panel

  tft.drawLine(120, 163, 200, 163, TFT_BLUE); // line above needle  

  tft.setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (M_SIZE*100 + tl) + M_SIZE*120;
    uint16_t y0 = sy * (M_SIZE*100 + tl) + M_SIZE*140;
    uint16_t x1 = sx * M_SIZE*100 + M_SIZE*120;
    uint16_t y1 = sy * M_SIZE*100 + M_SIZE*140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (M_SIZE*100 + tl) + M_SIZE*120;
    int y2 = sy2 * (M_SIZE*100 + tl) + M_SIZE*140;
    int x3 = sx2 * M_SIZE*100 + M_SIZE*120;
    int y3 = sy2 * M_SIZE*100 + M_SIZE*140;

    // Green zone limits
    if (i >= -50 && i < 0) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    // Red zone limits
    if (i >= 0 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_RED);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_RED);
    }

    // Short scale tick length
    if (i % 25 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (M_SIZE*100 + tl) + M_SIZE*120;
    y0 = sy * (M_SIZE*100 + tl) + M_SIZE*140;
    x1 = sx * M_SIZE*100 + M_SIZE*120;
    y1 = sy * M_SIZE*100 + M_SIZE*140;

    // Draw tick
    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (M_SIZE*100 + tl + 10) + M_SIZE*120;
      y0 = sy * (M_SIZE*100 + tl + 10) + M_SIZE*140;
      tft.setRotation(3);
      switch (i / 25) {
        case -2: tft.drawCentreString("40", x0, y0 + 75, 2); break;
        case -1: tft.drawCentreString("20", x0, y0 + 150, 2); break;
        case 0: tft.drawCentreString("CTR", x0, y0 + 180, 2); break;
        case 1: tft.drawCentreString("20", x0, y0 + 150, 2); break;
        case 2: tft.drawCentreString("40", x0, y0 + 75, 2); break;
      }
      tft.setRotation(1);
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);   
    x0 = sx * M_SIZE*100 + M_SIZE*120;
    y0 = sy * M_SIZE*100 + M_SIZE*140;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  tft.setTextColor(TFT_DARKGREEN);  // Text colour
  tft.setRotation(3);
  tft.drawString("STBD", M_SIZE*(5 + 230 - 40), M_SIZE*(100 - 20), 2); // Label right
  tft.setTextColor(TFT_RED);  // Text colour  
  tft.drawString("PORT", M_SIZE*(5 + 60 - 40), M_SIZE*(100 - 20), 2);  //Label left
  tft.setTextColor(TFT_BLACK);  // Text colour
  tft.drawCentreString("Rudder", M_SIZE*120, M_SIZE*30, 4); // Label centre
  tft.drawRect(0, 0, 320, 240, TFT_WHITE);  // Draw bezel line
  tft.setRotation(1);

  plotNeedle(0, 0); // Put meter needle at 0
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(int value, byte ms_delay)
{

  if (value < -10) value = -10; // Limit value to emulate needle end stops
  if (value > 100) value = 100;

  // Move the needle until new value reached
  while (!(value == old_analog)) {
    if (old_analog < value) old_analog++;
    else old_analog--;

    if (ms_delay == 0) old_analog = value; // Update immediately if delay is 0

    float sdeg = map(old_analog, -10, 110, -150, -30); // Map value to angle
    // Calcualte tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(M_SIZE*(120 + 20 * ltx - 1), M_SIZE*(140 - 20), osx - 1, osy, TFT_WHITE);
    tft.drawLine(M_SIZE*(120 + 20 * ltx), M_SIZE*(140 - 20), osx, osy, TFT_WHITE);
    tft.drawLine(M_SIZE*(120 + 20 * ltx + 1), M_SIZE*(140 - 20), osx + 1, osy, TFT_WHITE);

    // Re-plot text under needle
    tft.setTextColor(TFT_BLACK);
//    tft.drawCentreString("Rudder", M_SIZE*120, M_SIZE*30, 4); // only required, if label interferres with needle

    // Store new needle end coords for next erase
    ltx = tx;
    osx = M_SIZE*(sx * 98 + 120);
    osy = M_SIZE*(sy * 98 + 140);

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(M_SIZE*(120 + 20 * ltx - 1), M_SIZE*(140 - 20), osx - 1, osy, TFT_RED);
    tft.drawLine(M_SIZE*(120 + 20 * ltx), M_SIZE*(140 - 20), osx, osy, TFT_MAGENTA);
    tft.drawLine(M_SIZE*(120 + 20 * ltx + 1), M_SIZE*(140 - 20), osx + 1, osy, TFT_RED);

    // Slow needle down slightly as it approaches new postion
    if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;

    // Wait before next update
    delay(ms_delay);
  }
}
