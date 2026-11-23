# Patch File Builder

The Patch File Builder compares two NES ROMs and creates a `.rnbw` file containing the complete 64 KiB PRG-ROM and CHR-ROM sectors that changed.

The resulting patch can be applied by the Rainbow BootROM or by another implementation of the [Rainbow patch file format](https://github.com/BrokeStudio/rainbow-net/blob/master/NES/rainbow-patch-format.md).

## ROM A and ROM B

- **ROM A** is the version currently installed on the cartridge.
- **ROM B** is the new version that should be installed.

The patch contains sector data from ROM B for every sector that differs from ROM A.

## Input requirements

Both inputs must:

- use a NES 2.0 header, legacy iNES files are rejected
- have identical PRG-ROM and CHR-ROM layouts
- have PRG-ROM and CHR-ROM sizes that are multiples of 64 KiB
- contain no more than 128 sectors per ROM destination
- contain all data declared by their NES 2.0 headers

An optional 512-byte NES trainer is handled when locating the PRG-ROM and CHR-ROM regions. The NES header and trainer are not included in the comparison because the patch format only targets PRG-ROM and CHR-ROM flash sectors.

## Graphical interface

Select ROM A and ROM B from their file fields or drop files on the application while the Patch File Builder is selected.

After both files have been selected, the interface displays:

- the number of changed PRG-ROM sectors
- the number of changed CHR-ROM sectors
- the total number of sectors that will be written

The comparison is recomputed whenever either input path changes. The files are validated and compared again immediately before writing the patch.

Use **Create patch** to select the destination `.rnbw` file.

## Command-line interface

```console
RainbowToolbox patch <rom-a.nes> <rom-b.nes> [-o <patch.rnbw>]
```

Example:

```console
RainbowToolbox patch game-old.nes game-new.nes -o game-update.rnbw
```

If `--output` is omitted, the output path is derived from ROM B by replacing its extension with `.rnbw`.

Run the following command for the complete generated help:

```console
RainbowToolbox patch --help
```

On success, the CLI displays the output path and changed-sector counts:

```text
Patch created: game-update.rnbw
PRG-ROM sectors: 2
CHR-ROM sectors: 1
Total sectors:   3
```

## Comparison behavior

PRG-ROM and CHR-ROM are compared independently in 64 KiB sectors. As soon as a difference is found within a sector, the complete sector from ROM B is added to the patch.

Blocks are written in PRG-ROM order followed by CHR-ROM order. Reserved header bytes are set to zero.

If both ROMs are identical, the comparison reports zero changed sectors and no empty patch is generated.

## Patch writing

The patch is first written to a temporary file. Once every declared sector block has been written successfully, the temporary file replaces the selected output file.

For `N` changed sectors, the resulting size is:

```text
16 + N × (16 + 65536) bytes
```

The 16-byte file header is followed by one 16-byte sector header and exactly 65,536 data bytes for each changed sector.

Refer to the [official format documentation](https://github.com/BrokeStudio/rainbow-net/blob/master/NES/rainbow-patch-format.md) for the normative byte-level specification.
