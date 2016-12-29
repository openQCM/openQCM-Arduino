/* LICENSE
 * openQCM ia free software licensed under GNU GPL v3.0 General Public License 2016
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
 * openQCM is the unique open source quartz crystal microbalance http://openqcm.com/
 * openQCM Java software project is available on github repository 
 * https://github.com/marcomauro/openQCM
 * 
 * Measure QCM frequency using FreqCount library developed by Paul Stoffregen 
 * https://github.com/PaulStoffregen/FreqCount
 *
 * NOTE       - designed for arduino micro board
 *            - designed for 6 and 10 Mhz At-cut quartz crystal
 *            - 3.3 VDC supply voltage quartz crystal oscillator
 *            - Configure EXTERNAL reference voltage used for analog input
 *            - Thermistor temperature sensor
 *            - read the openQCM serial number in EEPROM
 *
 * author     Marco Mauro 
 * version    1.2
 * date       May 2015 
 *
 */

// include library for frequency counting
#include <FreqCount.h>
// include EERPOM library
#include <EEPROM.h>

// fixed "gate interval" time for counting cycles 1000ms  
#define GATE   1000
// Thermistor pin
#define THERMISTORPIN A1 
// // resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average 
#define NUMSAMPLES 10
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000  
// current address in EEPROM series
#define ADDRESS_SERIES 0
// current address in EEPROM first number 
#define ADDRESS_NUMBERFIRST 1
// current address in EEPROM second number 
#define ADDRESS_NUMBERSECOND 2


// print data to serial port 
void dataPrint(unsigned long Count, int Temperature){
  Serial.print("RAWMONITOR");
  Serial.print(Count);
  Serial.print("_");
  Serial.print(Temperature);
  Serial.write(255);
}


// measure temperature
int getTemperature(void){
  int i;
  float average;
  int samples[NUMSAMPLES];
  float thermistorResistance;
  int Temperature; 

  // acquire N samples
  for (i=0; i< NUMSAMPLES; i++) {
    samples[i] = analogRead(THERMISTORPIN);
    delay(10);
  }

  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
    average += samples[i];
  }
  average /= NUMSAMPLES;

  // convert the value to resistance
  thermistorResistance = average * SERIESRESISTOR / (1023 - average);
  
  float steinhart;
  steinhart = thermistorResistance / THERMISTORNOMINAL;          // (R/Ro)
  steinhart = log(steinhart);                       // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                        // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                      // Invert
  steinhart -= 273.15;                              // convert to C

  // decimal value
  Temperature = steinhart * 10;
  return(Temperature);
}

// QCM frequency by counting the number of pulses in a fixed time 
unsigned long frequency = 0;
// thermistor temperature
int temperature = 0;

void setup(){
  Serial.begin(115200);
  // Configure the reference voltage used for analog input 
  analogReference(EXTERNAL);
  // init the frequency counter
  FreqCount.begin(GATE);
}

void loop(){
  // read the openQCM serial number at the connection
  if (Serial.available()) {
    int val = Serial.parseInt();
    if (val == 1){
      byte valueSeries = EEPROM.read(ADDRESS_SERIES);
      byte valueNumberFirst = EEPROM.read(ADDRESS_NUMBERFIRST);
      byte valueNumberSecond = EEPROM.read(ADDRESS_NUMBERSECOND);
      Serial.print("SERIALNUMBER");
      Serial.print(valueSeries, DEC);
      Serial.print(valueNumberFirst, DEC);
      Serial.print(valueNumberSecond, DEC);
      Serial.write(255);
    }
  }
  
  // read quartz crystal microbalance frequency and temperature 
  if (FreqCount.available()) 
  {
    frequency = FreqCount.read();       // measure QCM frequency
    temperature = getTemperature();     // measure temperature 
    dataPrint(frequency, temperature);  // print data
  }
}

