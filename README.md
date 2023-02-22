# Crosshair

Creates a crosshair in the middle of the primary screen

Written in WinForms (C#), works only on Windows

Probably not the best way to do it, but hey, **it works!**

## Links

* [Keyboard Shortcuts](#keyboardshortcuts)
* [Configuration](#configuration)
* [Installation](#installation)
* [TODO](#todo)

# Keyboard Shorcuts	
There is some shortcut to know before using this app

	RightAlt + LeftArrow 	=> 	Moves the crossair left
	RightAlt + UpArrow 		=> 	Moves the crossair up
	RightAlt + RightArrow 		=> 	Moves the crossair right
	RightAlt + DownArrow 		=> 	Moves the crossair down
	RightAlt + Home			=> 	Brings the crossair back in the center

	Ctrl + RightAlt + P     		=> 	Toggles the crossair
	Ctrl + LeftAlt + LeftShift + Q 	=>	Exits the application

# Configuration
There is a config.json file in the root directory that can be customized
The file of an array of objects with the following properties

	{
		"ProcessName": 	string,			// Name of the process without the '.exe'
		"WidthRule": 	string | null,		// Rule (%w => width) of the position in X of the crosshair
		"HeightRule": 	string | null,		// Rule (%h => height) of the position in Y of the crosshair
		"X":			number, 	// Exact position in X of the crosshair
		"Y":			number		// Exact position in Y of the crosshair
	}

**If X and Y are unset or set to a value >= 0, `WidthRule` and `HeightRule` are ignored** 

An example with csgo is available in the config.json file

# Installation

* Download the prebuilt version from the [realease tab](#)

Or

* Download the source code and build it yourself with etiher Visual Studio or this command

		dotnet publish -c Release -r win-x64 /p:PublishSingleFile=true 
	This will generate all the required files in the `bin\Release\net6.0-windows\win-x64\publish` folder

# TODO

1. Fix system tray icon context menu, currently lagging for some reasons
2. Create a better/easier configuration interface
4. Other general fixes
