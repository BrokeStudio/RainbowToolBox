#include <cstdint>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

// Rainbow Tool Box version
#include "version.h"

// imgui
#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileBrowser.h"
#include "imgui_memory_editor.h"
#include "imgui_helpers.h"

// Tree Manager
#include "TreeManager.h"

// FontAwesome
#include "IconsFontAwesome6.h"

// #include <thread>

// using namespace std;
using namespace imgui_addons;
// using namespace TreeManager;

enum class COMMAND_ACTIONS
{
  IDLE,
  REFRESH,
  MOUNT_DRIVE,
  ADD_FILE,
  CREATE_DRIVE,
  UNMOUNT_DRIVE,
  MOVE_NODE,
  RENAME_NODE,
  DELETE_NODE,
  VIEW,
  EXTRACT,
};

typedef struct command_t
{
  COMMAND_ACTIONS action = COMMAND_ACTIONS::IDLE;
  // std::string new_value = "";
  char new_value[257] = "";
  node *cur_node = NULL;
} command_t;

// global vars
ImGuiFileBrowser file_dialog;
command_t command;
bool show_demo_window = false;
bool show_error_dialog = false;
bool show_about = false;
bool show_hex_viewer = false;
std::string error_msg = "";

uint8_t *file_data = NULL;
std::string file_data_path = "";
int file_data_size = 0;

// Répertoire de travail initial dans les propriétés du projet : $(ProjectDir)

// Release -> Editeurs de liens / Entrées / Dépendances supplémentaires : kernel32.lib; user32.lib; gdi32.lib; winspool.lib; comdlg32.lib; advapi32.lib; shell32.lib; ole32.lib; oleaut32.lib; uuid.lib; odbc32.lib; odbccp32.lib; % (AdditionalDependencies)

void show_error(TREE_MANAGER_ERRORS e)
{
  // set error message
  error_msg = std::string(tree_manager_errors_text[static_cast<int>(e)]);
  show_error_dialog = true;
}

void reset_command()
{
  command.action = COMMAND_ACTIONS::IDLE;
  // command.new_value = "";
  command.new_value[0] = '\0';
  command.cur_node = NULL;
}

void show_context_menu_popup(node *cur_node, std::string path)
{
  if (ImGui::BeginPopupContextItem("tree popup"))
  {
    // register clicked node
    command.cur_node = cur_node;

    // branch on node type
    if (cur_node->type == NODE_TYPES::ROOT) // root
    {
      if (ImGui::Selectable(ICON_FA_ROTATE " Refresh"))
        command.action = COMMAND_ACTIONS::REFRESH;

      if (ImGui::Selectable(ICON_FA_PLUS " Create drive"))
        command.action = COMMAND_ACTIONS::CREATE_DRIVE;

      if (ImGui::Selectable(ICON_FA_PLUG_CIRCLE_PLUS " Mount drive"))
        command.action = COMMAND_ACTIONS::MOUNT_DRIVE;
    }
    else if (cur_node->type == NODE_TYPES::DRIVE) // drive root
    {
      if (ImGui::Selectable(ICON_FA_PLUS " Add file"))
        command.action = COMMAND_ACTIONS::ADD_FILE;

      if (ImGui::Selectable(ICON_FA_PLUG_CIRCLE_XMARK " Unmount drive"))
        command.action = COMMAND_ACTIONS::UNMOUNT_DRIVE;
    }
    else if (cur_node->type == NODE_TYPES::FOLDER) // folder
    {
      if (ImGui::Selectable(ICON_FA_PLUS " Add file"))
        command.action = COMMAND_ACTIONS::ADD_FILE;

      if (ImGui::Selectable(ICON_FA_PEN " Rename folder"))
      {
        strcpy(command.new_value, cur_node->path.c_str());
        command.action = COMMAND_ACTIONS::RENAME_NODE;
      }

      if (ImGui::Selectable(ICON_FA_XMARK " Delete folder"))
        command.action = COMMAND_ACTIONS::DELETE_NODE;
    }
    else if (cur_node->type == NODE_TYPES::FILE) // file
    {
      if (ImGui::Selectable(ICON_FA_MAGNIFYING_GLASS " View file"))
        command.action = COMMAND_ACTIONS::VIEW;

      if (ImGui::Selectable(ICON_FA_DOWNLOAD " Extract file"))
        command.action = COMMAND_ACTIONS::EXTRACT;

      if (ImGui::Selectable(ICON_FA_PEN " Rename file"))
      {
        strcpy(command.new_value, cur_node->path.c_str());
        command.action = COMMAND_ACTIONS::RENAME_NODE;
      }

      if (ImGui::Selectable(ICON_FA_TRASH_CAN " Delete file"))
        command.action = COMMAND_ACTIONS::DELETE_NODE;
    }
    ImGui::EndPopup();
  }
}

static int FilterFileName(ImGuiInputTextCallbackData *data)
{
  // Uppercase becomes lowercase
  if (data->EventChar >= 'A' && data->EventChar <= 'Z')
  {
    data->EventChar += 'a' - 'A';
    return 0;
  }

  // forbidden characters
  if (data->EventChar > 256 || strchr("<>:\"\\|?*", (char)data->EventChar))
    return 1;

  return 0;
}

// fonction qui affiche l'arbre
void render_tree(node *cur_node, std::string full_path = "")
{
  // init vars
  bool open = false;

  // init table
  ImGui::TableNextRow();
  ImGui::TableNextColumn();

  // handle node types
  switch (cur_node->type)
  {
  case NODE_TYPES::ROOT:

    ImGui::PushID(full_path.c_str());

    // root node is always open
    ImGui::SetNextItemOpen(true);

    // render node
    open = ImGui::TreeNodeEx(cur_node->label.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);

    // handle right click
    ImGui::OpenPopupOnItemClick("tree popup", ImGuiPopupFlags_MouseButtonRight);
    show_context_menu_popup(cur_node, full_path);

    ImGui::PopID();

    // render table
    ImGui::TableNextColumn();
    ImGui::TextDisabled("--");
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(node_types[static_cast<int>(cur_node->type)]);

    // render children if opened
    if (open)
    {
      // add children
      for (size_t i = 0; i < cur_node->children_count; i++)
      {
        std::string tmp_path = "/" + cur_node->children[i]->label + "/";
        render_tree(cur_node->children[i], tmp_path);
      }
      ImGui::TreePop();
    }
    break;
  case NODE_TYPES::DRIVE:
  case NODE_TYPES::FOLDER:
    ImGui::PushID(full_path.c_str());

    // render node
    open = ImGui::TreeNodeEx(cur_node->label.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
    if (cur_node->type == NODE_TYPES::DRIVE)
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
        ImGui::SetTooltip("%s", cur_node->filename.c_str());

    // handle right click
    ImGui::OpenPopupOnItemClick("tree popup", ImGuiPopupFlags_MouseButtonRight);
    show_context_menu_popup(cur_node, full_path);

    ImGui::PopID();

    // handle drag
    if (cur_node->type == NODE_TYPES::FOLDER)
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) // ImGuiDragDropFlags_None // ImGuiDragDropFlags_SourceAllowNullID
      {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("DND_DEMO_CELL", &cur_node, sizeof(cur_node));

        // drag ghost
        ImGui::TextUnformatted(full_path.c_str());
        ImGui::EndDragDropSource();
      }

    // handle drop
    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
      {
        node *src_node = *(node **)payload->Data;
        command.action = COMMAND_ACTIONS::MOVE_NODE;
        command.cur_node = src_node;
        strcpy(command.new_value, (full_path + src_node->path.substr(src_node->path.rfind("/") + 1)).c_str());
      }
      ImGui::EndDragDropTarget();
    }

    // render table
    ImGui::TableNextColumn();
    ImGui::TextDisabled("--");
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(node_types[static_cast<int>(cur_node->type)]);

    // render children if opened
    if (open)
    {
      // add children
      for (size_t i = 0; i < cur_node->children_count; i++)
      {
        std::string tmp_path = full_path + cur_node->children[i]->label;
        if (cur_node->children[i]->type != NODE_TYPES::FILE)
          tmp_path += "/";
        render_tree(cur_node->children[i], tmp_path);
      }
      ImGui::TreePop();
    }
    break;
  case NODE_TYPES::FILE:
    ImGui::PushID(full_path.c_str());

    // render node
    ImGui::TreeNodeEx(cur_node->label.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);

    // hex view on double click
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
      command.cur_node = cur_node;
      command.action = COMMAND_ACTIONS::VIEW;
    }

    // handle right click
    ImGui::OpenPopupOnItemClick("tree popup", ImGuiPopupFlags_MouseButtonRight);
    show_context_menu_popup(cur_node, full_path);

    ImGui::PopID();

    // handle drag
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
      // Set payload to carry the index of our item (could be anything)
      ImGui::SetDragDropPayload("DND_DEMO_CELL", &cur_node, sizeof(cur_node));

      // drag ghost
      ImGui::TextUnformatted(full_path.c_str());
      ImGui::EndDragDropSource();
    }

    // render table
    ImGui::TableNextColumn();
    ImGui::Text("%d", cur_node->size);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(node_types[static_cast<int>(cur_node->type)]);
    break;
  default:
    printf("NODE_TYPES unknown for %s\n", cur_node->label.c_str());
    break;
  }
  return;
}

void file_system_explorer_init()
{
  //
  // TODO: keep track of previously mounted files and mount them
  //
}

void file_system_explorer_render()
{
  const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
  // const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
  ImGui::Begin("Hello, world!", NULL, window_flags); // Create a window called "Hello, world!" and append into it.

#ifdef _DEBUG
  if (ImGui::Button("ImGui Demo"))
  {
    show_demo_window = true;
    ImGui::SetWindowFocus("Dear ImGui Demo");
  }
#endif

  // About section
  ImGui::SameLine(ImGui::GetWindowWidth() - 52);
  if (ImGui::Button("About"))
    show_about = true;

  if (show_about)
  {
    ImGui::OpenPopup("About");
    // ImGui::SetNextWindowSize(ImVec2(350, 200));
    if (ImGui::BeginPopupModal("About", &show_about, ImGuiWindowFlags_AlwaysAutoResize))
    {
      ImGui::Text(ICON_FA_RAINBOW " Rainbow File Explorer %s", RAINBOW_FILE_EXPLORER_VERSION);
      ImGui::Separator();
      ImGui::Text("2023-2025, Broke Studio");
      ImGui::Text("Developed by Antoine Gohin and Charles Ganne.");
      ImGui::Separator();
      ImGui::Text("Powered by");
      ImGui::SameLine();
      ImGui::Text("Dear ImGui");
      // ImGui::TextLinkOpenURL("Dear ImGui", "https://github.com/ocornut/imgui");
      ImGui::SameLine();
      ImGui::Text("v" IMGUI_VERSION);
      ImGui::Text("by Omar Cornut");
      ImGui::EndPopup();
    }
  }

  // Tree section
  static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg;

  if (ImGui::BeginTable("File Browser", 3, flags))
  {
    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
    ImGui::TableHeadersRow();

    // display file browser tree
    render_tree(get_tree());

    ImGui::EndTable();
  }

  // refresh
  if (command.action == COMMAND_ACTIONS::REFRESH)
  {
    refresh_tree();
    reset_command();
  }

  // create/mount drive - select a file
  if (command.action == COMMAND_ACTIONS::CREATE_DRIVE || command.action == COMMAND_ACTIONS::MOUNT_DRIVE)
  {
    if (!ImGui::IsPopupOpen("Drive file"))
      ImGui::OpenPopup("Drive file");
  }

  if (ImGui::IsPopupOpen("Drive file"))
  {
    // file dialog for creating and mounting a drive
    ImVec2 dialog_size = ImVec2(IM_MIN(viewport->WorkSize.x, 700), IM_MIN(viewport->WorkSize.y, 310));
    ImGuiFileBrowser::DialogMode mode = ImGuiFileBrowser::DialogMode::OPEN; // COMMAND_ACTIONS::MOUNT_DRIVE
    if (command.action == COMMAND_ACTIONS::CREATE_DRIVE)
      mode = ImGuiFileBrowser::DialogMode::SAVE;

    if (file_dialog.showFileDialog("Drive file", mode, dialog_size, "*.*"))
    {
      try
      {
        if (command.action == COMMAND_ACTIONS::MOUNT_DRIVE)
          mount_drive(file_dialog.selected_fn, file_dialog.selected_path);
        else if (command.action == COMMAND_ACTIONS::CREATE_DRIVE)
          create_drive(file_dialog.selected_fn, file_dialog.selected_path);
      }
      catch (TREE_MANAGER_ERRORS e)
      {
        show_error(e);
      }
      reset_command();
    }
    else
    {
      if (!ImGui::IsPopupOpen("Drive file"))
        reset_command();
    }
  }

  // unmount drive
  if (command.action == COMMAND_ACTIONS::UNMOUNT_DRIVE)
  {
    unmount_drive(command.cur_node->label);
    reset_command();
  }

  // select a file to add to a drive/folder
  if (command.action == COMMAND_ACTIONS::ADD_FILE)
  {
    ImGui::OpenPopup("Open File");

    ImVec2 dialog_size = ImVec2(IM_MIN(viewport->WorkSize.x - 20, 700), IM_MIN(viewport->WorkSize.y - 20, 310));
    if (file_dialog.showFileDialog("Open File", ImGuiFileBrowser::DialogMode::OPEN, dialog_size, "*.*"))
    {
      try
      {
        add_file(command.cur_node, file_dialog.selected_path);
      }
      catch (TREE_MANAGER_ERRORS e)
      {
        show_error(e);
      }
      reset_command();
    }
    else
    {
      if (!ImGui::IsPopupOpen("Open File"))
        reset_command();
    }
  }

  // delete node
  if (command.action == COMMAND_ACTIONS::DELETE_NODE)
  {
    // open popup
    if (!ImGui::IsPopupOpen("Delete?"))
      ImGui::OpenPopup("Delete?");

    // always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // render popup
    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      ImGui::Text("Are you sure you want to delete this element?\nThis operation cannot be undone!");
      ImGui::Separator();

      if (ImGui::Button("OK", ImVec2(120, 0)))
      {
        // try to delete the node
        try
        {
          delete_node(command.cur_node);
        }
        catch (TREE_MANAGER_ERRORS e)
        {
          show_error(e);
        }
        ImGui::CloseCurrentPopup();
        reset_command();
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0)))
      {
        ImGui::CloseCurrentPopup();
        reset_command();
      }
      ImGui::EndPopup();
    }
  }

  // move file/folder
  if (command.action == COMMAND_ACTIONS::MOVE_NODE)
  {
    // try to move the node
    try
    {
      rename_node(command.cur_node, std::string(command.new_value));
    }
    catch (TREE_MANAGER_ERRORS e)
    {
      show_error(e);
    }
    reset_command();
  }

  // rename file/folder
  if (command.action == COMMAND_ACTIONS::RENAME_NODE)
  {
    // open popup
    if (!ImGui::IsPopupOpen("Rename?"))
      ImGui::OpenPopup("Rename?");

    // always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // render popup
    if (ImGui::BeginPopupModal("Rename?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      bool validate = false, cancel = false;
      ImGui::Text("Please enter the new name/path.");
      if (ImGui::IsWindowAppearing())
        ImGui::SetKeyboardFocusHere();
      ImGui::InputText("##name", command.new_value, 257, ImGuiInputTextFlags_CallbackCharFilter, FilterFileName);
      if (ImGui::IsItemDeactivated())
      {
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
          cancel = true;
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_KeypadEnter)))
          validate = true;
      }
      ImGui::Separator();
      if (ImGui::Button("OK", ImVec2(120, 0)))
        validate = true;
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0)))
        cancel = true;

      // validate or cancel
      if (validate)
      {
        // try to rename the node
        try
        {
          rename_node(command.cur_node, std::string(command.new_value));
        }
        catch (TREE_MANAGER_ERRORS e)
        {
          show_error(e);
        }

        ImGui::CloseCurrentPopup();
        reset_command();
      }
      if (cancel)
      {
        ImGui::CloseCurrentPopup();
        reset_command();
      }
      ImGui::EndPopup();
    }
  }

  // view file content in hex editor
  if (command.action == COMMAND_ACTIONS::VIEW)
  {
    try
    {
      get_file_content(command.cur_node, &file_data, &file_data_size);
      file_data_path = command.cur_node->path;
      show_hex_viewer = true;
    }
    catch (TREE_MANAGER_ERRORS e)
    {
      show_error(e);
    }
    reset_command();
  }

  // extract file in source folder
  if (command.action == COMMAND_ACTIONS::EXTRACT)
  {
    try
    {
      extract_file(command.cur_node);
    }
    catch (TREE_MANAGER_ERRORS e)
    {
      show_error(e);
    }
    reset_command();
  }

  // open error popup
  if (show_error_dialog)
    if (!ImGui::IsPopupOpen("Error"))
      ImGui::OpenPopup("Error");

  // render error popup
  if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
  {
    ImGui::TextUnformatted(error_msg.c_str());
    ImGui::Separator();
    if (ImGui::Button("OK", ImVec2(120, 0)))
    {
      ImGui::CloseCurrentPopup();
      show_error_dialog = false;
    }
    ImGui::EndPopup();
  }

  // render memory editor window if needed
  if (show_hex_viewer && file_data_size != 0)
  {
    static MemoryEditor mem_edit;
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.60f, viewport->WorkSize.y * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::Begin("File hex viewer", &show_hex_viewer, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("Current file: %s", file_data_path.c_str());
    ImGui::Separator();
    mem_edit.DrawContents(file_data, file_data_size);
    ImGui::End();
  }

  ImGui::End();

  // #ifdef _DEBUG
  //   if (show_demo_window)
  //     ImGui::ShowDemoWindow(&show_demo_window);
  // #endif
}

void show_status_bar_window(void)
{
  ImGui::Begin("Status");
  // ImGui::AlignTextToFramePadding();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::SameLine();
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::Text("width: %f - height : %f", viewport->WorkSize.x, viewport->WorkSize.y);
  ImGui::End();
}
