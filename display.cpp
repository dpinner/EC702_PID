#include "display.h"
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

static hd44780_I2Cexp lcd;  // declare lcd object: auto locate & config display for hd44780 chip

void initDisplay() {
  // initialize LCD with number of columns and rows:
  lcd.begin(16, 2);

  // Print a message to the LCD
  lcd.print("Hello, World!");
}

void updateDisplay() {
  lcd.setCursor(0, 1);
  lcd.print(millis() / 1000);
}