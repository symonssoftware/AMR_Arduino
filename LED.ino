#include <Adafruit_NeoPixel.h>

#define LED_PIN 13

#define LED_COUNT 149

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
   
uint16_t      currentPixel = 0;    
uint32_t      newColor = 0;
int           LOOP_INTERVAL_MS = 10;
unsigned long previousMillis = 0;

/************************************************************
   ledSetup
*/
void ledSetup() {
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}


/************************************************************
   ledLoop
*/
void ledLoop() {
  
  unsigned long currentMillis = millis();

  // We don't want to take any time doing anything in the loop
  // unless we need to so when all LEDs have been addressed, just
  // stop until the next command to change the color comes through
  if (currentPixel < LED_COUNT) { 
    
    if ((currentMillis - previousMillis) >= LOOP_INTERVAL_MS) { 

      colorWipe(newColor);
      previousMillis = currentMillis;
      currentPixel++;
    }
  }
}

/************************************************************
   setLedColor
*/
void setLedColor(int red, int green, int blue) {
  
  currentPixel = 0;
  newColor = strip.Color(red, green, blue);
}

/************************************************************
   colorWipe
*/
void colorWipe(uint32_t color) {
  
  strip.setPixelColor(currentPixel, color); 
  strip.show();                            
}
