#ifndef CONFIG_H
#define CONFIG_H

#include "ProcessConfig.h"
#include <array>
#include <thread>

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

    void loadConfig(int w, int h);

  public:
    CrosshairConfig(int w, int h);
    ~CrosshairConfig();

    /*
     * Checks that all quit keys are being pressed down
     */
    bool isQuitDown();

    /*
     * Checks that all toogle keys are being pressed down
     */
    bool isToggleDown();

    /*
     * Checks that all reset keys are being pressed down
     */
    bool isResetDown();

    /*
     * Checks that the super key is being pressed down
     */
    bool isSuperDown();

    /*
     * Gets the x value depending on the loaded config
     */
    int getX();

    /*
     * Gets the y value depending on the loaded config
     */
    int getY();

    /*
     * Gets the crosshair color
     */
    inline CrosshairColor getCrosshairColor() { return crosshairColor; }

    /*
     * Gets the text color
     */
    inline CrosshairColor getTextColor() { return textColor; }
};

#endif // CONFIG_H
