#include <iostream>
#include <fstream>
#include <stdio.h>

// imgui
#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileBrowser.h"
#include "imgui_memory_editor.h"
#include "imgui_helpers.h"

// Tree Manager
#include <RainbowFileSystem/TreeManager.h>

// FontAwesome
#include "IconsFontAwesome6.h"

using namespace imgui_addons;

namespace FileSystem
{

  enum class COMMAND_ACTIONS
  {
    IDLE,
    REFRESH,
    MOUNT_DRIVE,
    ADD_FILE,
    ADD_FOLDER,
    CREATE_DRIVE,
    SAVE_DRIVE,
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
    Node* cur_node = NULL;
    Node* target_node = NULL;
  } command_t;

  // global vars
  ImGuiFileBrowser file_dialog;
  command_t command;
  bool show_demo_window = false;
  bool show_error_dialog = false;
  bool show_about = false;
  bool show_hex_viewer = false;
  std::string error_msg = "";

  TreeManager tree_manager;

  Node* viewed_node = NULL;

  /**
   * @brief Opens the error dialog for a TreeManager error.
   *
   * @param error Error reported by TreeManager.
   */
  void show_error(Error error)
  {
    error_msg = get_error_message(error);
    show_error_dialog = true;
  }

  /**
   * @brief Returns the display name associated with a node type.
   *
   * @param type Node type to convert.
   * @return Name displayed in the Type column.
   */
  const char* get_node_type_name(NodeType type)
  {
    switch(type) {
      case NodeType::Root: return "Root";
      case NodeType::Drive: return "Drive";
      case NodeType::Folder: return "Folder";
      case NodeType::File: return "File";
    }
    return "Unknown";
  }

  /**
   * @brief Checks whether a node belongs to a subtree.
   *
   * This is used to close the memory viewer before its node, or one of its
   * parents, is removed from the tree.
   *
   * @param parent Root of the subtree to inspect.
   * @param searched_node Node to find.
   * @return true if searched_node belongs to the subtree.
   */
  bool contains_node(const Node* parent, const Node* searched_node)
  {
    if(parent == searched_node) {
      return true;
    }

    for(const std::unique_ptr<Node>& child : parent->children) {
      if(contains_node(child.get(), searched_node)) {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Clears the pending GUI command and its node pointers.
   */
  void reset_command()
  {
    command.action = COMMAND_ACTIONS::IDLE;
    // command.new_value = "";
    command.new_value[0] = '\0';
    command.cur_node = NULL;
    command.target_node = NULL;
  }

  /**
   * @brief Displays the context menu associated with a tree node.
   *
   * Selected actions are stored in command and handled later by render().
   *
   * @param cur_node Node associated with the context menu.
   * @param path Path used to identify the displayed tree item.
   */
  void show_context_menu_popup(Node* cur_node, const std::string& path)
  {
    if(ImGui::BeginPopupContextItem("tree popup")) {
      // register clicked node
      command.cur_node = cur_node;

      // branch on node type
      if(cur_node->type == NodeType::Root) // root
      {
        if(ImGui::Selectable(ICON_FA_ROTATE " Refresh")) {
          command.action = COMMAND_ACTIONS::REFRESH;
        }

        if(ImGui::Selectable(ICON_FA_PLUS " Create drive")) {
          command.action = COMMAND_ACTIONS::CREATE_DRIVE;
        }

        if(ImGui::Selectable(ICON_FA_PLUG_CIRCLE_PLUS " Mount drive")) {
          command.action = COMMAND_ACTIONS::MOUNT_DRIVE;
        }
      } else if(cur_node->type == NodeType::Drive) // drive root
      {
        if(ImGui::Selectable(ICON_FA_FLOPPY_DISK " Save drive")) {
          command.action = COMMAND_ACTIONS::SAVE_DRIVE;
        }

        if(ImGui::Selectable(ICON_FA_FILE_MEDICAL " Add file")) {
          command.action = COMMAND_ACTIONS::ADD_FILE;
        }

        if(ImGui::Selectable(ICON_FA_FOLDER_PLUS " Add folder")) {
          command.new_value[0] = '\0';
          command.action = COMMAND_ACTIONS::ADD_FOLDER;
        }

        if(ImGui::Selectable(ICON_FA_PLUG_CIRCLE_XMARK " Unmount drive")) {
          command.action = COMMAND_ACTIONS::UNMOUNT_DRIVE;
        }
      } else if(cur_node->type == NodeType::Folder) // folder
      {
        if(ImGui::Selectable(ICON_FA_FILE_MEDICAL " Add file")) {
          command.action = COMMAND_ACTIONS::ADD_FILE;
        }

        if(ImGui::Selectable(ICON_FA_FOLDER_PLUS " Add folder")) {
          command.new_value[0] = '\0';
          command.action = COMMAND_ACTIONS::ADD_FOLDER;
        }

        if(ImGui::Selectable(ICON_FA_PEN " Rename folder")) {
          strcpy(command.new_value, cur_node->name.c_str());
          command.action = COMMAND_ACTIONS::RENAME_NODE;
        }

        if(ImGui::Selectable(ICON_FA_TRASH " Delete folder")) {
          command.action = COMMAND_ACTIONS::DELETE_NODE;
        }
      } else if(cur_node->type == NodeType::File) // file
      {
        if(ImGui::Selectable(ICON_FA_MAGNIFYING_GLASS " View file")) {
          command.action = COMMAND_ACTIONS::VIEW;
        }

        if(ImGui::Selectable(ICON_FA_FILE_ARROW_DOWN " Extract file")) {
          command.action = COMMAND_ACTIONS::EXTRACT;
        }

        if(ImGui::Selectable(ICON_FA_PEN " Rename file")) {
          strcpy(command.new_value, cur_node->name.c_str());
          command.action = COMMAND_ACTIONS::RENAME_NODE;
        }

        if(ImGui::Selectable(ICON_FA_TRASH " Delete file")) {
          command.action = COMMAND_ACTIONS::DELETE_NODE;
        }
      }
      ImGui::EndPopup();
    }
  }

  /**
   * @brief Filters characters entered in file and folder name fields.
   *
   * @param data ImGui text input callback data.
   * @return 0 when the character is accepted, 1 when it is rejected.
   */
  static int FilterFileName(ImGuiInputTextCallbackData* data)
  {
    // Uppercase becomes lowercase
    if(data->EventChar >= 'A' && data->EventChar <= 'Z') {
      data->EventChar += 'a' - 'A';
      return 0;
    }

    // forbidden characters
    if(data->EventChar > 256 || strchr("<>:\"/\\|?*", (char)data->EventChar)) {
      return 1;
    }

    return 0;
  }

  /**
   * @brief Renders a node and its children in the file-system table.
   *
   * @param cur_node Node to render.
   * @param full_path Display path used as a unique ImGui identifier.
   */
  void render_tree(Node* cur_node, std::string full_path = "")
  {
    // init vars
    bool open = false;
    std::string displayed_name;

    // init table
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    // handle node types
    switch(cur_node->type) {
      case NodeType::Root:

        ImGui::PushID(full_path.c_str());

        // root node is always open
        ImGui::SetNextItemOpen(true);

        // render node
        open = ImGui::TreeNodeEx(cur_node->name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);

        // handle right click
        ImGui::OpenPopupOnItemClick("tree popup", ImGuiPopupFlags_MouseButtonRight);
        show_context_menu_popup(cur_node, full_path);

        ImGui::PopID();

        // render table
        ImGui::TableNextColumn();
        ImGui::TextDisabled("--");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(get_node_type_name(cur_node->type));

        // render children if opened
        if(open) {
          // add children
          for(const std::unique_ptr<Node>& child : cur_node->children) {
            std::string tmp_path = "/" + child->name + "/";
            render_tree(child.get(), tmp_path);
          }
          ImGui::TreePop();
        }
        break;
      case NodeType::Drive:
      case NodeType::Folder:
        ImGui::PushID(full_path.c_str());

        // render node
        displayed_name = cur_node->name;
        if(cur_node->type == NodeType::Drive && cur_node->modified) {
          displayed_name += " *";
        }
        open = ImGui::TreeNodeEx(displayed_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
        if(cur_node->type == NodeType::Drive) {
          if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay)) {
            ImGui::SetTooltip("%s", cur_node->image_path.string().c_str());
          }
        }

        // handle right click
        ImGui::OpenPopupOnItemClick("tree popup", ImGuiPopupFlags_MouseButtonRight);
        show_context_menu_popup(cur_node, full_path);

        ImGui::PopID();

        // handle drag
        if(cur_node->type == NodeType::Folder) {
          if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) // ImGuiDragDropFlags_None // ImGuiDragDropFlags_SourceAllowNullID
          {
            // Set payload to carry the index of our item (could be anything)
            ImGui::SetDragDropPayload("DND_CELL", &cur_node, sizeof(cur_node));

            // drag ghost
            ImGui::TextUnformatted(full_path.c_str());
            ImGui::EndDragDropSource();
          }
        }

        // handle drop
        if(ImGui::BeginDragDropTarget()) {
          if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_CELL")) {
            Node* src_node = *(Node**)payload->Data;
            command.action = COMMAND_ACTIONS::MOVE_NODE;
            command.cur_node = src_node;
            command.target_node = cur_node;
          }
          ImGui::EndDragDropTarget();
        }

        // render table
        ImGui::TableNextColumn();
        ImGui::TextDisabled("--");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(get_node_type_name(cur_node->type));

        // render children if opened
        if(open) {
          // add children
          for(const std::unique_ptr<Node>& child : cur_node->children) {
            std::string tmp_path = full_path + child->name;
            if(child->type != NodeType::File) {
              tmp_path += "/";
            }
            render_tree(child.get(), tmp_path);
          }
          ImGui::TreePop();
        }
        break;
      case NodeType::File:
        ImGui::PushID(full_path.c_str());

        // render node
        ImGui::TreeNodeEx(cur_node->name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);

        // hex view on double click
        if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
          command.cur_node = cur_node;
          command.action = COMMAND_ACTIONS::VIEW;
        }

        // handle right click
        ImGui::OpenPopupOnItemClick("tree popup", ImGuiPopupFlags_MouseButtonRight);
        show_context_menu_popup(cur_node, full_path);

        ImGui::PopID();

        // handle drag
        if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
          // Set payload to carry the index of our item (could be anything)
          ImGui::SetDragDropPayload("DND_CELL", &cur_node, sizeof(cur_node));

          // drag ghost
          ImGui::TextUnformatted(full_path.c_str());
          ImGui::EndDragDropSource();
        }

        // render table
        ImGui::TableNextColumn();
        ImGui::Text("%llu", static_cast<unsigned long long>(cur_node->data.size()));
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(get_node_type_name(cur_node->type));
        break;
      default:
        printf("NodeType unknown for %s\n", cur_node->name.c_str());
        break;
    }
    return;
  }

  /**
   * @brief Renders the file-system explorer and processes pending commands.
   * @param droppedFilename File dropped during the current frame.
   * @param scale Current interface scale.
   */
  void render(const std::string& droppedFilename, float scale)
  {
    if(!droppedFilename.empty()) {
      try {
        tree_manager.mount_drive("", droppedFilename);
      } catch(Error e) {
        show_error(e);
      }
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 popupButtonSize = ImVec2(150, 0);
    ImVec2 dialog_size = ImVec2(
      IM_MIN(viewport->WorkSize.x - 20.0f, 700.0f * scale),
      IM_MIN(viewport->WorkSize.y - 20.0f, 310.0f * scale));

    ImGui::SeparatorText("File System Explorer");
    ImGui::BeginChild("FileSystemExplorer");

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    // Tree section
    static ImGuiTableFlags flags =
      ImGuiTableFlags_ScrollY |
      ImGuiTableFlags_BordersOuterH |
      ImGuiTableFlags_Resizable |
      ImGuiTableFlags_RowBg |
      ImGuiTableFlags_SizingStretchProp;

    ImVec2 available_size = ImGui::GetContentRegionAvail();
    ImVec2 outer_size = ImVec2(0.0f, available_size.y); //TEXT_BASE_HEIGHT * 8);
    if(ImGui::BeginTable("File Browser", 3, flags, outer_size)) {
      ImGui::TableSetupScrollFreeze(1, 1);
      // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 50.0f);
      ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch, 25.0f);
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 25.0f);
      ImGui::TableHeadersRow();

      // display file browser tree
      render_tree(tree_manager.get_tree());

      ImGui::EndTable();
    }

    // refresh
    if(command.action == COMMAND_ACTIONS::REFRESH) {
      try {
        if(!tree_manager.refresh()) {
          error_msg = "Save modified drives before refreshing the tree.";
          show_error_dialog = true;
        } else {
          viewed_node = NULL;
          show_hex_viewer = false;
        }
      } catch(Error e) {
        show_error(e);
      }
      reset_command();
    }

    // create/mount drive - select a file
    if(command.action == COMMAND_ACTIONS::CREATE_DRIVE || command.action == COMMAND_ACTIONS::MOUNT_DRIVE) {
      if(!ImGui::IsPopupOpen("Drive file")) {
        ImGui::OpenPopup("Drive file");
      }

      ImGuiFileBrowser::DialogMode mode = ImGuiFileBrowser::DialogMode::OPEN;
      if(command.action == COMMAND_ACTIONS::CREATE_DRIVE) {
        mode = ImGuiFileBrowser::DialogMode::SAVE;
      }

      if(file_dialog.showFileDialog("Drive file", mode, dialog_size, "*.*")) {
        try {
          if(command.action == COMMAND_ACTIONS::CREATE_DRIVE) {
            tree_manager.create_drive(file_dialog.selected_fn, file_dialog.selected_path);
          } else {
            tree_manager.mount_drive(file_dialog.selected_fn, file_dialog.selected_path);
          }
        } catch(Error error) {
          show_error(error);
        }
        reset_command();
      } else if(!ImGui::IsPopupOpen("Drive file")) {
        reset_command();
      }
    }

    // unmount drive
    if(command.action == COMMAND_ACTIONS::UNMOUNT_DRIVE) {
      const bool close_viewer = viewed_node != NULL && contains_node(command.cur_node, viewed_node);

      if(!command.cur_node->modified) {
        if(tree_manager.unmount_drive(command.cur_node) && close_viewer) {
          viewed_node = NULL;
          show_hex_viewer = false;
        }
        reset_command();
      } else {
        if(!ImGui::IsPopupOpen("Unsaved changes")) {
          ImGui::OpenPopup("Unsaved changes");
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if(ImGui::BeginPopupModal("Unsaved changes", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
          ImGui::Text("This drive contains unsaved changes.");
          ImGui::Text("Do you want to save them before unmounting?");
          ImGui::Separator();

          if(ImGui::Button("Save", popupButtonSize)) {
            try {
              tree_manager.save_drive(command.cur_node);
              tree_manager.unmount_drive(command.cur_node);
              if(close_viewer) {
                viewed_node = NULL;
                show_hex_viewer = false;
              }
            } catch(Error e) {
              show_error(e);
            }
            ImGui::CloseCurrentPopup();
            reset_command();
          }

          ImGui::SameLine();
          if(ImGui::Button("Don't save", popupButtonSize)) {
            tree_manager.unmount_drive(command.cur_node, true);
            if(close_viewer) {
              viewed_node = NULL;
              show_hex_viewer = false;
            }
            ImGui::CloseCurrentPopup();
            reset_command();
          }

          ImGui::SameLine();
          if(ImGui::Button("Cancel", popupButtonSize)) {
            ImGui::CloseCurrentPopup();
            reset_command();
          }

          ImGui::EndPopup();
        }
      }
    }

    // save the complete in-memory drive
    if(command.action == COMMAND_ACTIONS::SAVE_DRIVE) {
      try {
        tree_manager.save_drive(command.cur_node);
      } catch(Error e) {
        show_error(e);
      }
      reset_command();
    }

    // select a file to add to a drive/folder
    if(command.action == COMMAND_ACTIONS::ADD_FILE) {
      if(!ImGui::IsPopupOpen("Open File")) {
        ImGui::OpenPopup("Open File");
      }

      if(file_dialog.showFileDialog("Open File", ImGuiFileBrowser::DialogMode::OPEN, dialog_size, "*.*")) {
        try {
          tree_manager.add_entry(command.cur_node, file_dialog.selected_path);
        } catch(Error error) {
          show_error(error);
        }
        reset_command();
      } else if(!ImGui::IsPopupOpen("Open File")) {
        reset_command();
      }
    }

    // create an empty folder
    if(command.action == COMMAND_ACTIONS::ADD_FOLDER) {
      if(!ImGui::IsPopupOpen("New folder")) {
        ImGui::OpenPopup("New folder");
      }

      ImVec2 center = ImGui::GetMainViewport()->GetCenter();
      ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
      if(ImGui::BeginPopupModal("New folder", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool validate = false;
        bool cancel = false;
        ImGui::Text("Please enter the folder name.");
        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##folder_name", command.new_value, 257, ImGuiInputTextFlags_CallbackCharFilter, FilterFileName);
        if(ImGui::Button("OK", popupButtonSize)) {
          validate = true;
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel", popupButtonSize)) {
          cancel = true;
        }

        if(validate) {
          try {
            tree_manager.create_folder(command.cur_node, command.new_value);
          } catch(Error e) {
            show_error(e);
          }
          ImGui::CloseCurrentPopup();
          reset_command();
        } else if(cancel) {
          ImGui::CloseCurrentPopup();
          reset_command();
        }
        ImGui::EndPopup();
      }
    }

    // delete node
    if(command.action == COMMAND_ACTIONS::DELETE_NODE) {
      // open popup
      if(!ImGui::IsPopupOpen("Delete?")) {
        ImGui::OpenPopup("Delete?");
      }

      // always center this window when appearing
      ImVec2 center = ImGui::GetMainViewport()->GetCenter();
      ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

      // render popup
      if(ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this element?\nThis operation cannot be undone!");
        ImGui::Separator();

        if(ImGui::Button("OK", popupButtonSize)) {
          // try to delete the node
          try {
            if(viewed_node != NULL && contains_node(command.cur_node, viewed_node)) {
              viewed_node = NULL;
              show_hex_viewer = false;
            }
            tree_manager.remove_entry(command.cur_node);
          } catch(Error e) {
            show_error(e);
          }
          ImGui::CloseCurrentPopup();
          reset_command();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if(ImGui::Button("Cancel", popupButtonSize)) {
          ImGui::CloseCurrentPopup();
          reset_command();
        }
        ImGui::EndPopup();
      }
    }

    // move file/folder
    if(command.action == COMMAND_ACTIONS::MOVE_NODE) {
      // try to move the node
      try {
        tree_manager.move_entry(command.cur_node, command.target_node);
      } catch(Error e) {
        show_error(e);
      }
      reset_command();
    }

    // rename file/folder
    if(command.action == COMMAND_ACTIONS::RENAME_NODE) {
      // open popup
      if(!ImGui::IsPopupOpen("Rename?")) {
        ImGui::OpenPopup("Rename?");
      }

      // always center this window when appearing
      ImVec2 center = ImGui::GetMainViewport()->GetCenter();
      ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

      // render popup
      if(ImGui::BeginPopupModal("Rename?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool validate = false, cancel = false;
        ImGui::Text("Please enter the new name.");
        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##name", command.new_value, 257, ImGuiInputTextFlags_CallbackCharFilter, FilterFileName);
        if(ImGui::IsItemDeactivated()) {
          if(ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            cancel = true;
          }
          if(ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
            validate = true;
          }
        }
        ImGui::Separator();
        if(ImGui::Button("OK", popupButtonSize)) {
          validate = true;
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel", popupButtonSize)) {
          cancel = true;
        }

        // validate or cancel
        if(validate) {
          // try to rename the node
          try {
            tree_manager.rename_entry(command.cur_node, std::string(command.new_value));
          } catch(Error e) {
            show_error(e);
          }

          ImGui::CloseCurrentPopup();
          reset_command();
        }
        if(cancel) {
          ImGui::CloseCurrentPopup();
          reset_command();
        }
        ImGui::EndPopup();
      }
    }

    // view file content in hex editor
    if(command.action == COMMAND_ACTIONS::VIEW) {
      try {
        viewed_node = command.cur_node;
        show_hex_viewer = true;
      } catch(Error e) {
        show_error(e);
      }
      reset_command();
    }

    // extract file
    if(command.action == COMMAND_ACTIONS::EXTRACT) {
      if(!ImGui::IsPopupOpen("Extract file")) {
        ImGui::OpenPopup("Extract file");
      }

      if(file_dialog.showFileDialog("Extract file", ImGuiFileBrowser::DialogMode::SAVE, dialog_size, "*.*")) {
        try {
          tree_manager.extract_file(command.cur_node, file_dialog.selected_path);
        } catch(Error error) {
          show_error(error);
        }
        reset_command();
      } else if(!ImGui::IsPopupOpen("Extract file")) {
        reset_command();
      }
    }

    // open error popup
    if(show_error_dialog && !ImGui::IsPopupOpen("Error")) {
      ImGui::OpenPopup("Error");
    }

    // render error popup
    if(ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::TextUnformatted(error_msg.c_str());
      ImGui::Separator();
      if(ImGui::Button("OK", popupButtonSize)) {
        ImGui::CloseCurrentPopup();
        show_error_dialog = false;
      }
      ImGui::EndPopup();
    }

    // render memory editor window if needed
    if(show_hex_viewer && viewed_node != NULL) {
      static MemoryEditor mem_edit;
      mem_edit.ReadOnly = true;
      ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.60f, viewport->WorkSize.y * 0.60f), ImGuiCond_FirstUseEver);
      ImGui::Begin("File hex viewer", &show_hex_viewer, ImGuiWindowFlags_NoScrollbar);
      ImGui::Text("Current file: %s", viewed_node->entry_path.c_str());
      ImGui::Separator();
      mem_edit.DrawContents(viewed_node->data.data(), viewed_node->data.size());
      ImGui::End();
    }

    if(!show_hex_viewer) {
      viewed_node = NULL;
    }

    ImGui::EndChild();
  }

  /**
   * @brief Displays the development status window.
   */
  void show_status_bar_window(void)
  {
    ImGui::Begin("Status");
    // ImGui::AlignTextToFramePadding();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::SameLine();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::Text("width: %f - height : %f", viewport->WorkSize.x, viewport->WorkSize.y);
    ImGui::End();
  }
}
