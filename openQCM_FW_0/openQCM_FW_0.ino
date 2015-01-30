/* LICENSE
 * Copyright (C) 2014 openQCM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * INTRO
 * openQCM is the unique opensource quartz crystal microbalance http://openqcm.com/
 * openQCM Java software project is available on github repository 
 * https://github.com/marcomauro/openQCM
 * 
 * Measure QCM frequency using FreqCount library developed by Paul Stoffregen 
 * https://github.com/PaulStoffregen/FreqCount
 *
 * NOTE       - designed for 10 Mhz At-cut quartz crystal
 *            - 3.3 VDC supply voltage quartz crystal oscillator
 *            - Configure EXTERNAL reference voltage used for analog input
 *
 * author     Marco Mauro 
 * version    0 (basic)
 * date       october 2014 
 *
 */
 
 #include <FreqCount.h>
 
 // fixed "gate interval" time for counting cycles 1000ms  
 #define GATE   1000
 // fixed Nyquist–Shannon sampling frequency
 #define ALIAS  8000000
 
 // print data to serial port 
 void dataPrint(unsigned long Count, int Temperature){
  Serial.print("RAWMONITOR");
  Serial.print(Count);
  Serial.print("_");
  Serial.print(Temperature);
  Serial.write(255);
}

// map value to a double
double map_value 
(int x, int in_min, int in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// measure temperature
int getTemperature(void){
  // TODO calibration
  const int calib = - 18;
  // resistor in voltage divider circuit
  const int R_ref = 100;
  // voltage reference in voltage divider circuit
  const double V_ref = 3.3;
  double volt = map_value(analogRead(1), 0, 1023, 0, V_ref );
  double alpha = volt/V_ref;
  double resistance = (alpha/(1 - alpha))*R_ref;
  // PT100 linear relation between resistance and temperature
  // t(°C) = 2,596*R(ohm) - 259,8
  // Resistance range (100, 140) OHM 
  int Temperature = int ( 2.596 * resistance - 259.8 );
  // TODO PT100 calibration  
  Temperature = Temperature + calib;
  return(Temperature);
}

// variable declaration
// QCM frequency
unsigned long frequency = 0;
// counting the number of pulses in a fixed time 
unsigned long count = 0;
int temperature = 0;

void setup(){
  Serial.begin(115200);
  // Configure the reference voltage used for analog input 
  analogReference(EXTERNAL);
  FreqCount.begin(GATE);
}

void loop(){
  if (FreqCount.available()) 
  {
    count = FreqCount.read();           // counting the number of pulses
    frequency = (2 * ALIAS) - count;    // measure QCM frequency
    temperature = getTemperature();     // measure temperature 
    dataPrint(frequency, temperature);  // print data
  }
}
