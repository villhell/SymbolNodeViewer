#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "ConfigReader.h"

const char* CONFIG_FILE = "/config.txt";
const char* api_url = "https://villhell-symbol-mainnet.net:3001/chain/info";

String ssid;
String password;
ConfigReader configReader(CONFIG_FILE);

bool loadWifiConfig() {
  M5.Display.println("Starting WiFi config load...");

  M5.Display.println("Initializing SD card...");
  if (!configReader.begin()) {
    M5.Display.println("SD Card initialization failed!");
    return false;
  }
  M5.Display.println("SD Card initialized.");

  M5.Display.println("Checking if config file exists...");
  if (!SD.exists(CONFIG_FILE)) {
    M5.Display.println("Config file does not exist!");
    return false;
  }
  M5.Display.println("Config file exists.");

  M5.Display.println("Opening config file...");
  File configFile = SD.open(CONFIG_FILE, FILE_READ);
  if (!configFile) {
    M5.Display.println("Failed to open config file!");
    return false;
  }
  M5.Display.println("Config file opened successfully.");

  M5.Display.println("Reading config file contents:");
  while (configFile.available()) {
    String line = configFile.readStringUntil('\n');
    M5.Display.println(line);
    line.trim();
    int separatorIndex = line.indexOf('=');
    if (separatorIndex != -1) {
      String key = line.substring(0, separatorIndex);
      String value = line.substring(separatorIndex + 1);
      if (key == "ssid") {
        ssid = value;
        M5.Display.println("SSID found: " + ssid);
      } else if (key == "password") {
        password = value;
        M5.Display.println("Password found: " + String(password.length()) + " characters");
      }
    }
  }
  
  configFile.close();
  M5.Display.println("Config file closed.");

  if (ssid.length() == 0 || password.length() == 0) {
    M5.Display.println("Invalid SSID or password!");
    return false;
  }
  
  M5.Display.println("WiFi config loaded successfully.");
  return true;
}

void setup() {
  auto cfg = M5.config();
  cfg.external_spk = true;
  cfg.external_spk_detail.omit_atomic_spk = true;
  M5.begin(cfg);
  M5.Display.setTextSize(2);

  // SDカードから設定を読み取る
  if (!configReader.begin()) {
      Serial.println("Failed to initialize SD card!");
      M5.Lcd.println("Failed to initialize SD card!");
      return;
  }
  
  Serial.println("SDカード読み取り成功");

  std::map<String, String> config = configReader.readConfig();
  if (config.empty()) {
      Serial.println("Failed to read config file!");
      M5.Lcd.println("Failed to read config file!");
      return;
  }
  Serial.println("config読み取り成功");

  // 設定を変数に代入
  ssid = config["ssid"];
  password = config["password"];

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Display.println("Connecting to WiFi...");
  }
  M5.Display.println("Connected to WiFi");
  delay(1000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(api_url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (error) {
        M5.Display.println("JSON parsing failed");
      } else {
        // heightを取得
        long height = doc["height"];
        
        // 画面をクリアし、枠を描画
        M5.Display.clear();
        M5.Display.drawRect(10, 10, M5.Display.width() - 20, M5.Display.height() - 20, WHITE);
        
        // データを表示
        M5.Display.setCursor(20, 30);
        M5.Display.println("Chain Height:");
        M5.Display.setCursor(20, 60);
        M5.Display.println(height);
      }
    }
    else {
      M5.Display.println("Error on HTTP request");
    }
    http.end();
  }
  
  delay(10000); // 10秒ごとに更新
}