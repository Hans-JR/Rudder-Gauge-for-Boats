# Rudder-Gauge-for-Boats
Based on ESP32 and ILI9341 TFT Display 320x240

This cocde is based on an example of the TFT_eSPI library.
A homemade actuator made of a gear set which turnes a 180 Ohm potentiometer 
provides the input signal to GPIO 35 (Analog ADC1_CH7)
The resistor value ranges from ~5 to 180 Ohm and is measured with a 1K resistor in row and translated to percent.
The ADC value in the scetch has to be calibrated accordingly.

The original gear set is designed by https://hackaday.io/project/168592-opencpn-chart-plotter-w-autopilot-and-waypoints
