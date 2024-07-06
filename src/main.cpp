#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "ConfigReader.h"

const char* CONFIG_FILE = "/config.txt";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 32400;  // 日本時間の場合（UTC+9）
const int   daylightOffset_sec = 0;

String node_domain;
String api_url;
String ssid;
String password;
ConfigReader configReader(CONFIG_FILE);

unsigned long lastUpdateTime = 0;
long lastHeight = 0;
int updateInterval = 40000; // 30秒ごとに更新
int yellowThreshold = 40000; // 30秒
int redThreshold = 300000; // 5分

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

void updateDisplay(long height, int color) {
  M5.Display.clear(color);
  M5.Display.drawRect(10, 10, M5.Display.width() - 20, M5.Display.height() - 20, WHITE);
  
  // 背景色に応じて文字色を設定
  int textColor = (color == YELLOW) ? BLACK : WHITE;
  M5.Display.setTextColor(textColor);

  // ノードドメインを中央寄せで表示
  M5.Display.setTextSize(1);  // ドメイン表示用に文字サイズを小さくする
  M5.Display.setCursor((M5.Display.width() - M5.Display.textWidth(node_domain)) / 2, 20);
  M5.Display.println(node_domain);

  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 60);
  M5.Display.println("Height:");
  M5.Display.setCursor(20, 90);
  M5.Display.println(height);

  // 最終更新時刻を表示
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
    M5.Display.setCursor(20, 120);
    M5.Display.println("Last update:");
    M5.Display.setCursor(20, 150);
    M5.Display.println(timeString);
  } else {
    M5.Display.setCursor(20, 120);
    M5.Display.println("Time sync failed");
  }
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
  node_domain = config["node_domain"];
  api_url = config["api_url"];

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Display.println("Connecting to WiFi...");
  }
  M5.Display.println("Connected to WiFi");

  // NTPサーバーから時刻を取得
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  delay(1000);
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastUpdateTime >= updateInterval) {
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
          long height = doc["height"];
          
          if (height > lastHeight) {
            lastHeight = height;
            lastUpdateTime = currentTime;
            updateDisplay(height, BLACK);
          } else {
            unsigned long timeSinceLastUpdate = currentTime - lastUpdateTime;
            if (timeSinceLastUpdate >= redThreshold) {
              updateDisplay(lastHeight, RED);
            } else if (timeSinceLastUpdate >= yellowThreshold) {
              updateDisplay(lastHeight, YELLOW);
            }
          }
        }
      }
      else {
        M5.Display.println("Error on HTTP request");
      }
      http.end();
    }
  }
  
  delay(1000); // 1秒ごとにチェック
}