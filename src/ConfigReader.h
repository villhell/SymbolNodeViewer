#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <SD.h>
#include <SPI.h>
#include <map>

class ConfigReader {
public:
    ConfigReader(const char* filename);
    bool begin();
    std::map<String, String> readConfig();

private:
    const char* filename;
};

#endif // CONFIG_READER_H
