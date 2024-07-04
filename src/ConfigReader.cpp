#include "ConfigReader.h"

ConfigReader::ConfigReader(const char* filename) : filename(filename) {}

bool ConfigReader::begin() {
    if (!SD.begin(GPIO_NUM_4, SPI, 40000000)) {
        Serial.println("SD Card initialization failed!");
        return false;
    }
    Serial.println("SD Card initialized.");
    return true;
}

std::map<String, String> ConfigReader::readConfig() {
    std::map<String, String> config;
    File file = SD.open(filename);
    if (!file) {
        Serial.println("Failed to open config file!");
        return config;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        int separatorIndex = line.indexOf('=');
        if (separatorIndex != -1) {
            String key = line.substring(0, separatorIndex);
            String value = line.substring(separatorIndex + 1);
            config[key] = value;
        }
    }

    file.close();
    return config;
}
