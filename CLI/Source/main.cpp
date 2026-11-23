#include <iostream>

#include "CLI11.hpp"
#include "FileSystemCommand.h"
#include "PatcherCommand.h"
#include "termcolor.hpp"
#include "version.h"

/**
 * @brief Rainbow Toolbox command-line entry point.
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument values.
 * @return Process exit code.
 */
int main(int argc, char** argv)
{
#if defined(_WIN32)
  // allows for ANSI codes to be output correctly on Windows
  SetConsoleOutputCP(65001);
  HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleMode(hOutput, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

  std::cout << termcolor::bright_yellow
            << "  ___      _      _                _____         _ _             " << std::endl
            << " | _ \\__ _(_)_ _ | |__  _____ __ _|_   _|__  ___| | |__  _____ __" << std::endl
            << " |   / _` | | ' \\| '_ \\/ _ \\ V  V / | |/ _ \\/ _ \\ | '_ \\/ _ \\ \\ /" << std::endl
            << " |_|_\\__,_|_|_||_|_.__/\\___/\\_/\\_/  |_|\\___/\\___/_|_.__/\\___/_\\_\\" << std::endl
            << "                                                                 " << std::endl
            << termcolor::reset
            << "v" << get_app_version_string() << std::endl
            << std::endl;

  // AppLog::log.cliOutput = true;

  CLI::App app { "Tools for Rainbow game and software development." };
  // Internal RNBWFS paths start with '/', so '/option' must not be parsed as
  // a Windows-style command-line option.
  app.allow_windows_style_options(false);
  app.set_version_flag("-v,--version", get_app_version_string());
  app.require_subcommand(1);
  int command_result = 0;

  Commands::register_file_system(app, command_result);
  Commands::register_patcher(app, command_result);

  try {
    app.parse(argc, argv);
  } catch(const CLI::ParseError& error) {
    return app.exit(error);
  }

  return command_result;
}
