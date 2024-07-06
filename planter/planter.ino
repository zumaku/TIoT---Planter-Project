#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// Replace with your network credentials
//const char* ssid = "ABDUL RAHMAN";
//const char* password = "58150812";
//const char* ssid = "Pakdiyaa2";
//const char* password = "belikodata";
const char* ssid = "Bagi2 Rezeki";
const char* password = "88888888";

// Replace with your Telegram BOT API token
const char* BOTtoken = "7479955281:AAEVEQwbw8high8CmAlub9Z8oam2Z7Um_eY";

// Replace with your Telegram chat ID
const char* chat_id = "6228744854";

// Define the analog pin
const short soilMoisturePin1 = 33;
const short soilMoisturePin2 = 32;
const short trigRelay = 21;

// Mode definitions
enum Mode { ACTIVE, LAZY };
Mode currentMode = LAZY;
bool startSiram = false;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  client.setInsecure(); // Disable SSL certificate verification

  pinMode(soilMoisturePin1, INPUT);
  pinMode(soilMoisturePin2, INPUT);
  pinMode(trigRelay, OUTPUT);

  digitalWrite(trigRelay, LOW);
}

void loop() {
  if (millis() - lastTime > timerDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    if (currentMode == ACTIVE) {
      autoWater();
    } else if (currentMode == LAZY) {
      lazyWater();
    }

    lastTime = millis();
  }
}

void autoWater() {
  short sensorValue1 = analogRead(soilMoisturePin1);
  short sensorValue2 = analogRead(soilMoisturePin2);

  Serial.println("========== [ACTIVE MODE] ==========");
  Serial.print("Soil Moisture Sensor Value 1: ");
  Serial.println(sensorValue1);
  Serial.print("Soil Moisture Sensor Value 2: ");
  Serial.println(sensorValue2);

  if(sensorValue1 < 3000 || sensorValue2 < 3000) {         
    digitalWrite(trigRelay, HIGH);
    Serial.println("========== Pompa Menyala ==========");
  } else {
    digitalWrite(trigRelay, LOW);
    Serial.println("========== Pompa Mati ==========");
  }
}

void lazyWater() {
  short sensorValue1 = analogRead(soilMoisturePin1);
  short sensorValue2 = analogRead(soilMoisturePin2);

  Serial.println("========== [LAZY MODE] ==========");
  Serial.print("Soil Moisture Sensor Value 1: ");
  Serial.println(sensorValue1);
  Serial.print("Soil Moisture Sensor Value 2: ");
  Serial.println(sensorValue2);

  if(sensorValue1 < 3000 || sensorValue2 < 3000) {         
    String message = "Tanah kering! Butuh penyiraman.";
    bot.sendMessage(chat_id, message, "");
    Serial.println("========== Tanah Kering ==========");
    
    if (startSiram) {
      digitalWrite(trigRelay, HIGH);
      Serial.println("========== Pompa Menyala ==========");
      
      if (sensorValue1 > 3000 && sensorValue2 > 3000) {
        digitalWrite(trigRelay, LOW);
        String message = "Tanah sudah basah.";
        bot.sendMessage(chat_id, message, "");
        startSiram = false;
        Serial.println("========== Pompa Mati ==========");
      }
    }
  } else {
    digitalWrite(trigRelay, LOW);
    Serial.println("========== Tanah Basah ==========");
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/setActive") {
      currentMode = ACTIVE;
      bot.sendMessage(chat_id, "Mode diatur ke ACTIVE", "");
    } else if (text == "/setLazy") {
      currentMode = LAZY;
      bot.sendMessage(chat_id, "Mode diatur ke LAZY", "");
    } else if (text == "/checkMode") {
      String modeStatus = (currentMode == ACTIVE) ? "Mode ACTIVE" : "Mode LAZY";
      bot.sendMessage(chat_id, "Current mode: " + modeStatus, "");
    } else if (text == "/check") {
      short sensorValue1 = analogRead(soilMoisturePin1);
      short sensorValue2 = analogRead(soilMoisturePin2);
      String message = "Soil Moisture Sensor Value 1: " + String(sensorValue1) + "\n" +
                       "Soil Moisture Sensor Value 2: " + String(sensorValue2);
      bot.sendMessage(chat_id, message, "");
    } else if (text == "/startSiram") {
      startSiram = true;
      bot.sendMessage(chat_id, "Penyiraman dimulai", "");
    } else if (text == "/stopSiram") {
      startSiram = false;
      bot.sendMessage(chat_id, "Penyiraman dimulai", "");
    }
  }
}
