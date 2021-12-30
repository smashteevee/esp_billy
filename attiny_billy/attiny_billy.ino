/*
* Attiny85 code to read rising PIR sensor events, save it and periodically send updates via ESP8266 (ESP Billy)
 * via Software serial pins
*/


#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "SystemStatus.h"

//#define DEBUG
//#include "DebugMacros.h"
int state = LOW;             // by default, no motion detected
volatile bool readVoltage = false;
int val = 0; 
float voltage = 0.0;
int detectedRecentActivity = false;
int watchdog_counter = 0;


//SystemStatus sys = SystemStatus();

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}

/*
 Function to set up PCI
*/
static inline void initInterrupt(void)
{
	GIMSK |= (1 << PCIE);   // pin change interrupt enable
	PCMSK |= (1 << PCINT2); // pin change interrupt enabled for PCINT2
	sei();                  // enable interrupts
}

/*
* WDT interrupt
*/
ISR(WDT_vect) {
  watchdog_counter++; // incrementa wdt counter
}

/*
ISR set flag for things we want to do when awake
*/
ISR(PCINT0_vect)
{
  
  readVoltage = true;
 
}

/*
   Function to sleep and wake 
*/
void sleepNow() {
 

  // disable ADC
  ADCSRA &= ~(1<<ADEN);
  power_adc_disable();
  // Watchdog 1 seconds
  setup_watchdog(6);  
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // Set sleep mode
  sleep_enable();                        // Enables the sleep bit in the mcucr register so sleep is possible

  sleep_cpu();                          // Zzzzzzzzzz...

  sleep_disable();                       // first thing after waking from sleep: clear SE bit

  // re-enable ADC
  power_adc_enable();
  ADCSRA |= (1 << ADEN);


}

void setup() {
    Serial.begin(115200); // Begin serial communication with ESP
    pinMode(A1, INPUT); // read in PIR
    
    Serial.write("Hello, from Tiny Billy.");
    // Setup PCI
    initInterrupt();

}

void loop() {
  // read any serial response from ESP Billy
  /*if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte);
  }*/
  
  // If woke because of PCI, take note if rising
  if (readVoltage) {
    readVoltage = false; // reset flag

    // Only bother taking note if we hadnt previously detected recent activity
    if (!detectedRecentActivity) {


        val = analogRead(A1);   // read sensor value

        // Only bother taking note if rising edge (PIR activated)
            if (val > 512) {
              
              // Take note that we've detected recent activity
              detectedRecentActivity = true;
              
              // calculate voltage too
              // TODO: MOVE TO PERIODIC WATCHDOG TIMER CODE TO SEND UPDATE TO ESP BILLY
               voltage = val * (5.0/1023);

              
              
            
            }


    } else

    // Else if wokeup every 30s, wake ESP Billy and send over data to transmit
      if (!watchdog_counter || watchdog_counter > 30) // ~30 secs sleep
    {
       watchdog_counter = 0; // reset sleep counter

        // TODO: WAKE ESP BILLY

        // Send eial info
         Serial.print("data update: ");
         Serial.print(val);
        Serial.print(", voltage:");
        Serial.println(voltage);
    }
  }

 /* if (val == HIGH) {           // check if the sensor is HIGH
    if (state == LOW) {
       Serial.write("TRUE");
       delay(500);    
      state = HIGH;       // update variable state to HIGH
    }
  } else {
      delay(500);             // delay 200 milliseconds 
      
      if (state == HIGH){
        Serial.write("FALSE");
        state = LOW;       // update variable state to LOW
    }
  }
  // Every 5 seconds, send some sensor status to Billy
  //Serial.write(sys.getTemperatureInternal());
 */
 
  sleepNow(); // ZZZ until PCI or time to send upate
  
}