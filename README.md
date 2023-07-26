# Rainbow Tool Box

[![Build Status](https://github.com/BrokeStudio/RainbowToolBox/workflows/build/badge.svg)](https://github.com/BrokeStudio/RainbowToolBox/actions?workflow=build)
![GitHub License](https://badgen.net/github/license/BrokeStudio/RainbowToolBox)

The **Rainbow Tool Box** is designed to helped developers to create games/software using our Wi-Fi enabled boards named [**Rainbow**](https://github.com/BrokeStudio/rainbow-lib).

The **Rainbow Tool Box** offers:

- [File System Explorer](#file-system-explorer): create/modify your Rainbow file system files.
- [Patch File Builder](#patch-file-builder): create patch file to be used with the Rainbow bootrom or with your own code.

## File System Explorer

When emulating Rainbow boards, we also want to emulate the ESP8266 Flash Memory and/or the on-board SD card. To do this we created a simple file system format contained in a single file. This tool allows you to easily create/modify those files.

### File System File Format

A file system file consists of the following sections, in order:

- Header (7 bytes)
- File header (minimum 9 bytes)
- File data

(_file header_ and _file data_ can be repeated)

### Header format

The format of the header is as follows:

- 0-6: constant $52 $4E $42 $57 $46 $53 $1A ("RNBWFS" followed by MS-DOS end-of-file)
- 7: file format version (only $00 supported for now)

### File header

The format of the file header is as follows:

- 0-2: file separator, constant $46 $3E ("F>")
- 3-6: file data length (big endian)
- 7: file name length
- 8-?: file name

### File data

Raw data, length depends on file content.

## Patch File Builder

**_still WIP/not released_**

If you want to update your cartridge, you may not want to erase the whole flash chip, but just the sectors that need to be updated. This tool allow you to create a patch file, between two ROM files, to be used with the Rainbow bootloader (or with your own code).

## Credits

Developed by Antoine Gohin and Charles Ganne as part of his internship.

[Dear ImGui](https://github.com/ocornut/imgui) is developed by Omar Cornut.

## License

Rainbow Tool Box is licensed under the MIT License, see LICENSE.txt for more information.

## Contact

[![Mail](https://badgen.net/static/Email/contact@brokestudio.fr/black?icon=https%3A%2F%2Fbrokestudio.fr%2Fbs.svg)](mailto:contact@brokestudio.fr)
[![Website](https://badgen.net/static/Website/https:%2F%2Fbrokestudio.fr/F0B92D?icon=https%3A%2F%2Fbrokestudio.fr%2Fbs.svg)](https://brokestudio.fr)  
[![Discord](https://badgen.net/discord/members/FffVMAuhTX?label=Discord&icon=discord)](https://www.github.gg/FffVMAuhTX)
[![Twitter](https://badgen.net/static/twitter/@Broke_Studio?icon=twitter)](https://twitter.com/Broke_Studio)
