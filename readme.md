# Factorio Achievement Enabler

## Installation
Grab the latest from [releases](/../../releases) and place it in the same folder as your Factorio.exe (Factorio\bin\x64). In Steam, add `cmd /c start %command%` as the launch options under the game properties.

![image](https://user-images.githubusercontent.com/65210810/175926763-35ef5ca0-0ce9-425e-a3b5-f1ef2bb4ee8c.png)

## Notes

This won't enable achievements on saves where cheats or console commands have been used, it only covers mods. Patches just pulled from IDA.

Updated as of Factorio **1.1.57**. If it doesn't work on the current version, please file an [issue](/../../issues/new) if there isn't one already.
You can include a log by running `path/to/Factorio.exe > path/to/output.log`.

Built with CMake, VC++ configs might not work. 

# License

Released under the MIT license - http://opensource.org/licenses/MIT

Template thanks to [blaquee/dll-hijack](/../../../../../blaquee/dll-hijack)
