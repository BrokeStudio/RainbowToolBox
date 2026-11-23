# Building Rainbow Toolbox

The Rainbow Toolbox workspace contains the `CLI` and `GUI` applications. Both applications include the shared file-system and patch-builder sources directly.

Project files are generated from [Build.lua](Build.lua) using Premake. Regenerate them after changing the build configuration rather than editing generated project files directly.

Unless otherwise stated, run the commands below from the repository root.

## Common requirements

- A C++20-capable compiler.
- Premake 5, either available as `premake5` on `PATH` or invoked from `Vendor/Binaries/Premake/`.
- Platform-specific GUI dependencies listed below.

The workspace provides three configurations:

| Configuration | Description                                                  |
| ------------- | ------------------------------------------------------------ |
| `Debug`       | Debug runtime, symbols enabled, no optimization              |
| `Release`     | Release runtime, optimization and symbols enabled            |
| `Dist`        | Release distribution, optimization enabled, symbols disabled |

With Make, select them using `config=debug`, `config=release` or `config=dist`. With Visual Studio or MSBuild, use the corresponding capitalized configuration name.

## Windows

### Prerequisites

- Visual Studio with the Desktop development with C++ workload and a Windows SDK.
- Premake with support for the selected Visual Studio generator.

The repository contains SDL2 headers and precompiled x86 and x64 static libraries under `External/SDL2/`.

### Generate and build

From a Visual Studio developer PowerShell, run:

```powershell
.\Vendor\Binaries\Premake\Windows\premake5.exe --file=Build.lua vs2026
msbuild RainbowToolbox.slnx -m -t:Rebuild -p:Configuration=Dist -p:Platform=x64
```

Use `-p:Platform=Win32` for the 32-bit build. You can also open the generated `RainbowToolbox.slnx` in Visual Studio, select the configuration and platform, and build the solution. Set `GUI` as the startup project to run the graphical application.

The `Dist` output directories are:

- `Binaries/windows-x86_64/Dist/RainbowToolbox/`
- `Binaries/windows-x86/Dist/RainbowToolbox/`

## Linux

### Prerequisites

The CI builds on Ubuntu 22.04. For an Ubuntu or Debian development environment:

```sh
sudo apt-get update
sudo apt-get install build-essential clang libsdl2-dev libudev-dev libgl-dev
```

### Generate and build

Run the helper script:

```sh
./Scripts/Setup-Linux.sh
make config=dist
```

Equivalent direct Premake invocation:

```sh
Vendor/Binaries/Premake/Linux/premake5 --cc=clang --file=Build.lua gmake2
make config=dist
```

The distribution is written below `Binaries/linux-x86_64/Dist/`.

## macOS

### Prerequisites

- Xcode Command Line Tools, including Clang and Make.
- Premake for the target architecture.
- An SDL2 framework at `External/SDL2-macOS/SDL2.framework`.

The generated application bundle embeds the SDL2 framework. The minimum configured macOS version is 12.0.

### Generate and build

For Apple Silicon:

```sh
Vendor/Binaries/Premake/macOS/premake5 --cc=clang --arch=arm64 --file=Build.lua gmake
make config=dist
```

For Intel, replace `--arch=arm64` with `--arch=x86_64`.

The helper script uses the default architecture selected by `Build.lua`:

```sh
./Scripts/Setup-macOS.sh
make config=dist
```

The `Dist` build creates `RainbowToolbox.app` below the corresponding `Binaries/<system>-<architecture>/Dist/app/` directory. Standalone application outputs are placed in the adjacent project directories.

## Generated files and output directories

Premake generates platform project files and intermediate Makefiles from:

- [Build.lua](Build.lua);
- [CLI/Build-CLI.lua](CLI/Build-CLI.lua);
- [GUI/Build-GUI.lua](GUI/Build-GUI.lua).

Intermediate object files are written below `Binaries/Intermediates/`. Application outputs are grouped by system, architecture, configuration and project below `Binaries/`.

Use `Dist` when reproducing the directory layout intended for distribution.
