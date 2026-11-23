#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "version.h"
#include "SDL_version.h"
#include "About.h"

namespace About
{
  /** @brief Renders the application information panel. */
  void render()
  {
    ImGui::SeparatorText("About");
    ImGui::BeginChild("About");
    ImGui::Text(ICON_FA_RAINBOW " Rainbow Toolbox v%s", get_app_version_string());
    ImGui::Separator();
    ImGui::Text("2024-2026, Broke Studio");
    ImGui::Text("Developed by Antoine Gohin");
    ImGui::Text("Special thanks to Charles Ganne");
    ImGui::Separator();
    ImGui::Text("Powered by");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("Dear ImGui", "https://github.com/ocornut/imgui");
    ImGui::SameLine();
    ImGui::Text("v" IMGUI_VERSION);
    ImGui::Text("by Omar Cornut");
    ImGui::Separator();
    ImGui::Text("SDL2 v%d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
    ImGui::EndChild();
  }

}
