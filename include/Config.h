#ifndef CONFIG_H
#define CONFIG_H

#include <array>
#include <thread>
#include "ProcessConfig.h"

union CrosshairColor
{
    uint32_t value;
    uint8_t rgb[4];
};

class CrosshairConfig
{

private:
    std::array<uint8_t, 4> quitKeys;
    std::array<uint8_t, 4> toggleKeys;
    std::array<uint8_t, 4> resetKeys;
    uint8_t super_key;
    CrosshairColor crosshairColor;
    CrosshairColor textColor;
    std::thread th;
    std::unique_ptr<ProcessConfig> defaultConfig;
    json processes;

    void loadConfig(int w, int h);
    void saveConfig();

public:
    CrosshairConfig(int w, int h);
    ~CrosshairConfig();

    bool isQuitDown();
    bool isToggleDown();
    bool isResetDown();
    bool isSuperDown();

    int calcX();
    int calcY();

    inline CrosshairColor getCrosshairColor() { return crosshairColor; }
    inline CrosshairColor getTextColor() { return textColor; }
};

#endif // CONFIG_H