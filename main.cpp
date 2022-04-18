/**
 * Note: Read this program in conjunction with the
 * requirements document and flowchart.
*/
#include <Arduino.h>
#include "messages.h"

//Set debugWaitDelayTimes to 1 to use short delay times, 0 to use requirements specified times
#define debugWaitDelayTimes 0
boolean isHeaterOff = true;
boolean isPumpOff = true;
boolean isInRunBurnerMode = false;
#if debugWaitDelayTimes
  //Wait times for testing & debugging
  const unsigned long oilPumpOnTime = 4000;
  const unsigned long oilHeaterOnTime = 4000;
  const unsigned long burnerPurgeTime = 4000;
  const unsigned long airSolenoidDelayTime = 4000;
  const unsigned long flameSenseWaitTime = 4000;
#else
  //Wait times specified in requirements
  const unsigned long oilPumpOnTime = 60000;
  const unsigned long oilHeaterOnTime = 6000000;
  const unsigned long burnerPurgeTime = 5000;
  const unsigned long airSolenoidDelayTime = 1000;
  const unsigned long flameSenseWaitTime = 5000;
#endif

//Forward function declarations
int run_oil_pump();
int run_oil_heater();
int igniter(unsigned long);

//Input Pins
const int oilLevelReadPin = 2;
const int oilTemperatureReadPin = 3;
const int flameSenseReadPin = 4;
//Output Pins
const int oilPumpSwitch = 7;
const int oilHeaterSwitch = 8;
const int nozzleBlockSwitch = 9;
const int blowerFanSwitch = 10;
const int igniterSwitch = 11;
const int airSolenoidSwitch =12;
const int abortSwitch = 13;

void setup() {
  lcd_init();
  lcd_on();
  pinMode(oilLevelReadPin, INPUT);
  pinMode(oilPumpSwitch, OUTPUT);
  pinMode(oilTemperatureReadPin, INPUT);
  pinMode(oilHeaterSwitch, OUTPUT);
  pinMode(nozzleBlockSwitch, OUTPUT);
  pinMode(blowerFanSwitch, OUTPUT);
  pinMode(igniterSwitch, OUTPUT);
  pinMode(airSolenoidSwitch, OUTPUT);
  pinMode(abortSwitch, OUTPUT);
}

int get_state_oil_level() {
  return digitalRead(oilLevelReadPin);
}

int get_state_oil_temperature() {
  return digitalRead(oilTemperatureReadPin);
}

int get_state_flame_sense() {
  return digitalRead(flameSenseReadPin);
}

/**
 * lcd is not written to if this function invoked from within abort()
 * Reason: A hack to avoid overwriting issue.
 */
void set_state_oil_pump(int state, boolean abortInvoked = false) {
  digitalWrite(oilPumpSwitch, state);
  if(!abortInvoked) {
    lcd_pump_state(state);
  }
}

/**
 * lcd is not written to if this function invoked from within abort()
 * Reason: A hack to avoid overwriting issue.
 */
void set_state_oil_heater(int state, boolean abortInvoked = false) {
  digitalWrite(oilHeaterSwitch, state);
  if(!abortInvoked) {
    lcd_heater_state(state);
  }
}

void set_state_nozzle_block_heater(int state) {
  digitalWrite(nozzleBlockSwitch, state);
}

void set_state_blower_fan(int state) {
  digitalWrite(blowerFanSwitch, state);
}
void set_state_igniter(int state) {
  digitalWrite(igniterSwitch, state);
}

void set_state_air_solenoid(int state) {
  digitalWrite(airSolenoidSwitch, state);
}

/**
 * Function: check_oil_level
 * Runs the pump if the oil level is low by invoking run_oil_pump function.
 * @return HIGH if oil full else calls abort function.
 */
int check_oil_level() {
  int oilLevel = get_state_oil_level();
  if(oilLevel == LOW) {
    oilLevel = run_oil_pump();
  }
  if(oilLevel == HIGH) {
    set_state_oil_pump(LOW);
  }
  else {
    lcd_oil_pump_timeout();
    abort();
  }
  return oilLevel;
}

/**
 * Function: check_oil_temperature
 * Heats the oil if oil temperature is low by invoking run_oil_heater function.
 * @return HIGH if oil warm else calls abort function.
 */
int check_oil_temperature() {
  int temperature = get_state_oil_temperature();
  if(temperature == LOW) {
    set_state_oil_heater(HIGH);
    set_state_nozzle_block_heater(HIGH);
    temperature = run_oil_heater();
  }
  if(temperature == HIGH) {
    set_state_oil_heater(LOW);
  }
  else {
    lcd_oil_heater_timout();
    abort();
  }
  return temperature;
}

/**
 * Function:run_oil_heater
 * Switches on oil heater.
 * Monitors oil temperature.
 * If in burner mode and oil level low then runs oil pump
 * Heater remains on until oil warm OR allowable runtime exhausted.
 * @return HIGH if oil warm else LOW.
 */
int run_oil_heater() {
  isHeaterOff = false;
  unsigned long startHeaterTime = millis();
  set_state_oil_heater(HIGH);
  while ((millis() - startHeaterTime) < oilHeaterOnTime) {
    if(get_state_oil_level() == LOW && isPumpOff == true && isInRunBurnerMode == true) {
      run_oil_pump();
    }
    if(get_state_oil_level() == HIGH) {
      set_state_oil_pump(LOW);
    }
    if(get_state_oil_temperature() == HIGH) {
      set_state_oil_heater(LOW);
      isHeaterOff = true;
      return HIGH;
    }
  }
  set_state_oil_heater(LOW);
  isHeaterOff = true;
  return LOW;
}

/**
 * Function: run_oil_pump
 * Switches on pump.
 * Monitors oil level.
 * If in burner mode and oil temperature low then switches on oil heater.
 * Pump runs until oil full OR allowable runtime exhausted.
 * @return HIGH if oil full else LOW
 */
int run_oil_pump() {
  isPumpOff = false;
  unsigned long startPumpTime = millis();
  set_state_oil_pump(HIGH);
  while ((millis() - startPumpTime) < oilPumpOnTime) {
    if(get_state_oil_temperature() == LOW && isHeaterOff == true && isInRunBurnerMode == true) {
      run_oil_heater();
    }
    if(get_state_oil_temperature() == HIGH) {
      set_state_oil_heater(LOW);
    }
    if(get_state_oil_level() == HIGH) {
      set_state_oil_pump(LOW);
      isPumpOff = true;
      return HIGH;
    }
  }
  set_state_oil_pump(LOW);
  isPumpOff = true;
  return LOW;
 }

/**
 * Method: start_burner
 * switch on blower fan
 * if flame sense high, abort
 * else :
 *   - wait 10 secs and switch on igniter
 *   - wait 5 secs and switch on air solenoid
 *   - wait 5 secs and read flame sense
 *   - if flame sense low then abort else run burner (stage 5)
 */
void start_burner() {
  if(get_state_flame_sense() == LOW) {
    set_state_nozzle_block_heater(HIGH);
    set_state_blower_fan(HIGH);
    delay(burnerPurgeTime);
    igniter(millis());
    delay(airSolenoidDelayTime);
    set_state_air_solenoid(HIGH);
    delay(flameSenseWaitTime);
  }
  else {
    lcd_flame_sensed();
    abort();
  }
}

/**
 * Function:igniter
 * Attempts to ignite the fuel.
 * Igniter continues to ignition OR allowable runtime exhausted.
 * @return HIGH if ignition successful, i.e. flame sense high, else LOW.
 */
int igniter(unsigned long igniterStartTime) {
  set_state_igniter(HIGH);
  while ((millis() - igniterStartTime) < burnerPurgeTime) {
    //Flame sense may be simulated by closing the toggle switch
    if(get_state_flame_sense()) {
      return HIGH;
    }
  }
  return LOW;
}

/**
 * Stage 4: Run the burner.
 * Runs the oil pump and oil heater at specific intervals within an infinite loop.
 * The BlockNot library timer is used to control when the pump and heater functions invoked.
 * The loop terminates under these conditions:
 * - the flame sense is LOW.
 * - the controller is powered down.
 * - the oil pump or heater runtime exceeds the maximum specified respective wait time in which case the abort function is invoked.
 */
void run_burner() {
  isInRunBurnerMode = true;
  set_state_igniter(LOW);
  while(true) {
    check_oil_level();
    check_oil_temperature();
    lcd_burner_in_run_mode();
    if(get_state_flame_sense() == LOW) {
      lcd_flame_not_sensed();
      abort();
    }
  }
}

/**
 * Function: abort
 * Shuts down the system and exits.
 */
void abort() {
  digitalWrite(abortSwitch, HIGH);
  set_state_blower_fan(LOW);
  set_state_igniter(LOW);
  set_state_air_solenoid(LOW);
  set_state_oil_pump(LOW, true);
  set_state_oil_heater(LOW, true);
  set_state_nozzle_block_heater(LOW); // Note: no led
  exit(0);
}

/**
 * Function: loop
 * This superloop runs continuously.
 * It can exercise all five stages specified in requirements and flowchart.
 */
void loop() {
  int level = check_oil_level();
  int heat = LOW;
  if(level) {
    heat = check_oil_temperature();
  }
  if(heat) {
    start_burner();
  }
  if(level && heat) {
    run_burner();
  }
  exit(0);
}
