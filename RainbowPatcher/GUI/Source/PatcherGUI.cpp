#include <algorithm>
#include <filesystem>

// imgui
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileBrowser.h"
#include "imgui_helpers.h"

// Patcher
#include <RainbowPatcher/PatchBuilder.h>

// FontAwesome
#include "IconsFontAwesome6.h"

using namespace imgui_addons;

namespace Patcher
{
  std::string file_a;
  std::string file_b;
  std::string file_patch;
  bool show_success_dialog = false;
  RomDiff romDiff;
  ImGuiFileBrowser file_dialog;

  /**
   * @brief Linear interpolation between two vectors
   *
   * @param a source vector
   * @param b destination vector
   * @param t interpolation factor
   * @return ImVec4
   */
  ImVec4 LerpImVec4(const ImVec4& a, const ImVec4& b, float t)
  {
    return ImVec4(
      a.x + (b.x - a.x) * t,
      a.y + (b.y - a.y) * t,
      a.z + (b.z - a.z) * t,
      a.w + (b.w - a.w) * t);
  }

  struct BrowseResult
  {
    bool InputTextChanged;
    bool ButtonClicked;
  };

  /**
   * @brief Custom widget (text input + button) for browsing files
   *
   * @param label widget label
   * @param file destination string
   * @param start_fade start a fade on the text input (used when drag & droping file in the app)
   */
  BrowseResult Browse(const char* label, std::string& file, bool start_fade)
  {
#define BROWSE_TEXT "Browse..."

    BrowseResult result = {};

    // static float t = 0.0f; // interpolation factor
    // static bool fade = false;

    ImGuiStyle& style = ImGui::GetStyle();

    const float browse_text_size = ImGui::CalcTextSize(BROWSE_TEXT).x;

    ImGui::PushID(label);
    ImGui::SetNextItemWidth(IM_MAX(1.0f, ImGui::CalcItemWidth() - browse_text_size - style.FramePadding.x * 2.0f));

    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID fade_id = ImGui::GetID("fade");
    ImGuiID fade_time_id = ImGui::GetID("fade_time");

    bool fade = storage->GetBool(fade_id, false);
    float fade_time = storage->GetFloat(fade_time_id, 0.0f);

    const float fade_duration = 1.0f;

    if(start_fade) {
      fade = true;
      fade_time = 0.0f;
    }

    if(fade) {
      fade_time += ImGui::GetIO().DeltaTime;

      if(fade_time >= fade_duration) {
        fade_time = fade_duration;
        fade = false;
      }
    }

    storage->SetBool(fade_id, fade);
    storage->SetFloat(fade_time_id, fade_time);

    bool colors_pushed = false;

    if(fade) {
      float t = fade_time / fade_duration;

      const ImVec4 fade_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
      ImVec4 frame_color = LerpImVec4(fade_color, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg), t);
      ImVec4 text_color = LerpImVec4(fade_color, ImGui::GetStyleColorVec4(ImGuiCol_Text), t);

      ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_color);
      ImGui::PushStyleColor(ImGuiCol_Text, text_color);
      colors_pushed = true;
    }

    if(start_fade) {
      ImGui::ClearActiveID();
    }
    result.InputTextChanged = ImGui::InputText(label, &file);

    if(colors_pushed) {
      ImGui::PopStyleColor(2);
    }

    const ImVec2 backup_frame_padding = style.FramePadding;
    style.FramePadding.x = style.FramePadding.y;
    ImGui::SameLine(0, style.ItemInnerSpacing.x);
    result.ButtonClicked = ImGui::Button(BROWSE_TEXT);
    style.FramePadding = backup_frame_padding;

    ImGui::PopID();

    return result;
  }

  /** @brief Recomputes the comparison displayed by the GUI. */
  void update_comparison()
  {
    if(file_a.empty() || file_b.empty()) {
      romDiff.clear();
      return;
    }

    romDiff = compare(file_a, file_b);
  }

  /** @brief Resets the patcher GUI state. */
  void init()
  {
    file_a.clear();
    file_b.clear();
    file_patch.clear();
    show_success_dialog = false;
    romDiff.clear();
  }

  /**
   * @brief Renders the patch-builder interface.
   * @param droppedFilename File dropped on the application during this frame.
   * @param scale Current interface scale.
   */
  void render(const std::string& droppedFilename, float scale)
  {
    bool file_A_fade = false;
    bool file_B_fade = false;
    bool file_patch_fade = false;

    auto select_file_A = [](std::string selected_path) {
      file_a = selected_path;
      update_comparison();
    };

    auto select_file_B = [&file_patch_fade](std::string selected_path) {
      file_b = selected_path;
      if(file_patch.empty()) {
        std::filesystem::path default_path = file_b;
        default_path.replace_extension(".rnbw");
        file_patch = default_path.string();
        file_patch_fade = true;
      }
      update_comparison();
    };

    auto select_file_patch = [](std::string selected_path) {
      file_patch = selected_path;
    };

    if(!droppedFilename.empty()) {
      if(file_a.empty()) {
        file_A_fade = true;
        select_file_A(droppedFilename);
        update_comparison();
      } else if(file_b.empty()) {
        file_B_fade = true;
        select_file_B(droppedFilename);
        update_comparison();
      } else if(file_patch.empty()) {
        file_patch_fade = true;
        select_file_patch(droppedFilename);
      }
    }

    ImGui::SeparatorText("Patch File Builder");
    ImGui::BeginChild("Patcher");
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 popupButtonSize = ImVec2(150, 0);
    ImVec2 dialog_size = ImVec2(
      std::min(viewport->WorkSize.x - 20.0f, 700.0f * scale),
      std::min(viewport->WorkSize.y - 20.0f, 310.0f * scale));

    // File A input
    ImGui::TextUnformatted("ROM A (input, currently installed ROM)");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    BrowseResult file_A_result = Browse("##rom_a", file_a, file_A_fade);
    if(file_A_result.InputTextChanged) {
      update_comparison();
    }
    if(file_A_result.ButtonClicked) {
      ImGui::OpenPopup("Open ROM A");
    }
    if(file_dialog.showFileDialog("Open ROM A", ImGuiFileBrowser::DialogMode::OPEN, dialog_size, ".nes")) {
      select_file_A(file_dialog.selected_path);
    }

    // File B input
    ImGui::Spacing();
    ImGui::TextUnformatted("ROM B (input, new ROM)");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    BrowseResult file_B_result = Browse("##rom_b", file_b, file_B_fade);
    if(file_B_result.InputTextChanged) {
      update_comparison();
    }
    if(file_B_result.ButtonClicked) {
      ImGui::OpenPopup("Open ROM B");
    }
    if(file_dialog.showFileDialog("Open ROM B", ImGuiFileBrowser::DialogMode::OPEN, dialog_size, ".nes")) {
      select_file_B(file_dialog.selected_path);
    }

    // Patch output
    ImGui::Spacing();
    ImGui::TextUnformatted("Patch file (output)");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    BrowseResult file_patch_result = Browse("##patch", file_patch, file_patch_fade);
    if(file_patch_result.InputTextChanged) {
      update_comparison();
    }
    if(file_patch_result.ButtonClicked) {
      ImGui::OpenPopup("Save patch file");
    }
    if(file_dialog.showFileDialog("Save patch file", ImGuiFileBrowser::DialogMode::SAVE, dialog_size, ".rnbw")) {
      select_file_patch(file_dialog.selected_path);
    }

    // Create patch button
    ImGui::Spacing();
    bool can_create = romDiff.is_valid() && romDiff.sector_count() != 0;
    if(!can_create) {
      ImGui::BeginDisabled();
    }
    if(ImGui::Button(ICON_FA_FLOPPY_DISK " Create patch", ImVec2(-FLT_MIN, 0.0f))) {
      if(create_patch(file_a, file_b, file_patch, romDiff)) {
        show_success_dialog = true;
      }
    }
    if(!can_create) {
      ImGui::EndDisabled();
    }

    // Rom diff
    ImGui::Spacing();
    ImGui::SeparatorText("Changed sectors");
    if(file_a.empty() || file_b.empty()) {
      ImGui::TextDisabled("Select ROM A and ROM B to compare them.");
    } else if(romDiff.is_valid()) {
      ImGui::Text("PRG-ROM sectors to update: %u", romDiff.prg_sector_count);
      ImGui::Text("CHR-ROM sectors to update: %u", romDiff.chr_sector_count);
      ImGui::Text("Total sectors: %u", romDiff.sector_count());
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.35f, 1.0f), "%s", romDiff.error_message.c_str());
    }

    if(show_success_dialog && !ImGui::IsPopupOpen("Patch created")) {
      ImGui::OpenPopup("Patch created");
    }

    ImVec2 min_size = { 400.0f, 150.0f };
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(std::max<float>(dialog_size.x, min_size.x), std::max<float>(dialog_size.y, min_size.y)), ImGuiCond_Appearing);
    if(ImGui::BeginPopupModal("Patch created", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::TextUnformatted("Patch successfully created.");
      ImGui::TextWrapped("%s", file_patch.c_str());
      ImGui::Separator();
      float available_width = ImGui::GetContentRegionAvail().x;
      float offset = (available_width - popupButtonSize.x) * 0.5f;
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
      if(ImGui::Button("OK", popupButtonSize)) {
        ImGui::CloseCurrentPopup();
        show_success_dialog = false;
      }
      ImGui::EndPopup();
    }

    ImGui::EndChild();
  }
}
