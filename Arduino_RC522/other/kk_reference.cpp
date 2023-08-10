/*
   --------------------------------------------------------------------------------------------------------------------
   Example to change UID of changeable MIFARE card.
   --------------------------------------------------------------------------------------------------------------------
   This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid

   This sample shows how to set the UID on a UID changeable MIFARE card.
   NOTE: for more informations read the README.rst

   @author Tom Clement
   @license Released into the public domain.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   9     // Configurable, see typical pin layout above
#define SS_PIN    10    // Configurable, see typical pin layout above
#define LED 5
#define sw 6

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

/* Set your new UID here! */
#define NEW_UID {0xD1, 0xD7, 0x61, 0x75}
int UIDArray[4] = {01, 02, 03, 04};

MFRC522::MIFARE_Key key;

String ReadString() {
  String tmp = "";
  while (1) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        return tmp;
      }
      tmp += c;
      delay(5);    // 沒有延遲的話 UART 串口速度會跟不上Arduino的速度，會導致資料不完整
    }
  }
}

void dumpCard() {
  Serial.println(F("Waiting card..."));
  // Look for new cards and Select one of the cards
  while ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial()) {
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

String ManualSetUID() {
  Serial.println("Manual mode.");
  Serial.println("Enter UID....");
  Serial.println("!!!Need NewLine flag !!!");
  String UIDstr = ReadString();
  Serial.print("Get UID = ");
  Serial.println(UIDstr);

  //12:45:78:01
  UIDArray[0] = UIDstr.substring(0, 1).toInt() * 16 + UIDstr.substring(1, 2).toInt();
  UIDArray[1] = UIDstr.substring(3, 4).toInt() * 16 + UIDstr.substring(4, 5).toInt();
  UIDArray[2] = UIDstr.substring(6, 7).toInt() * 16 + UIDstr.substring(7, 9).toInt();
  UIDArray[3] = UIDstr.substring(9, 10).toInt() * 16 + UIDstr.substring(10, 11).toInt();
}
String ReadUID() {
  // Look for new cards
  while ( ! mfrc522.PICC_IsNewCardPresent()) {

  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  UIDArray[0] = mfrc522.uid.uidByte[0];
  UIDArray[1] = mfrc522.uid.uidByte[1];
  UIDArray[2] = mfrc522.uid.uidByte[2];
  UIDArray[3] = mfrc522.uid.uidByte[3];

  String str = String(UIDArray[0], HEX) + ":" + String(UIDArray[1], HEX) + ":" + String(UIDArray[2], HEX) + ":" + String(UIDArray[3], HEX);
  return str;
  delay(500);
}

void WriteUID() {
  Serial.println("Ready to write UID");
  //Look for new cards, and select one if present
  while ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
  }

  // Now a card is selected. The UID and SAK is in mfrc522.uid.

  // Dump UID
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Set new UID
  byte newUidbyte[4] = {};

  newUidbyte[0] = (byte) (0xff & UIDArray[0]);
  newUidbyte[1] = (byte) ((0xff & UIDArray[1]));
  newUidbyte[2] = (byte) ((0xff & UIDArray[2]));
  newUidbyte[3] = (byte) ((0xff & UIDArray[3]));

  if ( mfrc522.MIFARE_SetUid(newUidbyte, (byte)4, true) ) {
    Serial.println(F("Wrote new UID to card."));
  }

  // Halt PICC and re-select it so DumpToSerial doesn't get confused
  mfrc522.PICC_HaltA();
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    return;
  }

  // Dump the new memory contents
  Serial.println(F("New UID and contents:"));
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  delay(2000);
}
void setup() {
  Serial.begin(115200);  // Initialize serial communications with the PC
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
  Serial.println(F("-----RFID Card cloner ver.1.0-----"));

  pinMode(LED, OUTPUT);
  pinMode(sw, INPUT);
  digitalWrite(LED, LOW);

  // Prepare key all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

// But of course this is a more proper approach
void loop() {
  //Auto Clone Mode
  if (digitalRead(sw) == HIGH) {
    while (!ReadUID()) {
    }
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(3000);
    digitalWrite(LED, HIGH);
    WriteUID();
    digitalWrite(LED, LOW);
    delay(250);
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(3000);
  }
  //Computer Control Mode
  else {
    Serial.print(F("cmd>>>"));
    while (!Serial.available()) {
    }
    char cmd = Serial.read();
    if (cmd == 'R') {
      Serial.println(F("Card reading mode."));
      Serial.println(F("Wating card..."));
      String currentUID = ReadUID();
      Serial.println(F("Successfully read card"));
      Serial.print(F("Current UID in memory is "));
      Serial.println(currentUID);
    }
    if (cmd == 'W') {
      Serial.println(F("Card overwrite mode."));
      WriteUID();
      Serial.println(F("Successfully written card"));
      Serial.println(F("Confirm card UID....."));
      Serial.print(F("Current card UID is "));
      Serial.println(ReadUID());
    }
    if (cmd == 'M') {
      ManualSetUID();
    }
    if (cmd == 'D') {
      Serial.println(F("Card information dump mode."));
      dumpCard();
    }
    if (cmd == 'U') {
      Serial.println(F("Memory mode."));
      Serial.print(F("Current UID in memory is "));
      Serial.println(String(UIDArray[0], HEX) + ":" + String(UIDArray[1], HEX) + ":" + String(UIDArray[2], HEX) + ":" + String(UIDArray[3], HEX));
    }
    else {
      Serial.println();
    }
  }
}
