#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 4 line display

void lcdSetup() {
  Wire.begin();
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight 
}

/************************************************************
   lcdLoop
*/
void lcdLoop() {

}
