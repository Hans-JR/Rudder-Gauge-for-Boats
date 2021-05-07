# Rudder-Gauge-for-Boats
Based on ESP32 and ILI9341 TFT Display 320x240

This cocde is based on an example of the TFT_eSPI library.
A homemade actuator made of a gear set which turnes a 180 Ohm potentiometer 
provides the input signal to GPIO 35 (Analog ADC1_CH7)
The resistor value ranges from ~5 to 180 Ohm and is measured with a 1K resistor in series and translated to percent.
The ADC value in the sketch has to be calibrated accordingly.

The original gear set is designed by https://hackaday.io/project/168592-opencpn-chart-plotter-w-autopilot-and-waypoints

![IMG_20210506_133548](https://user-images.githubusercontent.com/37930673/117303739-554a2680-ae7d-11eb-8504-1f1abc6dc235.jpg)
