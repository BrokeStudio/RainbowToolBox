#include "IconsFontAwesome6.h"

// #include "AppLog.h"
// #include "Ini.h"
#include "Settings.h"

namespace Settings
{

  constexpr ImVec4 colorWarning = ImVec4(1.0f, 0.734f, 0.189f, 1.0f);

  t_Settings settings = {
    "dark", // theme
    1,      // Roboto Mono Regular as default font
    // true,                            // save_on_exit
    false // imgui_demo
  };

  /**
   * @brief Applies one of the built-in ImGui color themes.
   * @param style_idx Theme index selected by the user.
   */
  void set_theme(int style_idx)
  {
    switch(style_idx) {
      case 0:
        settings.theme = "dark";
        ImGui::StyleColorsDark();
        break;
      case 1:
        settings.theme = "light";
        ImGui::StyleColorsLight();
        break;
      case 2:
        settings.theme = "classic";
        ImGui::StyleColorsClassic();
        break;
    }
  }

  // /**
  //  * @brief Set settings property
  //  *
  //  * @param key
  //  * @param value
  //  */
  // void set_property(const std::string& key, const std::string& value)
  // {
  //   if(key == "theme") {
  //     if(value == "dark") {
  //       set_theme(0);
  //     } else if(value == "light") {
  //       set_theme(1);
  //     } else if(value == "classic") {
  //       set_theme(2);
  //     }
  //   } else if(key == "font") {
  //     int font;
  //     try {
  //       font = std::stoi(value);
  //     } catch(const std::exception& e) {
  //       font = 0;
  //     }
  //     settings.font = (font == 0 || font == 1) ? font : 0;
  //   } /* else if(key == "save_on_exit") {
  //      settings.save_on_exit = value == "true" ? true : false;
  //    }*/
  //#if defined(_DEBUG)
  //   else if(key == "imgui_demo") {
  //     settings.imgui_demo = value == "true" ? true : false;
  //   }
  //#endif
  //   else {
  //     APP_LOG(LogTypes_Warning, "[SETTINGS] Don't know what to do with: " + key + "=" + value);
  //   }
  // }

  /** @brief Renders the application settings panel. */
  void render()
  {
    ImGui::SeparatorText("Settings");
    ImGui::BeginChild("Settings");

    if(ImGui::BeginTable("rom_dump_table", 2, ImGuiTableFlags_SizingStretchProp)) {
      // Setup table columns sizes
      ImVec2 text_max_size = ImGui::CalcTextSize("Firmware update script");
      ImGui::TableSetupColumn("one", ImGuiTableColumnFlags_WidthFixed, text_max_size.x);
      ImGui::TableSetupColumn("two", ImGuiTableColumnFlags_WidthStretch);

      // theme
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("Theme");
      ImGui::TableSetColumnIndex(1);
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      static int style_idx = 0;
      if(ImGui::Combo("##settings_theme", &style_idx, "Dark\0Light\0Classic\0")) {
        set_theme(style_idx);
      }

      // font
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("Font");
      ImGui::TableSetColumnIndex(1);
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      ImGui::Combo("##settings_font", &settings.font, "ImGui default font\0Roboto Mono Regular\0Rubik Regular\0");

      // // save settings on exit
      // ImGui::TableNextRow();
      // ImGui::TableSetColumnIndex(0);
      // ImGui::TextUnformatted("Save settings on exit");
      // ImGui::TableSetColumnIndex(1);
      // ImGui::Checkbox("##seting_save_on_exit", &settings.save_on_exit);

#if defined(_DEBUG)
      // ImGui demo window
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("ImGui demo window");
      ImGui::TableSetColumnIndex(1);
      ImGui::Checkbox("##settings_imgui_demo_window", &settings.imgui_demo);
#endif

      ImGui::EndTable();
    }

    // ImGui::NewLine();
    // if(ImGui::Button("Save settings")) {
    //   Ini::save();
    // }

    ImGui::EndChild();
  }
}
