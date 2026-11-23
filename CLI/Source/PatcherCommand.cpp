#include "PatcherCommand.h"

#include <filesystem>
#include <iostream>
#include <string>

#include "termcolor.hpp"

#include <RainbowPatcher/PatchBuilder.h>

namespace Commands
{
  namespace
  {
    struct PatcherOptions
    {
      std::string file_a;
      std::string file_b;
      std::string patch_file;
    };

    PatcherOptions options;

    /**
     * @brief Executes the patch command with the parsed options.
     * @return Process exit code.
     */
    int create_patch()
    {
      if(options.patch_file.empty()) {
        std::filesystem::path default_path = options.file_b;
        default_path.replace_extension(".rnbw");
        options.patch_file = default_path.string();
      } else {
        std::filesystem::path output_path = options.patch_file;
        if(output_path.extension() != ".rnbw") {
          output_path += ".rnbw";
          options.patch_file = output_path.string();
        }
      }

      Patcher::RomDiff romDiff;
      if(!Patcher::create_patch(options.file_a, options.file_b, options.patch_file, romDiff)) {
        std::cerr << termcolor::bright_red << "Error: " << termcolor::reset
                  << romDiff.error_message << std::endl;
        return 1;
      }

      std::cout << termcolor::bright_green << "Patch created: " << termcolor::reset
                << options.patch_file << std::endl
                << "PRG-ROM sectors: " << romDiff.prg_sector_count << std::endl
                << "CHR-ROM sectors: " << romDiff.chr_sector_count << std::endl
                << "Total sectors:   " << romDiff.sector_count() << std::endl;
      return 0;
    }
  }

  /**
   * @brief Registers the patch-builder command in CLI11.
   * @param app Root CLI11 application.
   * @param command_result Receives the command exit code.
   */
  void register_patcher(CLI::App& app, int& command_result)
  {
    CLI::App* patch = app.add_subcommand("patch", "Create a Rainbow patch between two NES 2.0 ROMs.");
    patch->add_option("rom-a", options.file_a, "Currently installed ROM file")
      ->required()
      ->check(CLI::ExistingFile);
    patch->add_option("rom-b", options.file_b, "New ROM file")
      ->required()
      ->check(CLI::ExistingFile);
    patch->add_option("-o,--output", options.patch_file, "Output .rnbw patch file");
    patch->callback([&command_result]() { command_result = create_patch(); });
  }
}
