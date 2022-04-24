
/************************************************************
 * setup
 */
void setup() {
  Serial.begin(115200);
  
  piSerialSetup();
  lcdSetup();
}

/************************************************************
 * loop
 */
void loop() {

  piSerialLoop();
  lcdLoop();
}
