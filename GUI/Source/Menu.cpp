#include "imgui.h"
#include "IconsFontAwesome6.h"

#include "Menu.h"

namespace Menu
{
  int current_selection = 0;

  /** @brief Renders the main navigation menu. */
  void render()
  {
    ImVec2 size = ImVec2(0.0f, 35.0f);
    ImVec2 alignment = ImVec2(0.0f, 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
    ImGui::SeparatorText("Menu");
    ImGui::BeginChild("MenuItems");

    if(ImGui::Selectable(" " ICON_FA_FILE_ZIPPER " File System Explorer", current_selection == 1, 0, size)) {
      current_selection = 1;
    }

    if(ImGui::Selectable(" " ICON_FA_PUZZLE_PIECE " Patch File Builder", current_selection == 2, 0, size)) {
      current_selection = 2;
    }

    if(ImGui::Selectable(" " ICON_FA_WRENCH " Settings", current_selection == 3, 0, size)) {
      current_selection = 3;
    }

    if(ImGui::Selectable(" " ICON_FA_CIRCLE_INFO " About", current_selection == 4, 0, size)) {
      current_selection = 4;
    }

    ImGui::PopStyleVar();
    ImGui::EndChild(); // MenuItems
  }

}
