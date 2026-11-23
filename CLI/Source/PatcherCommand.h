#pragma once
#ifndef PATCHER_COMMAND_H
  #define PATCHER_COMMAND_H

  #include "CLI11.hpp"

namespace Commands
{
  void register_patcher(CLI::App& app, int& command_result);
}

#endif
