

/************************************************************
   setup
*/
void setup() {
  Serial.begin(115200);

  piSerialSetup();
  lcdSetup();
  ledSetup();

  delay(1000);

  //setLedColor(255,0,0);
}

/************************************************************
   loop
*/
void loop() {

  piSerialLoop();
  lcdLoop();
  ledLoop();
}
