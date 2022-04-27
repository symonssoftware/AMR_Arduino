const byte MAX_RX_DATA_BUFFER_SIZE = 255;
byte rxBuffer[MAX_RX_DATA_BUFFER_SIZE];

const byte WAITING_FOR_HEADER_BYTE1 = 0;
const byte WAITING_FOR_HEADER_BYTE2 = 1;
const byte WAITING_FOR_DATA_LENGTH_BYTE = 2;
const byte WAITING_FOR_CMD_BYTE = 3;
const byte WAITING_FOR_DATA_BYTES = 4;
const byte WAITING_FOR_CRC_BYTE1 = 5;
const byte WAITING_FOR_CRC_BYTE2 = 6;

const byte HEADER_BYTE1_POSITION = 0;
const byte HEADER_BYTE2_POSITION = 1;
const byte DATA_LENGTH_BYTE_POSITION = 2;
const byte CMD_BYTE_POSITION = 3;
const byte START_OF_DATA_POSITION = 4;

byte msgReceiverState = WAITING_FOR_HEADER_BYTE1;

const byte SERIAL_COMMAND_BATTERY_VOLTAGE = 0x02;
const byte SERIAL_COMMAND_HEADING = 0x03;
const byte SERIAL_COMMAND_LED = 0x04;

const byte SERIAL_DEBUG = 0;

/************************************************************
 * piSerialSetup
 */
void piSerialSetup() {
  Serial2.begin(115200);
  Serial2.flush();
}

/************************************************************
 * piSerialLoop
 */
void piSerialLoop() {

  byte nextRxByte;
  
  static byte dataIndex = 0;
  static byte crc1 = 0;
  static byte crc2 = 0;

  if (Serial2.available()) {

    switch (msgReceiverState) {
      
      case WAITING_FOR_HEADER_BYTE1:
        nextRxByte = Serial2.read();
        
        if (nextRxByte == 0xFF) {
          if (SERIAL_DEBUG) Serial.println("New message");
          rxBuffer[HEADER_BYTE1_POSITION] = nextRxByte;
          msgReceiverState = WAITING_FOR_HEADER_BYTE2;
        }
        else {
          clearRxBuffer();
        }
      break;
      case WAITING_FOR_HEADER_BYTE2:
        nextRxByte = Serial2.read();
        
        if (nextRxByte == 0xFE) {
          if (SERIAL_DEBUG) Serial.println("2nd header byte read in");
          rxBuffer[HEADER_BYTE2_POSITION] = nextRxByte;
          msgReceiverState = WAITING_FOR_DATA_LENGTH_BYTE;
        }
        else {
          clearRxBuffer();
        }

      break;
      case WAITING_FOR_DATA_LENGTH_BYTE:
        nextRxByte = Serial2.read();
        if (SERIAL_DEBUG) Serial.print("Data length byte read in: ");
        if (SERIAL_DEBUG) Serial.println(nextRxByte, HEX);
        rxBuffer[DATA_LENGTH_BYTE_POSITION] = nextRxByte;
        msgReceiverState = WAITING_FOR_CMD_BYTE;
      break;
      case WAITING_FOR_CMD_BYTE:
        nextRxByte = Serial2.read();
        if (SERIAL_DEBUG) Serial.print("Cmd byte read in: ");
        if (SERIAL_DEBUG) Serial.println(nextRxByte, HEX);
        rxBuffer[CMD_BYTE_POSITION] = nextRxByte;
        dataIndex = 0;
        crc1 = 0;
        crc2 = 0;
        msgReceiverState = WAITING_FOR_DATA_BYTES;
      break;
      case WAITING_FOR_DATA_BYTES:
        if (dataIndex < rxBuffer[DATA_LENGTH_BYTE_POSITION]) {
          nextRxByte = Serial2.read();
          if (SERIAL_DEBUG) Serial.print("Reading data... ");
          if (SERIAL_DEBUG) Serial.println(nextRxByte, HEX);
          rxBuffer[dataIndex + START_OF_DATA_POSITION] = nextRxByte;
          dataIndex++;
        }
        else {
          if (SERIAL_DEBUG) Serial.println("Done with data");
          msgReceiverState = WAITING_FOR_CRC_BYTE1;
        }
      break;
      case WAITING_FOR_CRC_BYTE1:
        crc1 = Serial2.read();
        if (SERIAL_DEBUG) Serial.print("Reading first CRC byte:");
        if (SERIAL_DEBUG) Serial.println(crc1, HEX);
        msgReceiverState = WAITING_FOR_CRC_BYTE2;
      break;
      case WAITING_FOR_CRC_BYTE2:
        crc2 = Serial2.read();
        if (SERIAL_DEBUG) Serial.print("Reading second CRC byte:");
        if (SERIAL_DEBUG) Serial.println(crc2, HEX);
        uint16_t crc = crc16(&rxBuffer[START_OF_DATA_POSITION], 
                             rxBuffer[DATA_LENGTH_BYTE_POSITION]);

        if ((crc1 == (byte)((crc >> 8) & 0xff)) &&
            (crc2 == (byte)crc & 0xff)) {
          if (SERIAL_DEBUG) Serial.println(" -- Message is valid");
          if (SERIAL_DEBUG) Serial.println("");
          sendResponse(rxBuffer[CMD_BYTE_POSITION], true);
          processMessage();
        }
        else {
          Serial.println(" -- Message is borked");
          Serial.println("");
          sendResponse(rxBuffer[CMD_BYTE_POSITION], false);
        }

        clearRxBuffer();
        msgReceiverState = WAITING_FOR_HEADER_BYTE1;
      break;
      default:
        Serial.println("Invalid msgReceiverState");
    }
  }
}

/************************************************************
 * clearRxBuffer
 */
void clearRxBuffer() {
  Serial2.flush();
  for (int i = 0; i < MAX_RX_DATA_BUFFER_SIZE; i++) {
    rxBuffer[i] = 0x00;
  }
}

/************************************************************
 * sendResponse
 */
void sendResponse(byte cmd, bool isAck) {
  byte txBuffer[256];

  txBuffer[0] = 0xFF;
  txBuffer[1] = 0xFE;
  txBuffer[2] = 0x01;

  if (isAck) {
    txBuffer[3] = 0x00;
  }
  else {
    txBuffer[3] = 0x01;
  }

  txBuffer[4] = cmd;

  uint16_t crc = crc16(&txBuffer[START_OF_DATA_POSITION], txBuffer[2]);

  txBuffer[5] = (byte)((crc >> 8) & 0xff);
  txBuffer[6] = (byte)crc & 0xff;

  Serial2.write(txBuffer, 7);
}

/************************************************************
 * crc16
 */
uint16_t crc16(const unsigned char* data_p, unsigned char length) {

  unsigned int reg_crc = 0xFFFF;

  while (length--) {
    reg_crc ^= *data_p++;

    for (int j = 0; j < 8; j++) {
      if (reg_crc & 0x01) {
        reg_crc = (reg_crc >> 1) ^ 0xA001;
      }
      else {
        reg_crc = reg_crc >> 1;
      }
    }
  }

  return reg_crc;
}


/************************************************************
 * crc16
 */
void processMessage() {
  byte cmd = rxBuffer[CMD_BYTE_POSITION];

  switch (cmd) {
    case SERIAL_COMMAND_BATTERY_VOLTAGE:
    
      union {
        byte dataArray[4];
        float batteryVoltage;
      } batteryVoltageUnion;

      for (int i = 0; i < 4; i++) {
        batteryVoltageUnion.dataArray[i] = rxBuffer[i + START_OF_DATA_POSITION];
      }

      Serial.print("Battery voltage: ");
      Serial.println(batteryVoltageUnion.batteryVoltage, 2);
      lcd.setCursor(0, 0); // go to the top left corner
      lcd.print("Batt voltage: ");
      lcd.setCursor(14,0);
      lcd.print(String(batteryVoltageUnion.batteryVoltage, 2) + "V");
    break;
    case SERIAL_COMMAND_HEADING:
    
      union {
        byte dataArray[4];
        float heading;
      } headingUnion;

      for (int i = 0; i < 4; i++) {
        headingUnion.dataArray[i] = rxBuffer[i + START_OF_DATA_POSITION];
      }

      Serial.print("Heading: ");
      Serial.println(headingUnion.heading, 2);
      lcd.setCursor(0, 1); // go to the top left corner
      lcd.print("Heading: ");
      lcd.setCursor(9,1);
      lcd.print(String(headingUnion.heading, 2) + " deg");
    break;
    case SERIAL_COMMAND_LED:
      Serial.println("LED Message");
      setLedColor(255,0,0);
      setLedColor(rxBuffer[START_OF_DATA_POSITION], 
                  rxBuffer[START_OF_DATA_POSITION + 1], 
                  rxBuffer[START_OF_DATA_POSITION + 2]);
    break;
    default:
    
      Serial.print("Invalid command: ");
      Serial.println(cmd, HEX);
  }
}
