#include <Windows.h>

#include <algorithm>
#include <filesystem>
#include <map>

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include "Config.h"
#include "ConfigException.hpp"
#include "luaFunctions.h"

#define ISKEYDOWN(key) ((GetAsyncKeyState(key) & 0x8000) == 0x8000)

constexpr const char *CONFIG_FILE = "config.lua";

const std::map<std::string, uint8_t> KEY_MAP = {
    {"ralt", VK_RMENU},     {"lalt", VK_LMENU},     {"alt", VK_MENU},
    {"rctrl", VK_RCONTROL}, {"lctrl", VK_LCONTROL}, {"ctrl", VK_CONTROL},
    {"lshift", VK_LSHIFT},  {"rshift", VK_RSHIFT},  {"shift", VK_SHIFT},
    {"home", VK_HOME}};

struct LuaStateDestructor
{
    void operator()(lua_State *L) { lua_close(L); }
};

uint8_t parse_key(const char *key)
{
    return KEY_MAP.contains(key) ? KEY_MAP.at(key) : (*key) & 0xDF;
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
    }
    else
    {
        this->loadConfig(w, h);
    }
}

CrosshairConfig::~CrosshairConfig()
{
    ProcessConfig::stopLoop();
    if (this->th.joinable())
        this->th.join();
}

bool CrosshairConfig::isQuitDown()
{
    // If there is less than 4 keys, there wiil be 0s in the array.
    // The first comparaison makes sure these keys do not have to be pressed
    return std::ranges::all_of(quitKeys, [](uint8_t key)
                               { return key == 0 || ISKEYDOWN(key); });
}

/*
 * Checks that all toggle keys are being pressed down
 */
bool CrosshairConfig::isToggleDown()
{
    // If there is less than 4 keys, there wiil be 0s in the array.
    // The first comparaison makes sure these keys do not have to be pressed
    return std::ranges::all_of(toggleKeys, [](uint8_t key)
                               { return key == 0 || ISKEYDOWN(key); });
}

/*
 * Checks that all reset keys are being pressed down
 */
bool CrosshairConfig::isResetDown()
{
    // If there is less than 4 keys, there wiil be 0s in the array.
    // The first comparaison makes sure these keys do not have to be pressed
    return std::ranges::all_of(resetKeys, [](uint8_t key)
                               { return key == 0 || ISKEYDOWN(key); });
}

/*
 * Checks that the super key is being pressed down
 */
bool CrosshairConfig::isSuperDown() { return ISKEYDOWN(this->super_key); }

/*
 * Helper to load keys from the lua config and parse it into the C++ config
 */
bool loadKeysFromConfig(lua_State *L, std::array<uint8_t, 4> &vec,
                        const char *const globalKey)
{
    // Get the global table
    lua_getglobal(L, globalKey);

    if (lua_istable(L, -1))
    {
        // Start iteration
        lua_pushnil(L);

        // There is a maximum of 4 valid keys, this is used to count this
        int i = 0;
        const char *v;
        while (lua_next(L, -2) && i < 4)
        {
            // All values that aren't strings are not processed
            if (lua_isstring(L, -1))
            {
                // Get the string value
                v = lua_tostring(L, -1);

                // Adds it to the array
                vec[i] = parse_key(v);
                i++;
            }

            lua_pop(L, 1);
        }

        // Remove table from stack
        lua_pop(L, 1);

        // Fills the rest of the array with 0, if applicable
        for (; i < 4; i++)
        {
            vec[i] = 0;
        }

        return true;
    }

    // If there is no table, or this variable is not a table,
    // nothing is loaded and it is flagged as so
    return false;
}

void CrosshairConfig::loadConfig(int w, int h)
{
    // Automatically managed lua state pointer
    auto lpL = std::unique_ptr<lua_State, LuaStateDestructor>(luaL_newstate());

    // Too lazy to write .get() everywhere.
    // This is probably not the safest way to handle things, but as long as
    // there is no pointer arithmetics or it is not freed, it should be fine
    lua_State *L = lpL.get();

    luaL_openlibs(L);

    // Register our custom functions before opening the config file,
    registerCustomFunctions(L);

    // The file should exists at this point, since it was checked in the
    // constructor. If it fails, it is probably a syntax error in the config
    if (luaL_dofile(L, CONFIG_FILE) != LUA_OK)
    {
        throw ConfigException("Could not load config");
    }

    // Loads the series of key from the config to the specified array, or a
    // default set of keys if none is present
    if (!loadKeysFromConfig(L, this->quitKeys, "QuitKeys"))
    {
        // Default keys
        quitKeys[0] = VK_LCONTROL;
        quitKeys[1] = VK_LSHIFT;
        quitKeys[2] = VK_LMENU;
        quitKeys[3] = 'Q';
    }

    if (!loadKeysFromConfig(L, this->toggleKeys, "ToggleKeys"))
    {
        // Default keys
        toggleKeys[0] = VK_RMENU;
        toggleKeys[1] = 'P';
        toggleKeys[2] = 0;
        toggleKeys[3] = 0;
    }

    if (!loadKeysFromConfig(L, this->resetKeys, "ResetKeys"))
    {
        // Default keys
        resetKeys[0] = VK_RCONTROL;
        resetKeys[1] = VK_RMENU;
        resetKeys[2] = VK_HOME;
        resetKeys[3] = 0;
    }

    // Load super key
    lua_getglobal(L, "SuperKey");

    if (lua_isstring(L, -1))
    {
        this->super_key = parse_key(lua_tostring(L, -1));
    }
    else if (lua_isinteger(L, -1))
    {
        // In case the key is a char code (Ex: 'A' => 65)
        this->super_key = static_cast<int>(lua_tointeger(L, -1));
    }
    else
    {
        // We need a super key for navigating the crosshair
        throw ConfigException("No super key in config! Please set a super key "
                              "in the config.lua file");
    }

    // Pops the result
    lua_pop(L, 1);

    // Load crosshair color
    lua_getglobal(L, "CrosshairColor");

    if (lua_isinteger(L, -1))
    {
        this->crosshairColor.value = (int)lua_tointeger(L, -1);
    }
    else
    {
        // Default value
        this->crosshairColor.value = 0xFFFFFF;
    }

    // Pops the result
    lua_pop(L, 1);

    // Load text color
    lua_getglobal(L, "TextColor");

    if (lua_isinteger(L, -1))
    {
        this->textColor.value = (int)lua_tointeger(L, -1);
    }
    else
    {
        // Default value
        this->textColor.value = 0xFFFFFF;
    }

    // Pops the result
    lua_pop(L, 1);

    // Load processes
    lua_getglobal(L, "Processes");

    if (lua_istable(L, -1))
    {
        ProcessConfig *cfg;

        // Start iteration
        lua_pushnil(L);

        // Key
        const char *k;

        // Loop thru array
        while (lua_next(L, -2))
        {
            cfg = new ProcessConfig(w, h);

            // Loop thru object kvp
            lua_pushnil(L);
            while (lua_next(L, -2))
            {
                k = lua_tostring(L, -2);

                // There is no fixed order for the keys, so we compare to
                // available keys
                if (strcmp(k, "processName") == 0)
                {
                    cfg->processName = std::string(lua_tostring(L, -1));
                }
                else if (strcmp(k, "X") == 0)
                {
                    // Can be either a literal number,
                    if (lua_isinteger(L, -1))
                    {
                        cfg->x = std::max(0, (int)lua_tointeger(L, -1));
                    }
                    // or a function that returns a number
                    else if (lua_isfunction(L, -1))
                    {
                        // The function is called with the width parameter
                        lua_pushinteger(L, w);
                        lua_call(L, 1, 1);
                        cfg->x = std::max(0, (int)lua_tointeger(L, -1));
                    }
                }
                else if (strcmp(k, "Y") == 0)
                {
                    // Can be either a literal number,
                    if (lua_isinteger(L, -1))
                    {
                        cfg->y = std::max(0, (int)lua_tointeger(L, -1));
                    }
                    // or a function that returns a number
                    else if (lua_isfunction(L, -1))
                    {
                        // The function is called with the height parameter
                        lua_pushinteger(L, h);
                        lua_call(L, 1, 1);
                        cfg->y = std::max(0, (int)lua_tointeger(L, -1));
                    }
                }

                // Clear object value result, but not the key
                lua_pop(L, 1);
            }

            // Clear array value, but keep key for next iteration
            lua_pop(L, 1);

            ProcessConfig::addToConfigs(cfg);
        }

        // Remove table from stack
        lua_pop(L, 1);

        // Start a thread scanning for processes
        this->th = std::thread(ProcessConfig::scanForProcesses);
    }
}

/*
 * If there is a loaded config, use this to get the x position,
 * otherwise, get the one from the default config (w / 2)
 */
int CrosshairConfig::getX()
{
    ProcessConfig::cfgMutex.lock();

    ProcessConfig *cfg = ProcessConfig::activeConfig != nullptr
                             ? ProcessConfig::activeConfig.get()
                             : defaultConfig.get();
    int x = cfg->x;

    ProcessConfig::cfgMutex.unlock();

    return x;
}

/*
 * If there is a loaded config, use this to get the y position,
 * otherwise, get the one from the default config (h / 2)
 */
int CrosshairConfig::getY()
{
    ProcessConfig::cfgMutex.lock();

    ProcessConfig *cfg = ProcessConfig::activeConfig != nullptr
                             ? ProcessConfig::activeConfig.get()
                             : defaultConfig.get();
    int y = cfg->y;

    ProcessConfig::cfgMutex.unlock();

    return y;
}
