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