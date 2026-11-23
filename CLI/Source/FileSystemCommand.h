#pragma once
#ifndef FILE_SYSTEM_COMMAND_H
  #define FILE_SYSTEM_COMMAND_H

  #include "CLI11.hpp"

namespace Commands
{
  void register_file_system(CLI::App& app, int& command_result);
}

#endif
