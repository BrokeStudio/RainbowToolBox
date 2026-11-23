# File System Explorer

The File System Explorer creates, mounts and modifies RNBWFS images used to represent Rainbow storage devices during development and emulation.

It is available from both the graphical application and the command-line interface.

## Graphical interface

The graphical explorer can:

- create and mount RNBWFS images
- add files and create folders
- rename, move and remove entries
- inspect file contents with the hexadecimal viewer
- extract files to the host file system
- save modified images safely

RNBWFS images can also be mounted by dropping them on the application while the File System Explorer is selected.

## Command-line interface

The `filesystem` command is also available through the shorter `fs` alias:

```console
RainbowToolbox fs create <image.fs>
RainbowToolbox fs list <image.fs>
RainbowToolbox fs add <image.fs> <source-file> [internal-path]
RainbowToolbox fs mkdir <image.fs> <internal-path>
RainbowToolbox fs extract <image.fs> <internal-file> [-o <destination>]
RainbowToolbox fs remove <image.fs> <internal-path> [--recursive]
RainbowToolbox fs validate <image.fs>
```

Run the following command for the complete generated help:

```console
RainbowToolbox fs --help
```

### Create an image

```console
RainbowToolbox fs create game.fs
```

The image file name is used as its drive name. The command refuses to replace an existing file.

### List an image

```console
RainbowToolbox fs list game.fs
```

The complete file-system tree is displayed with file sizes.

### Add a file

Add a file to the image root:

```console
RainbowToolbox fs add game.fs config.bin
```

Add it below an internal folder while keeping its host file name:

```console
RainbowToolbox fs add game.fs level01.bin /levels/world1/
```

Add it with a different internal path and name:

```console
RainbowToolbox fs add game.fs config.bin /data/settings.bin
```

Missing parent folders are created automatically.

### Create a folder

```console
RainbowToolbox fs mkdir game.fs /levels/world1
```

All missing parent folders are created automatically.

### Extract a file

Extract using the internal file name in the current directory:

```console
RainbowToolbox fs extract game.fs /data/config.bin
```

Select a destination path:

```console
RainbowToolbox fs extract game.fs /data/config.bin -o exported-config.bin
```

### Remove an entry

```console
RainbowToolbox fs remove game.fs /data/config.bin
```

A non-empty folder requires the recursive option:

```console
RainbowToolbox fs remove game.fs /levels --recursive
```

### Validate an image

```console
RainbowToolbox fs validate game.fs
```

The command checks the image signature, version, entry structure and data boundaries, then displays its file count, folder count and total file-data size.

## Internal paths

Internal paths:

- always begin with `/`
- use `/` as separator
- use case-insensitive 8.3 file and folder names
- have a maximum encoded length of 247 bytes

Examples:

```text
/config.bin
/levels/world1/map.bin
/gfx/tiles.chr
```

## Safe image updates

Commands that modify an existing image rebuild it through a temporary file. The original image is renamed to a backup before the temporary image takes its place. If finalization fails, the original image is restored.

## RNBWFS file format

The binary image layout, entry encoding, internal path rules and validation requirements are documented separately in the [Rainbow file system image format specification](rainbow-file-system-format.md).
