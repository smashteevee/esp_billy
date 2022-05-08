/* 

Copyright (C) 2015 Michal Canecky
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

// Also borrowed liberally from: https://github.com/jordan-public/Thermometer-Attiny85/blob/master/Thermometer-Attiny85.ino

#include "SystemStatus.h"
#include <avr/sleep.h>
#include <avr/power.h>
//#include "wiring_private.h"


SystemStatus::SystemStatus(uint8_t apin_batt) : pin_batt(apin_batt) {

}

SystemStatus::SystemStatus() : pin_batt(255) {

}


float SystemStatus::getVCC(int bandgap_voltage) {
  //reads internal 1V1 reference against VCC
  /*#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)
    ADMUX = _BV(MUX5) | _BV(MUX0); // For ATtiny84
  #elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__)
    ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85/45
  #elif defined(__AVR_ATmega1284P__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega1284
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
  #endif
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  uint8_t low = ADCL;
  long val = (ADCH << 8) | low;
  //discard previous result
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  low = ADCL;
  val = (ADCH << 8) | low;
  
  return ((long)1024 * bandgap_voltage) / val;  */

 // Measure chip voltage (Vcc)
 float rawVcc;
  ADCSRA |= _BV(ADEN);   // Enable ADC
  ADMUX  = 0x0c; //| _BV(REFS2);    // Use Vcc as voltage reference, 
                                 //    bandgap reference as ADC input
  delay(SETTLE);                    // Settling time min 1 ms, there is 
                                 //    time so wait 100 ms
  rawVcc = (float)getADC();      // use next sample as initial average
  for (int i=2; i<2000; i++) {   // calculate running average for 2000 measurements
    rawVcc += ((float)getADC() - rawVcc) / float(i);
  }
  ADCSRA &= ~(_BV(ADEN));        // disable ADC

  return 1024 * (float)(bandgap_voltage/1000) / rawVcc;
  
}


int SystemStatus::getVBatt(int vcc) {
  if (this->pin_batt == 255) return 0;
  unsigned int a = analogRead(this->pin_batt); //discard first reading
  a = analogRead(this->pin_batt);
  return (long)vcc * (long)a / 1024;
}


int SystemStatus::getFreeRAM() {
  extern int  __bss_end;
  extern int  *__brkval;
  int free_memory;
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  return free_memory;
}


int SystemStatus::getkHz() {
  return F_CPU / 1000;
}


int SystemStatus::getMHz() {
  return F_CPU / 1000000;
}

// From: http://21stdigitalhome.blogspot.com/2014/10/trinket-attiny85-internal-temperature.html
int SystemStatus::getChipTemperatureCelsius(int bandgap_voltage, float offset = 0, int vcc_voltage = 0) {
  int i;
  int t_celsius; 
  uint8_t vccIndex;
  float rawTemp;
  
  // Measure temperature
  ADCSRA |= _BV(ADEN);           // Enable AD and start conversion
  ADMUX = 0xF | _BV( REFS1 );    // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(SETTLE);                 // Settling time min 1 ms, wait 100 ms

  rawTemp = (float)getADC();     // use next sample as initial average
  for (int i=2; i<200; i++) {   // calculate running average for 200 measurements
    rawTemp += ((float)getADC() - rawTemp) / float(i); 
  }  
  ADCSRA &= ~(_BV(ADEN));        // disable ADC  

  
  if (!vcc_voltage) {
    vcc_voltage = getVCC(bandgap_voltage);
  }
  

  //index 0..13 for vcc 1.7 ... 3.0
  vccIndex = min(max(17,(uint8_t)(vcc_voltage * 10)),30) - 17;   

    Serial.print("rawTemp (k): ");
  Serial.println(rawTemp);
  // Temperature compensation using the chip voltage 
  // with 3.0 V VCC is 1 lower than measured with 1.7 V VCC 
  t_celsius = (int)(chipTemp(rawTemp, offset) + (float)vccIndex / 13);  
                                                                                   
  return t_celsius;
}

// Calibration of the temperature sensor has to be changed for your own ATtiny85
// per tech note: http://www.atmel.com/Images/doc8108.pdf
float SystemStatus::chipTemp(float raw, float offset = 0) {
  const float chipTempOffset = 273.15 + offset;           // Use offset to adjust
  const float chipTempCoeff = 0.95;            // Your value here, it may vary
  Serial.print("Temp w. Offset (C): ");
  Serial.println(raw - chipTempOffset);
  return((raw - chipTempOffset) / chipTempCoeff);
}

// Common code for both sources of an ADC conversion
int SystemStatus::getADC() {
  ADCSRA  |=_BV(ADSC);           // Start conversion
  while((ADCSRA & _BV(ADSC)));    // Wait until conversion is finished
  return ADC;
}

int8_t SystemStatus::getTemperatureInternal(uint8_t offset) {
  /* from the data sheet
    Temperature / °C -45°C +25°C +85°C
    Voltage     / mV 242 mV 314 mV 380 mV
  */
   //reads internal internal temp with 1V1 reference
  #if defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) // 
    ADMUX = (1<<REFS1) | (1<<MUX5)| (1<<MUX1); //turn 1.1V reference and select ADC8 For ATtiny84
    #elif (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)// -40°=230lsb +25°=300lsb +85=370lsb
    ADMUX = (1<<REFS1) | (1<<MUX3)| (1<<MUX2)| (1<<MUX1)| (1<<MUX0); //turn 1.1V reference and select ADC4 For ATtiny85
  #elif defined(__AVR_ATmega1284P__)
      // For ATmega1284 not working
  #else // -45°=242mV +25=314mV +85°=380mV
   ADMUX =  (1<<REFS0) |(1<<REFS1) | (1<<MUX3); //turn 1.1V reference and select ADC8  for atmega 328p
  #endif
  delay(5); //wait for internal reference to settle
	// start the conversion
	ADCSRA |= bit(ADSC);
	//sbi(ADCSRA, ADSC);
	// ADSC is cleared when the conversion finishes
	while (ADCSRA & bit(ADSC));
	//while (bit_is_set(ADCSRA, ADSC));
	uint8_t low  = ADCL;
	uint8_t high = ADCH;
	//discard first reading
	ADCSRA |= bit(ADSC);
	while (ADCSRA & bit(ADSC));
	low  = ADCL;
	high = ADCH;
	int a = (high << 8) | low;
  Serial.print("raw temp: ");
  Serial.println(a);
  return chipTemp(a, offset); 
}




void jebat_cecky() {
  detachInterrupt(0);
}


//
