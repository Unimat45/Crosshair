#include <Windows.h>

#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include "Config.h"
#include "ConfigException.h"

#define ISKEYDOWN(key) ((GetAsyncKeyState(key) & 0x8000) == 0x8000)

constexpr const char *CONFIG_FILE = "config.json";

const std::map<std::string, uint8_t> KEY_MAP = {
    {"ralt", VK_RMENU},
    {"lalt", VK_LMENU},
    {"alt", VK_MENU},
    {"rctrl", VK_RCONTROL},
    {"lctrl", VK_LCONTROL},
    {"ctrl", VK_CONTROL},
    {"lshift", VK_LSHIFT},
    {"rshift", VK_RSHIFT},
    {"shift", VK_SHIFT},
    {"home", VK_HOME}};

const std::map<uint8_t, std::string> REV_KEY_MAP = {
    {VK_RMENU, "ralt"},
    {VK_LMENU, "lalt"},
    {VK_MENU, "alt"},
    {VK_RCONTROL, "rctrl"},
    {VK_LCONTROL, "lctrl"},
    {VK_CONTROL, "ctrl"},
    {VK_LSHIFT, "lshift"},
    {VK_RSHIFT, "rshift"},
    {VK_SHIFT, "shift"},
    {VK_HOME, "home"}};

uint8_t parse_key(std::string key)
{
    return KEY_MAP.contains(key) ? KEY_MAP.at(key) : key.at(0) & 0xDF;
}

std::string rev_parse_key(uint8_t key)
{
    return REV_KEY_MAP.contains(key) ? REV_KEY_MAP.at(key) : std::string(1, static_cast<char>(key));
}

CrosshairConfig::CrosshairConfig(int w, int h)
{
    this->defaultConfig = std::make_unique<ProcessConfig>(w, h);

    if (!std::filesystem::exists(CONFIG_FILE))
    {
        quitKeys[0] = VK_LCONTROL;
        quitKeys[1] = VK_LSHIFT;
        quitKeys[2] = VK_LMENU;
        quitKeys[3] = 'Q';

        toggleKeys[0] = VK_RMENU;
        toggleKeys[1] = 'P';
        toggleKeys[2] = 0;
        toggleKeys[3] = 0;

        resetKeys[0] = VK_RCONTROL;
        resetKeys[1] = VK_RMENU;
        resetKeys[2] = VK_HOME;
        resetKeys[3] = 0;

        super_key = VK_RMENU;
        crosshairColor.value = 0xFFFFFF;
        textColor.value = 0xFFFFFF;

        this->saveConfig();
    }
    else
    {
        this->loadConfig(w, h);
    }
}

CrosshairConfig::~CrosshairConfig()
{
    this->saveConfig();
    ProcessConfig::stopLoop();
    this->th.join();
}

bool CrosshairConfig::isQuitDown()
{
    return std::ranges::all_of(quitKeys, [](uint8_t key)
                               { return key == 0 || ISKEYDOWN(key); });
}

bool CrosshairConfig::isToggleDown()
{
    return std::ranges::all_of(toggleKeys, [](uint8_t key)
                               { return key == 0 || ISKEYDOWN(key); });
}

bool CrosshairConfig::isResetDown()
{
    return std::ranges::all_of(resetKeys, [](uint8_t key)
                               { return key == 0 || ISKEYDOWN(key); });
}

bool CrosshairConfig::isSuperDown()
{
    return ISKEYDOWN(this->super_key);
}

void CrosshairConfig::loadConfig(int w, int h)
{
    json root;

    std::ifstream f(CONFIG_FILE);
    try
    {
        root = json::parse(f);
    }
    catch (...)
    {
        throw ConfigException("Cannot parse config file");
    }
    f.close();

    json quit_keys;

    try
    {
        quit_keys = root.at("quitKeys");
    }
    catch (...)
    {
        throw ConfigException("Invalid quit keys combination");
    }

    size_t quit_size = quit_keys.size();
    for (size_t i = 0; i < 4; i++)
    {
        if (i >= quit_size)
        {
            this->quitKeys[i] = 0;
        }
        else
        {
            this->quitKeys[i] = parse_key(quit_keys[i].template get<std::string>());
        }
    }

    json toggle_keys;

    try
    {
        toggle_keys = root.at("toggleKeys");
    }
    catch (...)
    {
        throw ConfigException("Invalid toggle keys combination");
    }

    size_t toggle_size = toggle_keys.size();
    for (size_t i = 0; i < 4; i++)
    {
        if (i >= toggle_size)
        {
            this->toggleKeys[i] = 0;
        }
        else
        {
            this->toggleKeys[i] = parse_key(toggle_keys[i].template get<std::string>());
        }
    }

    json reset_keys;

    try
    {
        reset_keys = root.at("resetKeys");
    }
    catch (...)
    {
        throw ConfigException("Invalid reset keys combination");
    }

    size_t reset_size = reset_keys.size();
    for (size_t i = 0; i < 4; i++)
    {
        if (i >= reset_size)
        {
            this->resetKeys[i] = 0;
        }
        else
        {
            this->resetKeys[i] = parse_key(reset_keys[i].template get<std::string>());
        }
    }

    try
    {
        this->super_key = parse_key(root.at("superKey").template get<std::string>());
    }
    catch (...)
    {
        throw ConfigException("Invalid super key");
    }

    try
    {
        std::string color = root["crosshairColor"].template get<std::string>();
        crosshairColor.value = std::stoi(color.substr(1), nullptr, 16); // Removes the #
    }
    catch (...)
    {
        throw ConfigException("Invalid crosshair color");
    }

    try
    {
        std::string tcolor = root["textColor"].template get<std::string>();
        textColor.value = std::stoi(tcolor.substr(1), nullptr, 16); // Removes the #
    }
    catch (...)
    {
        throw ConfigException("Invalid text color");
    }

    if (root.contains("processes"))
    {
        processes = root.at("processes");

        for (json cfg : processes)
        {
            ProcessConfig::addToConfigs(new ProcessConfig(cfg, w, h));
        }

        this->th = std::thread(ProcessConfig::scanForProcesses);
    }
}

void CrosshairConfig::saveConfig()
{
    json cfg;

    std::stringstream ss;
    ss << "#" << std::hex << crosshairColor.value;
    cfg["crosshairColor"] = ss.str();

    ss = std::stringstream();
    ss << "#" << std::hex << crosshairColor.value;
    cfg["textColor"] = ss.str();

    cfg["superKey"] = rev_parse_key(this->super_key);

    for (size_t i = 0; i < this->quitKeys.size(); i++)
    {
        if (quitKeys[i])
        {
            cfg["quitKeys"].push_back(rev_parse_key(quitKeys[i]));
        }
    }

    for (size_t i = 0; i < this->toggleKeys.size(); i++)
    {
        if (toggleKeys[i])
        {
            cfg["toggleKeys"].push_back(rev_parse_key(toggleKeys[i]));
        }
    }

    for (size_t i = 0; i < this->resetKeys.size(); i++)
    {
        if (resetKeys[i])
        {
            cfg["resetKeys"].push_back(rev_parse_key(resetKeys[i]));
        }
    }

    cfg["processes"] = processes;

    std::ofstream f(CONFIG_FILE);
    f << cfg.dump(4) << std::endl;
    f.close();
}

int CrosshairConfig::calcX() {
    ProcessConfig *cfg = ProcessConfig::activeConfig != nullptr ? ProcessConfig::activeConfig.get() : defaultConfig.get();

    return cfg->calcX();
}

int CrosshairConfig::calcY() {
    ProcessConfig *cfg = ProcessConfig::activeConfig != nullptr ? ProcessConfig::activeConfig.get() : defaultConfig.get();

    return cfg->calcY();
}
