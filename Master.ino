// espnow_master.ino
#include <esp_now.h>
#include <WiFi.h>

// Put slave MAC here (copy from slave Serial Monitor)
// Example: "24:6F:28:A1:B2:C3" -> {0x24,0x6F,0x28,0xA1,0xB2,0xC3}
uint8_t data[4] = {0, 0, 0, 0};
uint8_t receiver1MAC[] = {0x44, 0x1d, 0x64, 0xe3, 0xfb, 0x71};
uint8_t receiver2MAC[] = {0x8c, 0xaa, 0xb5, 0xa1, 0x26, 0xf5};
uint8_t receiver3MAC[] = {0x44, 0x1d, 0x64, 0xe4, 0xa7, 0xbc};
uint8_t receiver4MAC[] = {0x08, 0xa6, 0xf7, 0x0d, 0xea, 0x9d};


void OnDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA); // Station mode required for ESP-NOW

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true) delay(1000);
  }

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  // Add Peer 1
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiver1MAC, 6);
  peerInfo.channel = 0; // 0 = use current Wi-Fi channel
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Add Peer 2
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receiver2MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Add Peer 3
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receiver3MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Add Peer 4
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receiver4MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.println("Enter mode: ");
}

void send(){
  esp_now_send(receiver1MAC, &data[0], sizeof(data[0]));
  esp_now_send(receiver2MAC, &data[1], sizeof(data[1]));
  esp_now_send(receiver3MAC, &data[2], sizeof(data[2]));
  esp_now_send(receiver4MAC, &data[3], sizeof(data[3]));
  Serial.print("Sending to ESP1: ");
  Serial.println(data[0]);
  Serial.print("Sending to ESP2: ");
  Serial.println(data[1]);
  Serial.print("Sending to ESP3: ");
  Serial.println(data[2]);
  Serial.print("Sending to ESP4: ");
  Serial.println(data[3]);
}


void loop() {

  if(Serial.available() > 0){
    int val = Serial.parseInt();
    if (val > 0){
      if(val==2) val = 0;   
      for(int i=0; i<4; i++){
        data[i] = val;
      }
    }
  }
  send();
  delay(1000); // send every 1 sec
}
