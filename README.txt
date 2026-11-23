RAINBOW TOOLBOX
===============

Rainbow Toolbox provides graphical and command-line tools for developing games
and software for Rainbow boards.

This release includes:

- A Rainbow file system explorer and editor
- Tools for creating Rainbow patch files
- A graphical application
- A command-line interface


GETTING STARTED
===============

Launch the graphical application to access the File System Explorer and Patch
File Builder from the main menu.

The command-line interface provides general help and version information:

  RainbowToolbox --help
  RainbowToolbox --version

Its main command families are:

  RainbowToolbox filesystem --help
  RainbowToolbox fs --help
  RainbowToolbox patch --help

Example patch creation:

  RainbowToolbox patch rom-a.nes rom-b.nes -o update.rnbw


IMPORTANT
=========

This is an early experimental release. Back up your files before modifying
them with Rainbow Toolbox.

Rainbow file system images are loaded completely into memory. The 64-bit build
is therefore recommended for large images.


DOCUMENTATION
=============

Project and documentation:

  https://github.com/BrokeStudio/RainbowToolbox

Rainbow project:

  https://github.com/BrokeStudio/rainbow-net


SUPPORT AND FEEDBACK
====================

Report bugs and request features at:

  https://github.com/BrokeStudio/RainbowToolbox/issues

Please include your operating system, whether the problem affects the GUI or
CLI, the operation or command involved, and the steps needed to reproduce it.

Broke Studio Discord:

  https://discord.gg/FffVMAuhTX


LICENSE
-------

Rainbow Toolbox is licensed under the MIT License. See LICENSE.txt for details.

Developed by Antoine Gohin / Broke Studio.
