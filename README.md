# Rainbow Toolbox

[![Build Status](https://github.com/BrokeStudio/RainbowToolbox/actions/workflows/build/badge.svg)](https://github.com/BrokeStudio/RainbowToolbox/actions?workflow=build)
![GitHub License](https://badgen.net/github/license/BrokeStudio/RainbowToolbox)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)

Rainbow Toolbox provides graphical and command-line tools for developing games and software for [Rainbow](https://github.com/BrokeStudio/rainbow-net) boards.

## Tools

### File System Explorer

Create, mount, inspect and modify Rainbow file system images from the graphical application or command line.

```console
RainbowToolbox fs --help
```

[Read the File System Explorer documentation](docs/file-system-explorer.md).

[Read the `.fs` image format specification](https://github.com/BrokeStudio/rainbow-net/blob/master/NES/rainbow-file-system-format.md).

Rainbow Toolbox loads an image completely into memory before allowing modifications. Modified images are rebuilt into a temporary file and finalized only after the complete new image has been written successfully.

### Patch File Builder

Compare two NES 2.0 ROMs, inspect the changed PRG-ROM and CHR-ROM sectors, and generate a Rainbow cartridge update patch.

```console
RainbowToolbox patch <rom-a.nes> <rom-b.nes> [-o <patch.rnbw>]
```

[Read the Patch File Builder documentation](docs/patch-file-builder.md).

[Read the official `.rnbw` patch format specification](https://github.com/BrokeStudio/rainbow-net/blob/master/NES/rainbow-patch-format.md).

## Graphical application

The graphical application exposes both tools through its main menu.

## Command-line interface

General help and version information are available with:

```console
RainbowToolbox --help
RainbowToolbox --version
```

The CLI provides two command families:

```text
patch         Create a Rainbow update patch.
filesystem    Create and modify RNBWFS images (alias: fs).
```

See the tool-specific documentation above for complete commands, examples and format information.

## Building

See [COMPILING.md](COMPILING.md) for prerequisites and build instructions for Windows, Linux and macOS.

## Third-party software

This project uses:

- [CLI11](https://github.com/CLIUtils/CLI11)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImGuiFileBrowser](https://github.com/gallickgunner/ImGui-Addons)
- [ImGui Memory Editor](https://github.com/ocornut/imgui_club)
- [Premake](https://premake.github.io/)
- [SDL2](https://www.libsdl.org/)
- [termcolor](https://github.com/ikalnytskyi/termcolor)

The GUI embeds the following fonts:

- **Roboto Mono Regular**, licensed under the [SIL Open Font License 1.1](GUI/fonts/RobotoMonoRegular-License.txt)
- **Rubik Regular**, licensed under the [SIL Open Font License 1.1](GUI/fonts/RubikRegular-License.txt)
- **Font Awesome 6 Free**, licensed under the [SIL Open Font License 1.1](GUI/fonts/FontAwesome-License.txt)

## AI usage and limitations

Generative AI tools were used to assist with documentation and code review. All generated content was reviewed and validated by the project maintainers.

## Credits

Developed by Antoine Gohin / Broke Studio.

Special thanks to Charles Ganne for his help during his internship.

## License

Rainbow Toolbox is licensed under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.

## Support and feedback

For bug reports and feature requests, [open an issue](https://github.com/BrokeStudio/RainbowToolbox/issues).

Please include:

- your operating system and application version
- whether the issue affects the GUI or CLI
- the command or operation involved
- steps to reproduce the problem and any error messages

You can also join the [Broke Studio Discord server](https://discord.gg/FffVMAuhTX) for questions and discussion.
