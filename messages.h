#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Forward declaration
const char* int_to_string(int state);

char* message[] = {
  "ABORTED",              //message[0]
  "FLAME SENSED",         //message[1]
  "IN START BURNER MODE", //message[2]
  "OIL PUMP TIME OUT",    //message[3]
  "OIL HEATER TIME OUT",  //message[4]
  "FLAME NOT SENSED",     //message[5]
  "IN RUN BURNER MODE",   //message[6]
  "OIL PUMP ",            //message[7]
  "OIL HEATER",           //message[8]
  };

LiquidCrystal_I2C lcd(0x27,20,4);

void lcd_init() {
  lcd.init();
}

void lcd_on() {
  lcd.backlight();
  lcd.clear();
}

void lcd_burner_in_run_mode() {
  lcd.setCursor(1,3);
  lcd.print(message[6]);
}

void lcd_flame_sensed() {
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print(message[1]);
  lcd.setCursor(0,1);
  lcd.print(message[2]);
  lcd.setCursor(6,2);
  lcd.print(message[0]);
}

void lcd_flame_not_sensed() {
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print(message[5]);
  lcd.setCursor(1,1);
  lcd.print(message[6]);
  lcd.setCursor(6,2);
  lcd.print(message[0]);
}

void lcd_pump_state(int pumpState) {
  lcd.setCursor(1,0);
  lcd.print(message[7]);
  lcd.setCursor(10,0);
  lcd.print(int_to_string(pumpState));
}

void lcd_heater_state(int heaterState) {
  lcd.setCursor(1,1);
  lcd.print(message[8]);
  lcd.setCursor(12,1);
  lcd.print(int_to_string(heaterState));
}

void lcd_oil_pump_timeout() {
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print(message[3]);
  lcd.setCursor(6,2);
  lcd.print(message[0]);
}

void lcd_oil_heater_timout() {
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print(message[4]);
  lcd.setCursor(6,2);
  lcd.print(message[0]);
}

/**
 * Function: int_to_string, a helper function to return the bespoke
 * string equivalent value of an int.
 * @params: int state
 * @return: ON if state is HIGH, otherwise OFF
 */
const char* int_to_string(int state) {
  if(state == 1) {
    return "ON ";
  }
  else {
    return "OFF";
  }
}
