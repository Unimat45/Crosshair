# Crosshair

Creates a crosshair in the middle of the primary screen for helping in games lacking one.

Written in C++ using the SDL2 library, and works only on Windows.

## Links

* [Keyboard Shortcuts](#keyboardshortcuts)
* [Configuration](#configuration)
* [Installation](#installation)

# Keyboard Shorcuts	
Controlling the crosshair is done using keyboard combinations. These key combinations can be customized in the [configuration](#configuration) file, but here are the default values:

| Combination                       | Effect |
| -----------                       | ------ |
| Right Alt + Left Arrow            | Moves the crossair left |
| Right Alt + Up Arrow              | Moves the crossair up |
| Right Alt + Right Arrow           | Moves the crossair right|
| Right Alt + Down Arrow            | Moves the crossair down |
| Right Alt + Home                  | Brings the crossair back in the center |
| Ctrl + Right Alt + P              | Toggles the crossair |
| Ctrl + Left Alt + Left Shift + Q  | Exits the application |

# Configuration
Almost all key combinations can be customized in the [config.lua](./config.lua) file.
Letter keys are represented as a single case insensitive character (Ex: Q) and special keys are represented like this

| Key               | Value  |
| ---               | -----  |
| Right Alt         | ralt   |
| Left Alt          | lalt   |
| Either Alt        | alt    |
| Right Control     | rctrl  |
| Left Control      | lctrl  |
| Either Control    | ctrl   |
| Right Shift       | rshift |
| Left Shift        | lshift |
| Either Shift      | shift  |
| Home              | home   |

Here is a quick exemple of the default config:

```lua
-- This is the key that needs to be held down to move the crosshair around and show the coordonates.
-- This is the only constant that is required in the config
SuperKey = "ralt"

-- These 3 constants are lists of keys needed to execute their respective actions

-- Keys to quit the program
QuitKeys = { "lalt", "lctrl", "lshift", "Q" }

-- Keys to show and hide the crosshair
ToggleKeys = { "ralt", "P" }

-- Keys to reset the position of the crosshair
ResetKeys = { "rctrl", "ralt", "home" }

-- The next 2 constants are for customizing the colors. They are represented as RGB hex values.
CrosshairColor = 0xFFD8D1

-- There is also a RGB method available.
TextColor = RGB(178, 149, 147)

-- This is the process table. It's primary use is for games
-- that does not have the crosshair at the center of the screen.
Processes = {
    {
        -- This is the process name to look for.
        -- It can be found in the task manager's details tab.
        processName = "csgo.exe",

        -- This is the X and Y position of the cursor.
        -- It can either be a number
        X = 687,
        -- or a function returning a number.
        Y = function (h) return (h * 2 / 3) - 99 end
    }
}
```

**The numbers in the default config are not recommendations, i chose them at random**

# Installation

* Download the prebuilt version from the [realease tab](https://github.com/Unimat45/Crosshair/releases)

Or

* Download the source code and build it yourself with CMake

		CMake -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release
        CMake --build build

This will generate all the required files in the `./bin` folder
