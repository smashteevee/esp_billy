

/*
* Attiny85 code to read rising PIR sensor events, save it and periodically send updates via ESP8266 (ESP Billy)
 * via Software serial pins
*/


#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include "src/SystemStatus.h"

//#define DEBUG
//#include "DebugMacros.h"
volatile bool readVoltage = false;

bool detectedRecentActivity = false; // motion detection
volatile int watchdog_counter = 0;
// ATTIny85 Pins per https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x5.md

// RIGHT_SIDE PINS
// - VCC
const int PIR_PIN = A1; // - A1/PB2
// PB1 (unused)
// PB0 (TX)

// LEFT SIDE PINS
// PB5 (unused)
// PB3 (unused)
const int MOSFET_GATE_PIN = PIN_B4; // PB4
// GND



SystemStatus sys = SystemStatus();
const int CUTOFF_VOLTAGE =  3000; // Roughly 0.9v/cell for nimh * 4; but 
//because LDO drops to 3.3, and VCC reading is not accurate, we set lower when sag
unsigned long int bandgapVoltage = 0L; // TO READ Calibrated VRef FROM EEPROM
float tempOffset = -1.46; // TO READ FROM EEPROM TODO:
const int WAKE_FREQUENCY_PERIODS = 3;//150;// 20min *60s = 1200 / 8s wakes = 

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

// this function replaces wdt_disable()
void wdt_off() {
  wdt_reset();
  MCUSR = 0x00;
  WDTCR |= (1<<WDCE) | (1<<WDE);
  WDTCR = 0x00;
}

/*
* WDT interrupt
*/
ISR(WDT_vect) {
  Serial.println("WDT Wokeup!");
  //wdt_off();
  wdt_disable();  // disable watchdog
  watchdog_counter++; // increment wdt counter
}

/*
ISR set flag for things we want to do when awake - read the voltage
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
  // Watchdog wake every 8 seconds
  setup_watchdog(9);  
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // Set sleep mode
  sleep_enable();                        // Enables the sleep bit in the mcucr register so sleep is possible

  //interrupts ();

  sleep_cpu();                          // Zzzzzzzzzz...

  sleep_disable();                       // first thing after waking from sleep: clear SE bit

  // re-enable ADC
  power_adc_enable();
  ADCSRA |= (1 << ADEN);


}

void setup() {

  
  DDRB &= ~(1 << MOSFET_GATE_PIN);   // Set MOSFET GATE PIN AS INPUT
  PORTB &= ~(1 << MOSFET_GATE_PIN);  // Set Mosfet Gate Pin LOW to turn off Supply

  Serial.begin(115200);  // Begin serial communication with ESP
  pinMode(PIR_PIN, INPUT);    // read in PIR


  // Read in the previously calibrated Bandgap 1v1
  delay(1000);

  // Read in the last calibrated 1v1 from EEPROM
  EEPROM.get(15, bandgapVoltage);  

  //Read first EEPROM cell.
  Serial.print("EEPROM[15] = "); Serial.println(EEPROM[15]);
  Serial.print("EEPROM[16] = "); Serial.println(EEPROM[16]);
  Serial.print("EEPROM[17] = "); Serial.println(EEPROM[17]);
  Serial.print("EEPROM[18] = "); Serial.println(EEPROM[18]);




  Serial.print("Starting up...bandgapVoltage = ");
  Serial.println(bandgapVoltage);
  // Setup PCI
  //pinMode( PIN_PB2, INPUT ); // SEE IF THIS HELPS
  initInterrupt();

}

void loop() {
  
  int analogValue = 0;
  // read any serial response from ESP Billy
  /*if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte);
  }*/

  // Check Billy VCC to see if we are good to operate, otherwise back to sleep
  long vcc= sys.getVCC(bandgapVoltage) * 1000; // TODO: Consider checking only every few wakeups

       Serial.print("; vcc: ");
      Serial.print(vcc);

   if (vcc > CUTOFF_VOLTAGE) {
      Serial.print("watchdog_counter:");
      Serial.print(watchdog_counter);
      

      
   

      // If awake because of PCI, take note if rising/high voltage
      if (readVoltage) {
        readVoltage = false;  // reset flag
        Serial.println("reading voltage");
        // Only bother taking note if we hadnt previously detected recent activity
        if (!detectedRecentActivity) {

          Serial.println("recently new activity");
          analogValue = analogRead(PIR_PIN);  // read sensor value
          Serial.print("analogValue: ");
          Serial.println(analogValue);
          // Only bother taking note if rising edge (PIR activated)
          if (analogValue > 300) { // TODO make this dynamic; supposed to be 419>
            Serial.println("high detected");
        
            // Take note that we've detected recent activity
            detectedRecentActivity = true;

          }
        }
      } else if (watchdog_counter > WAKE_FREQUENCY_PERIODS) { // ~else if awake because it's time to transmit update to ESP Billy

      // Get temperature
      //int temperature = 22;
      wdt_reset();
      int temperature = sys.getChipTemperatureCelsius(bandgapVoltage, tempOffset, vcc);
       // int temperature = sys.getTemperatureInternal(tempOffset);
      Serial.println("Got temp");
      wdt_reset();
        // TODO: PAUSE WDT
        // POWER ON ESP BILLY
        DDRB |= (1 << MOSFET_GATE_PIN);   // Set Gate Pin as Output
        PORTB |= (1 << MOSFET_GATE_PIN);  // Set Mosfet Gate Pin HIGH 
       // Serial.println("Mosfet GATE ON");
        // TODO: WAIT FOR READY THEN SEND instead of HARDCODE 10s
        //delay(10000);
        // TODO: Replace with loop and reset the WDT
        wdt_reset();
        delay(3000);
        wdt_reset();
        delay(4000);
        wdt_reset();
        /*if (!detectedRecentActivity) {
          detectedMotion = 0;
          voltage = 0;
        }*/
        // Send info over Serial to ESP Billy
        Serial.print("detectedMotion:");
        Serial.print(detectedRecentActivity);
        Serial.print(", voltage:");
        Serial.print(vcc);
        Serial.print(", temp:");
        Serial.println(temperature);

        // TODO: PUT ESP BILLY TO SLEEP after getting some response
          wdt_reset();
        delay(4000);

        // POWER OFF ESP BILLY
       // Serial.println("Power off mosfet");
        PORTB &= ~(1 << MOSFET_GATE_PIN);  // Set Mosfet Gate Pin LOW to turn off
        DDRB &= ~(1 << MOSFET_GATE_PIN);   // Set MOSFET GATE PIN AS INPUT
        Serial.print("sleep billy!`");
        delay(500);
        watchdog_counter = 0;  // reset watchdog sleep counter
        detectedRecentActivity = false; // reset detection flag
      
      }
   }
  sleepNow();  // ZZZ until PCI or time to send upate
}