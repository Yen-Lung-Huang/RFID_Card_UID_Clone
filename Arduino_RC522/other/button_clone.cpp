#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10
#define LED 5
#define sw 6

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte UIDArray[4] = {0x01, 0x02, 0x03, 0x04}; // Default UID

MFRC522::MIFARE_Key key;

String ReadString()
{
  String tmp = "";
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == '\n')
    {
      return tmp;
    }
    tmp += c;
    delay(5); // 沒有延遲的話 UART 串口速度會跟不上Arduino的速度，會導致資料不完整
  }
  return tmp;
}

// 定義一個ReadUID()函數，用來讀取卡片的UID
void ReadUID()
{
  static bool wait_card = false; // 宣告一個靜態變數，用來記錄是否需要顯示"Waiting card..."的訊息
  static String currentUID = ""; // 宣告一個靜態變數，用來記錄當前讀到的卡片的UID
  static unsigned long last_card_time = 0; // 宣告一個靜態變數，用來記錄上一次讀到卡片的時間
  const unsigned long wait_interval = 250; // 定義一個常數，用來設定等待卡片的間隔時間（毫秒）
  
  // 檢查是否有新的卡片
  if (mfrc522.PICC_IsNewCardPresent()) {
    char uidString[9]; // 宣告uidString這個變數，類型為char陣列，大小為9

    byte bufferATQA[2]; // 建立一個陣列，用於存放卡片的回應資料
    byte bufferSize = sizeof(bufferATQA); // 取得陣列的大小

    // 使用mfrc522.PICC_Select()函數向卡片發送選擇命令，並將回應資料存入mfrc522.uid結構中
    
    // 根據卡片的回應資料來設定bufferSize的值
    if (bufferATQA[0] == 0x04) { // 如果卡片是ISO/IEC 14443-4 compliant PICC
      bufferSize = bufferATQA[1] * 8; // bufferSize等於第二個位元組乘以8
    }
    else { // 如果卡片是其他類型的PICC
      bufferSize = bufferATQA[0] * 8; // bufferSize等於第一個位元組乘以8
    }

    MFRC522::StatusCode result = mfrc522.PICC_Select(&(mfrc522.uid), bufferSize);
    if (result != MFRC522::STATUS_OK){ // 如果沒有選擇到卡片
      // Serial.println(F("Failed to select card..."));
      return; // 結束函數
    }

    // 如果選擇到卡片，就顯示卡片的UID
    for (byte i = 0; i < mfrc522.uid.size; i++){
      // 將mfrc522.uid.uidByte[i]的值轉換成16進位的字元，並存入uidString中
      uidString[i * 2] = "0123456789ABCDEF"[mfrc522.uid.uidByte[i] >> 4];
      uidString[i * 2 + 1] = "0123456789ABCDEF"[mfrc522.uid.uidByte[i] & 0x0F];
    }
    uidString[8] = '\0';       // 在字串結尾加上空字元

    if (wait_card == true){
      currentUID = String(uidString);// 將uidString變數轉換成String物件
      Serial.println("Card UID: " + currentUID); // 在序列埠輸出卡片的UID
      wait_card = false; // 將wait_card設為false，表示不需要顯示"Waiting card..."的訊息
    }
    
    last_card_time = millis(); // 將當前的時間（毫秒）賦給last_card_time，作為下一次比較的依據
  }
  else { // 如果沒有新的卡片
    unsigned long current_time = millis(); // 取得當前的時間（毫秒）
    
    // 如果當前的時間減去上一次讀到卡片的時間大於或等於間隔時間，並且wait_card為false
    if (current_time - last_card_time >= wait_interval && wait_card == false) {
      Serial.println(F("Waiting card...")); // 在序列埠輸出訊息
      wait_card = true; // 將wait_card設為true，表示已經顯示過"Waiting card..."的訊息
    }
  }
}

void WriteUID()
{
  Serial.println("Ready to write UID");
  // Look for new cards, and select one if present
  while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
    delay(50);
  }

  // Now a card is selected. The UID and SAK is in mfrc522.uid.

  // Dump UID
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Set new UID
  byte newUidbyte[4] = {};

  newUidbyte[0] = (byte)(0xff & UIDArray[0]);
  newUidbyte[1] = (byte)(0xff & UIDArray[1]);
  newUidbyte[2] = (byte)(0xff & UIDArray[2]);
  newUidbyte[3] = (byte)(0xff & UIDArray[3]);

  if (mfrc522.MIFARE_SetUid(newUidbyte, (byte)4, true))
  {
    Serial.println(F("Wrote new UID to card."));
  }

  // Halt PICC and re-select it so DumpToSerial doesn't get confused
  mfrc522.PICC_HaltA();

  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  // Dump the new memory contents
  Serial.println(F("New UID and contents:"));
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

void setup()
{
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("-----RFID Card cloner ver.1.0-----"));
  pinMode(LED, OUTPUT);
  pinMode(sw, INPUT);
  digitalWrite(LED, LOW);
}

uint8_t button_state = 0; // 用一個布林變數來記錄是否是第一次按下開關

void loop()
{
  if (digitalRead(sw) == HIGH && button_state==0){           // 如果是第一次按下開關，就顯示一次Enter new UID
    Serial.flush(); // wait for transmission to complete
    while (Serial.available() > 0) {
      Serial.read(); // read and discard the byte
    }
    Serial.print(F("Enter new UID (8-digit hexadecimal):")); // 提示使用者輸入新的UID
    button_state += 1;                                       // 將布林變數設為false，避免重複顯示
    }

  if (button_state==1){
    if (Serial.available() > 0){    // 如果是第一次按下開關 & 如果串列埠有東西
      String newUID = ReadString();  // 讀取使用者輸入的字串

      if (newUID.length() == 8){     // 如果字串長度為8
        for (int i = 0; i < 4; i++){ // 將字串轉換成16進位數字，並存入UIDArray中
          char hexChars[3] = {newUID[i * 2], newUID[i * 2 + 1], '\0'};
          UIDArray[i] = strtol(hexChars, NULL, 16);
        }
        digitalWrite(LED, HIGH); // 點亮LED
        WriteUID();              // 寫入新的UID
        digitalWrite(LED, LOW);  // 熄滅LED
        Serial.println(F("UID updated successfully.")); // 提示使用者寫入成功
        button_state = 0;
      }
      else { // 如果字串長度不為8
        Serial.println(F("Invalid UID format. Please enter again.")); // 提示使用者格式錯誤
        // button_state = 1;
      }
      
      delay(1000); // 延遲一秒，避免重複輸入
    }
  }
  
  else if (digitalRead(sw) == LOW && button_state==0) { // 如果沒有按下開關，就進入讀取模式
    ReadUID();
  }
}
