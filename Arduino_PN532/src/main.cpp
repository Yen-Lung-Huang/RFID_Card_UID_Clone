#include <Arduino.h>
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>

void readNFC(void);
String byteArrayToHexString(byte* byteArray, byte length);

SoftwareSerial SWSerial(3, 2); // RX, TX

PN532_SWHSU pn532swhsu(SWSerial);
PN532 nfc(pn532swhsu);
String previousUID = "None";
bool cardPresent = false;

void setup(void)
{
  Serial.begin(115200);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    Serial.println("Didn't Find PN53x Module");
    while (1); // Halt
  }
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  nfc.SAMConfig();
}

void loop()
{
  readNFC();
  
  // 延遲時間可依需求調整
  delay(200); // 200 milliseconds delay
}

void readNFC()
{
  boolean success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
  uint8_t uidLength; // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // 定義一個靜態的旗標變數，用來記錄是否已經顯示過提示訊息
  static bool messageShown = false;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  if (success)
  {
    String UID = byteArrayToHexString(uid, uidLength);
    if (UID != previousUID)
    {
      cardPresent = true;
      previousUID = UID;
      Serial.println("\nCard detected!");
      Serial.print("UID: ");
      Serial.println(UID);

      // 如果已經顯示過提示訊息，就將旗標變數設為false，表示下次沒有感應到卡時要再次顯示提示訊息
      if (messageShown)
      {
        messageShown = false;
      }
    }
  }
  else
  {
    // 如果沒有顯示過提示訊息，就顯示提示訊息，並將旗標變數設為true，表示下次沒有感應到卡時不需要再次顯示提示訊息
    if (!messageShown)
    {
      Serial.println("\nNo card detected. Please place a card on the reader.");
      messageShown = true;
    }
    
    if (cardPresent)
    {
      cardPresent = false;
      previousUID = "None";
    }
  }
}



String byteArrayToHexString(byte* byteArray, byte length)
{
  String hexString = "";
  for (byte i = 0; i < length; i++)
  {
    if (byteArray[i] < 0x10)
    {
      hexString += "0";
    }
    hexString += String(byteArray[i], HEX);
    if (i < length - 1)
    {
      hexString += " ";
    }
  }
  // Convert lowercase letters to uppercase
  hexString.toUpperCase();
  return hexString;
}