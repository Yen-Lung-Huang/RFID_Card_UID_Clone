// 匯入MFRC522函式庫
#include <MFRC522.h>

// 定義感測器和卡片的資料結構
MFRC522 mfrc522(10, 9); // 建立一個MFRC522物件，參數分別為RST和SS腳位
MFRC522::MIFARE_Key key; // 建立一個MIFARE_Key物件，用於存放卡片的密鑰

// 初始化函數
void setup() {
  pinMode (7,OUTPUT); // 設定7號腳位為輸出模式，用於控制綠色LED
  Serial.begin(115200); // 開啟序列埠，設定傳輸速率為115200
  SPI.begin(); // 開啟SPI通訊
  mfrc522.PCD_Init(); // 初始化感測器
  Serial.println("感測器已就緒"); // 在序列埠輸出訊息

  // 設定卡片的密鑰，這裡使用預設的密鑰FF FF FF FF FF FF
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

// 主要迴圈函數
void loop() {
  byte bufferATQA[2]; // 建立一個陣列，用於存放卡片的回應資料
  byte bufferSize = sizeof(bufferATQA); // 取得陣列的大小

  // 使用mfrc522.PICC_RequestA()函數向卡片發送請求命令，並將回應資料存入bufferATQA陣列中
  MFRC522::StatusCode result = mfrc522.PICC_RequestA(bufferATQA, &bufferSize);

  // 判斷回應結果是否成功
  if (result == MFRC522::STATUS_OK) {
    Serial.println("偵測到卡片"); // 在序列埠輸出訊息
    digitalWrite(7,HIGH); // 點亮綠色LED

    // 如果偵測到卡片，就繼續執行以下的程式碼，這裡可以根據你的需求來修改或增加功能
    // ...

    // 在使用mfrc522.PICC_HaltA()函數之前，先判斷卡片是否仍然在感測器上
    result = mfrc522.PICC_RequestA(bufferATQA, &bufferSize);
    if (result != MFRC522::STATUS_OK) {
      // 如果沒有偵測到卡片，就表示卡片已經被移除了，才使用mfrc522.PICC_HaltA()函數讓卡片進入休眠狀態
      mfrc522.PICC_HaltA();
    }
  }
  else {
    Serial.println("沒有偵測到卡片"); // 在序列埠輸出訊息
    digitalWrite(7,LOW); // 熄滅綠色LED
  }
}
