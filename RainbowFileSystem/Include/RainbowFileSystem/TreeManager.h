#pragma once

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

constexpr std::size_t FILE_MAX_PATH_LENGTH = 247;

enum class Error
{
  DriveNotFound,
  FileAlreadyMounted,
  EntryAlreadyExists,
  EntryNotFound,
  FailedToOpenSource,
  FailedToOpenDestination,
  UnknownImageFormat,
  UnsupportedImageVersion,
  MalformedImage,
  InvalidPath,
  FileTooLarge
};

const char* get_error_message(Error error);

enum class NodeType
{
  Root,
  Drive,
  Folder,
  File
};

/**
 * @brief Node stored by the in-memory file-system editor.
 *
 * File nodes own their complete content in data. Folder nodes own their child
 * nodes. drive is a non-owning pointer to the drive containing this node.
 */
struct Node
{
  NodeType type = NodeType::Root;
  Node* drive = nullptr;
  std::string name;
  std::string entry_path;
  std::filesystem::path image_path;
  std::vector<std::uint8_t> data;
  std::vector<std::unique_ptr<Node>> children;
  bool modified = false;
};

/**
 * @brief Loads RNBWFS images into RAM and saves complete rebuilt images.
 *
 * Mutating operations only update the in-memory tree. save_drive() is the only
 * operation that modifies an image file.
 */
class TreeManager
{
  public:
  TreeManager();

  TreeManager(const TreeManager&) = delete;
  TreeManager& operator=(const TreeManager&) = delete;

  Node* get_tree() noexcept;
  const Node* get_tree() const noexcept;

  Node* mount_drive(std::string name, const std::filesystem::path& image_path);
  Node* create_drive(const std::string& name, const std::filesystem::path& image_path);
  bool unmount_drive(Node* drive, bool discard_changes = false);
  bool refresh();
  Node* add_entry(Node* parent, const std::filesystem::path& source_path);
  Node* create_folder(Node* parent, const std::string& name);
  Node* rename_entry(Node* node, const std::string& name);
  Node* move_entry(Node* node, Node* new_parent);
  bool remove_entry(Node* node);
  void extract_file(const Node* file, const std::filesystem::path& destination_path) const;

  void display_tree(const Node* parent, int level);

  void save_drive(Node* drive);

  private:
  Node root_;

  std::unique_ptr<Node> load_drive(
    const std::string& name,
    const std::filesystem::path& image_path);

  Node* add_path(
    Node* parent,
    const std::string& path,
    NodeType final_type,
    std::vector<std::uint8_t> data = {});

  Node* add_child(
    Node* parent,
    NodeType type,
    const std::string& name,
    const std::string& entry_path,
    std::vector<std::uint8_t> data = {});

  Node* find_child(Node& parent, const std::string& name) const noexcept;
  Node* find_parent(Node& parent, const Node* child) const noexcept;
  bool contains_node(const Node& parent, const Node* node) const noexcept;
  bool can_update_entry_paths(const Node& node, const std::string& entry_path) const;
  bool remove_from_tree(Node& parent, const Node* target);
  void update_entry_paths(Node& node);
  void update_drive(Node& node, Node* drive);

  void write_tree(std::ostream& image, const Node& parent) const;
  void write_file(std::ostream& image, const Node& file) const;
  void write_empty_folder(std::ostream& image, const Node& folder) const;
};
