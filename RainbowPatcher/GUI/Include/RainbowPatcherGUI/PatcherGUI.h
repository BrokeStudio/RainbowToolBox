#pragma once
#ifndef PATCHER_GUI_H
  #define PATCHER_GUI_H

  #include <string>

namespace Patcher
{
  void init();
  void render(const std::string& droppedFilename, float scale);
}

#endif

