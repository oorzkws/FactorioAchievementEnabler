# Factorio Achievement Enabler

## Installation
Grab the latest from [releases](/../../releases) and place the version.dll in the same folder as your Factorio.exe (Factorio\bin\x64). 

If you use Steam, add `cmd /c start "" %command%` as the launch options under the game properties.

![image](https://user-images.githubusercontent.com/65210810/190045080-6ef99754-1c7c-4064-b51e-8cefe155660e.png)

## Notes
This won't enable achievements on saves where cheats or console commands have been used, it only covers mods. Patches just pulled from IDA.

If it doesn't work on the current version, please file an [issue](/../../issues/new) if there isn't one already.
You can include a log by running `path/to/Factorio.exe > path/to/output.log`.

**Please do not submit any crash reports to the Factorio team with this active. If you think you have found a base-game crash, verify with this patch removed before submitting a possibly invalid bugreport.**

Built with CMake, VC++ configs might not work. For some reason (I don't understand CMake) the `DetourXS` directory needs to be copied to the build dir.

# GNU/Linux
The method used by this project (byte patch via version.dll) is Windows specific. Builds will not work on GNU/Linux unless cross-compiled and will not apply to Factorio's native GNU/Linux port.

There is a Linux version by a different maintainer [here](https://github.com/UnlegitSenpaii/FAE_Linux)

# License

Released under the MIT license - http://opensource.org/licenses/MIT

Template thanks to [blaquee/dll-hijack](/../../../../../blaquee/dll-hijack)
