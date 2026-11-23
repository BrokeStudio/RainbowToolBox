#include <algorithm>
#include <fstream>
#include <limits>
#include <ostream>
#include <utility>

#include <RainbowFileSystem/TreeManager.h>

namespace
{

  constexpr std::uint8_t IMAGE_VERSION = 0;

  /**
   * @brief Extract a file name from a path
   *
   * Both Windows and Unix path separators are supported. If the path does not
   * contain a separator, the complete path is returned. A trailing separator
   * produces an empty file name.
   *
   * @param path path containing the file name
   * @return file name without its parent directories
   */
  std::string get_filename(const std::string& path)
  {
    size_t separator_pos = path.find_last_of("/\\");

    if(separator_pos == std::string::npos) {
      return path;
    }

    return path.substr(separator_pos + 1);
  }

  /**
   * @brief Compares two ASCII strings without case sensitivity.
   * @return true when both strings are equal.
   */
  bool equals_ignore_ascii_case(const std::string& left, const std::string& right)
  {
    if(left.size() != right.size()) {
      return false;
    }

    for(std::size_t index = 0; index < left.size(); index++) {
      char left_character = left[index];
      char right_character = right[index];

      if(left_character >= 'a' && left_character <= 'z') {
        left_character = static_cast<char>(left_character - 'a' + 'A');
      }
      if(right_character >= 'a' && right_character <= 'z') {
        right_character = static_cast<char>(right_character - 'a' + 'A');
      }

      if(left_character != right_character) {
        return false;
      }
    }

    return true;
  }

  /** @brief Returns whether a character is allowed in an 8.3 name. */
  bool is_valid_83_character(unsigned char character)
  {
    if((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') || (character >= '0' && character <= '9')) {
      return true;
    }

    constexpr char valid_special_characters[] = "$%'-_@~`!(){}^#&";
    return std::find(
             std::begin(valid_special_characters),
             std::end(valid_special_characters) - 1,
             static_cast<char>(character)) != std::end(valid_special_characters) - 1;
  }

  /** @brief Returns whether a complete internal path uses the 8.3 format. */
  bool is_valid_83_path(const std::string& path)
  {
    if(path.empty() || path.size() > FILE_MAX_PATH_LENGTH || path.front() != '/') {
      return false;
    }

    std::size_t position = 1;
    if(position == path.size()) {
      return false;
    }

    while(position < path.size()) {
      std::size_t name_length = 0;
      std::size_t extension_length = 0;
      bool has_dot = false;

      while(position < path.size() && path[position] != '/') {
        const unsigned char character = static_cast<unsigned char>(path[position]);

        if(character == '.') {
          if(has_dot || name_length == 0) {
            return false;
          }
          has_dot = true;
        } else {
          if(!is_valid_83_character(character)) {
            return false;
          }

          if(!has_dot) {
            if(name_length >= 8) {
              return false;
            }
            name_length++;
          } else {
            if(extension_length >= 3) {
              return false;
            }
            extension_length++;
          }
        }

        position++;
      }

      if(name_length == 0 || (has_dot && extension_length == 0)) {
        return false;
      }

      if(position < path.size()) {
        position++;
        if(position == path.size() || path[position] == '/') {
          return false;
        }
      }
    }

    return true;
  }

  /** @brief Builds a child path from its parent path and entry name. */
  std::string join_entry_path(const Node& parent, const std::string& name)
  {
    return parent.entry_path == "/" ? "/" + name : parent.entry_path + "/" + name;
  }

  /** @brief Reads one byte or reports a malformed image. */
  int read_byte(std::istream& image)
  {
    const int value = image.get();
    if(value == EOF) {
      throw Error::MalformedImage;
    }
    return value;
  }

  /** @brief Reads one big-endian unsigned 32-bit value. */
  std::uint32_t read_u32(std::istream& image)
  {
    std::uint32_t value = 0;
    for(int index = 0; index < 4; index++) {
      value = (value << 8) | static_cast<std::uint32_t>(read_byte(image));
    }
    return value;
  }

  /** @brief Writes one big-endian unsigned 32-bit value. */
  void write_u32(std::ostream& image, std::uint32_t value)
  {
    image.put(static_cast<char>((value >> 24) & 0xff));
    image.put(static_cast<char>((value >> 16) & 0xff));
    image.put(static_cast<char>((value >> 8) & 0xff));
    image.put(static_cast<char>(value & 0xff));
  }

  /** @brief Reads and validates one length-prefixed internal path. */
  std::string read_entry_path(std::istream& image)
  {
    const int path_length = read_byte(image);
    if(path_length == 0 || path_length > static_cast<int>(FILE_MAX_PATH_LENGTH)) {
      throw Error::MalformedImage;
    }

    std::string path(static_cast<std::size_t>(path_length), '\0');
    image.read(path.data(), static_cast<std::streamsize>(path.size()));
    if(!image || !is_valid_83_path(path)) {
      throw Error::MalformedImage;
    }
    return path;
  }

  /** @brief Validates and writes one length-prefixed internal path. */
  void write_entry_path(std::ostream& image, const std::string& path)
  {
    if(!is_valid_83_path(path)) {
      throw Error::InvalidPath;
    }

    image.put(static_cast<char>(path.size()));
    image.write(path.data(), static_cast<std::streamsize>(path.size()));
  }

  /** @brief Writes the current RNBWFS signature and version. */
  void write_image_header(std::ostream& image)
  {
    image.write("RNBWFS", 6);
    image.put(static_cast<char>(0x1a));
    image.put(static_cast<char>(IMAGE_VERSION));
  }

  /** @brief Reads and validates an RNBWFS image header. */
  void check_image_header(std::istream& image)
  {
    constexpr char expected_signature[] = { 'R', 'N', 'B', 'W', 'F', 'S', static_cast<char>(0x1a) };
    for(char expected : expected_signature) {
      if(read_byte(image) != static_cast<unsigned char>(expected)) {
        throw Error::UnknownImageFormat;
      }
    }

    if(read_byte(image) != IMAGE_VERSION) {
      throw Error::UnsupportedImageVersion;
    }
  }

} // namespace

/**
 * @brief Returns the user-facing message associated with an error.
 * @param error Error value to describe.
 * @return Static null-terminated error message.
 */
const char* get_error_message(Error error)
{
  switch(error) {
    case Error::DriveNotFound: return "Drive not found.";
    case Error::FileAlreadyMounted: return "This image is already mounted.";
    case Error::EntryAlreadyExists: return "An entry with this name already exists.";
    case Error::EntryNotFound: return "Entry not found.";
    case Error::FailedToOpenSource: return "Failed to open the source file.";
    case Error::FailedToOpenDestination: return "Failed to write the destination file.";
    case Error::UnknownImageFormat: return "Unknown image format.";
    case Error::UnsupportedImageVersion: return "Unsupported image version.";
    case Error::MalformedImage: return "Malformed image.";
    case Error::InvalidPath: return "The name or path does not use the 8.3 format.";
    case Error::FileTooLarge: return "The file is too large.";
  }

  return "Unknown file system error.";
}

/** @brief Creates an empty in-memory file-system tree. */
TreeManager::TreeManager()
{
  root_.type = NodeType::Root;
  root_.name = "root";
}

/**
 * @brief Returns the mutable tree root.
 * @return Root node.
 */
Node* TreeManager::get_tree() noexcept
{
  return &root_;
}

/**
 * @brief Returns the immutable tree root.
 * @return Root node.
 */
const Node* TreeManager::get_tree() const noexcept
{
  return &root_;
}

/**
 * @brief Finds a direct child by name.
 * @param parent Parent node to search.
 * @param name Entry name to find.
 * @return Matching child, or nullptr.
 */
Node* TreeManager::find_child(Node& parent, const std::string& name) const noexcept
{
  for(const std::unique_ptr<Node>& child : parent.children) {
    if(equals_ignore_ascii_case(child->name, name)) {
      return child.get();
    }
  }
  return nullptr;
}

/**
 * @brief Finds the direct parent of a node.
 * @param parent Subtree root to search.
 * @param child Child node to find.
 * @return Direct parent, or nullptr.
 */
Node* TreeManager::find_parent(Node& parent, const Node* child) const noexcept
{
  for(const std::unique_ptr<Node>& current : parent.children) {
    if(current.get() == child) {
      return &parent;
    }

    Node* result = find_parent(*current, child);
    if(result != nullptr) {
      return result;
    }
  }
  return nullptr;
}

/**
 * @brief Tests whether a node belongs to a subtree.
 * @param parent Subtree root.
 * @param node Node to find.
 * @return true when the node belongs to the subtree.
 */
bool TreeManager::contains_node(const Node& parent, const Node* node) const noexcept
{
  if(&parent == node) {
    return true;
  }

  for(const std::unique_ptr<Node>& child : parent.children) {
    if(contains_node(*child, node)) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Checks whether a path can be assigned to an entire subtree.
 * @param node Subtree root.
 * @param entry_path Proposed root path.
 * @return true when every resulting path is valid.
 */
bool TreeManager::can_update_entry_paths(
  const Node& node,
  const std::string& entry_path) const
{
  if(!is_valid_83_path(entry_path)) {
    return false;
  }

  for(const std::unique_ptr<Node>& child : node.children) {
    const std::string child_path = entry_path + "/" + child->name;
    if(!can_update_entry_paths(*child, child_path)) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Adds one validated child to a parent node.
 * @return Added child node.
 */
Node* TreeManager::add_child(
  Node* parent,
  NodeType type,
  const std::string& name,
  const std::string& entry_path,
  std::vector<std::uint8_t> data)
{
  if(parent == nullptr || name.empty()) {
    throw Error::MalformedImage;
  }

  auto child = std::make_unique<Node>();
  child->type = type;
  child->drive = parent->drive;
  child->name = name;
  child->entry_path = entry_path;
  child->data = std::move(data);

  Node* result = child.get();
  parent->children.push_back(std::move(child));
  return result;
}

/**
 * @brief Adds a complete path and creates intermediate folders.
 * @return Final path node.
 */
Node* TreeManager::add_path(
  Node* parent,
  const std::string& path,
  NodeType final_type,
  std::vector<std::uint8_t> data)
{
  if(parent == nullptr || path.size() < 2 || path.front() != '/') {
    throw Error::MalformedImage;
  }

  const std::size_t slash_position = path.find('/', 1);
  if(slash_position != std::string::npos) {
    const std::string name = path.substr(1, slash_position - 1);
    if(name.empty()) {
      throw Error::MalformedImage;
    }

    Node* folder = find_child(*parent, name);
    if(folder == nullptr) {
      folder = add_child(
        parent,
        NodeType::Folder,
        name,
        join_entry_path(*parent, name));
    } else if(folder->type != NodeType::Folder) {
      throw Error::MalformedImage;
    }

    return add_path(folder, path.substr(slash_position), final_type, std::move(data));
  }

  const std::string name = path.substr(1);
  Node* existing = find_child(*parent, name);
  if(existing != nullptr) {
    if(existing->type == NodeType::Folder && final_type == NodeType::Folder) {
      return existing;
    }
    throw Error::EntryAlreadyExists;
  }

  return add_child(
    parent,
    final_type,
    name,
    join_entry_path(*parent, name),
    std::move(data));
}

/**
 * @brief Parses an RNBWFS image into a detached drive tree.
 * @param name Display name assigned to the drive.
 * @param image_path Host image path.
 * @return Loaded drive tree.
 */
std::unique_ptr<Node> TreeManager::load_drive(
  const std::string& name,
  const std::filesystem::path& image_path)
{
  std::ifstream image(image_path, std::ios::binary);
  if(!image) {
    throw Error::FailedToOpenSource;
  }

  check_image_header(image);

  auto drive = std::make_unique<Node>();
  drive->type = NodeType::Drive;
  drive->drive = drive.get();
  drive->name = name;
  drive->entry_path = "/";
  drive->image_path = image_path;

  while(true) {
    const int marker = image.peek();
    if(marker == EOF) {
      throw Error::MalformedImage;
    }
    if(marker == 0xff) {
      image.get();
      break;
    }

    const int type = read_byte(image);
    if(read_byte(image) != '>') {
      throw Error::MalformedImage;
    }

    if(type == 'D') {
      const std::string path = read_entry_path(image);
      add_path(drive.get(), path, NodeType::Folder);
      continue;
    }

    if(type == 'F') {
      const std::uint32_t size = read_u32(image);
      const std::string path = read_entry_path(image);
      std::vector<std::uint8_t> data(size);
      if(size != 0) {
        image.read(
          reinterpret_cast<char*>(data.data()),
          static_cast<std::streamsize>(data.size()));
        if(!image) {
          throw Error::MalformedImage;
        }
      }
      add_path(drive.get(), path, NodeType::File, std::move(data));
      continue;
    }

    throw Error::MalformedImage;
  }

  return drive;
}

/**
 * @brief Loads and mounts an existing RNBWFS image.
 * @param name Display name, or an empty string to use the file name.
 * @param image_path Host image path.
 * @return Mounted drive node.
 */
Node* TreeManager::mount_drive(
  std::string name,
  const std::filesystem::path& image_path)
{
  if(name.empty()) {
    name = get_filename(image_path.string());
  }

  for(const std::unique_ptr<Node>& child : root_.children) {
    if(child->image_path == image_path) {
      throw Error::FileAlreadyMounted;
    }
  }

  std::unique_ptr<Node> drive = load_drive(name, image_path);
  Node* result = drive.get();
  root_.children.push_back(std::move(drive));
  return result;
}

/**
 * @brief Creates and mounts an empty RNBWFS image.
 * @param name Display name assigned to the drive.
 * @param image_path Host image path to create.
 * @return Mounted drive node.
 */
Node* TreeManager::create_drive(
  const std::string& name,
  const std::filesystem::path& image_path)
{
  if(std::filesystem::exists(image_path)) {
    throw Error::EntryAlreadyExists;
  }

  std::ofstream image(image_path, std::ios::binary | std::ios::trunc);
  if(!image) {
    throw Error::FailedToOpenDestination;
  }

  write_image_header(image);
  image.put(static_cast<char>(0xff));
  image.close();
  if(!image) {
    throw Error::FailedToOpenDestination;
  }

  return mount_drive(name, image_path);
}

/**
 * @brief Removes a drive from the in-memory tree.
 * @param drive Drive to unmount.
 * @param discard_changes Whether unsaved changes may be discarded.
 * @return true when the drive was unmounted.
 */
bool TreeManager::unmount_drive(Node* drive, bool discard_changes)
{
  if(drive == nullptr || drive->type != NodeType::Drive ||
    (drive->modified && !discard_changes)) {
    return false;
  }
  return remove_from_tree(root_, drive);
}

/**
 * @brief Reloads every unmodified mounted drive.
 * @return false when at least one drive contains unsaved changes.
 */
bool TreeManager::refresh()
{
  for(const std::unique_ptr<Node>& drive : root_.children) {
    if(drive->modified) {
      return false;
    }
  }

  std::vector<std::unique_ptr<Node>> refreshed_drives;
  refreshed_drives.reserve(root_.children.size());
  for(const std::unique_ptr<Node>& drive : root_.children) {
    refreshed_drives.push_back(load_drive(drive->name, drive->image_path));
  }
  root_.children = std::move(refreshed_drives);
  return true;
}

/**
 * @brief Adds a host file below a drive or folder.
 * @param parent Destination drive or folder.
 * @param source_path Host file to import.
 * @return Added file node, or nullptr for an invalid parent type.
 */
Node* TreeManager::add_entry(Node* parent, const std::filesystem::path& source_path)
{
  if(parent == nullptr || (parent->type != NodeType::Drive && parent->type != NodeType::Folder)) {
    return nullptr;
  }

  const std::string name = source_path.filename().string();
  const std::string entry_path = join_entry_path(*parent, name);
  if(!is_valid_83_path(entry_path)) {
    throw Error::InvalidPath;
  }
  if(find_child(*parent, name) != nullptr) {
    throw Error::EntryAlreadyExists;
  }

  std::ifstream source(source_path, std::ios::binary | std::ios::ate);
  if(!source) {
    throw Error::FailedToOpenSource;
  }

  const std::streampos end_position = source.tellg();
  if(end_position < 0 || static_cast<std::uint64_t>(end_position) > std::numeric_limits<std::uint32_t>::max()) {
    throw Error::FileTooLarge;
  }

  std::vector<std::uint8_t> data(static_cast<std::size_t>(end_position));
  source.seekg(0);
  if(!data.empty()) {
    source.read(
      reinterpret_cast<char*>(data.data()),
      static_cast<std::streamsize>(data.size()));
    if(!source) {
      throw Error::FailedToOpenSource;
    }
  }

  Node* result = add_path(parent, "/" + name, NodeType::File, std::move(data));
  parent->drive->modified = true;
  return result;
}

/**
 * @brief Creates a folder below a drive or folder.
 * @param parent Destination drive or folder.
 * @param name New folder name.
 * @return Added folder node, or nullptr for an invalid parent type.
 */
Node* TreeManager::create_folder(Node* parent, const std::string& name)
{
  if(parent == nullptr || (parent->type != NodeType::Drive && parent->type != NodeType::Folder)) {
    return nullptr;
  }

  const std::string entry_path = join_entry_path(*parent, name);
  if(!is_valid_83_path(entry_path)) {
    throw Error::InvalidPath;
  }
  if(find_child(*parent, name) != nullptr) {
    throw Error::EntryAlreadyExists;
  }

  Node* result = add_path(parent, "/" + name, NodeType::Folder);
  parent->drive->modified = true;
  return result;
}

/**
 * @brief Rebuilds all descendant paths after moving or renaming a node.
 * @param node Subtree root.
 */
void TreeManager::update_entry_paths(Node& node)
{
  for(const std::unique_ptr<Node>& child : node.children) {
    child->entry_path = join_entry_path(node, child->name);
    update_entry_paths(*child);
  }
}

/**
 * @brief Assigns a drive pointer to an entire subtree.
 * @param node Subtree root.
 * @param drive Owning drive.
 */
void TreeManager::update_drive(Node& node, Node* drive)
{
  node.drive = drive;
  for(const std::unique_ptr<Node>& child : node.children) {
    update_drive(*child, drive);
  }
}

/**
 * @brief Renames an entry and updates all descendant paths.
 * @param node Entry to rename.
 * @param name New entry name.
 * @return Renamed node, or nullptr for an invalid node type.
 */
Node* TreeManager::rename_entry(Node* node, const std::string& name)
{
  if(node == nullptr || node->type == NodeType::Root || node->type == NodeType::Drive) {
    return nullptr;
  }

  Node* parent = find_parent(root_, node);
  if(parent == nullptr) {
    throw Error::EntryNotFound;
  }

  Node* existing = find_child(*parent, name);
  if(existing != nullptr && existing != node) {
    throw Error::EntryAlreadyExists;
  }

  const std::string new_path = join_entry_path(*parent, name);
  if(!can_update_entry_paths(*node, new_path)) {
    throw Error::InvalidPath;
  }

  node->name = name;
  node->entry_path = new_path;
  update_entry_paths(*node);
  node->drive->modified = true;
  return node;
}

/**
 * @brief Moves an entry below another folder or drive.
 * @param node Entry to move.
 * @param new_parent Destination folder or drive.
 * @return Moved node, or nullptr when the move is invalid.
 */
Node* TreeManager::move_entry(Node* node, Node* new_parent)
{
  if(node == nullptr || new_parent == nullptr || node->type == NodeType::Root ||
    node->type == NodeType::Drive ||
    (new_parent->type != NodeType::Drive && new_parent->type != NodeType::Folder) ||
    contains_node(*node, new_parent)) {
    return nullptr;
  }

  Node* old_parent = find_parent(root_, node);
  if(old_parent == nullptr) {
    throw Error::EntryNotFound;
  }
  if(old_parent == new_parent) {
    return node;
  }
  if(find_child(*new_parent, node->name) != nullptr) {
    throw Error::EntryAlreadyExists;
  }

  const std::string new_path = join_entry_path(*new_parent, node->name);
  if(!can_update_entry_paths(*node, new_path)) {
    throw Error::InvalidPath;
  }

  Node* old_drive = node->drive;
  std::unique_ptr<Node> moved_node;
  for(auto iterator = old_parent->children.begin(); iterator != old_parent->children.end(); ++iterator) {
    if(iterator->get() == node) {
      moved_node = std::move(*iterator);
      old_parent->children.erase(iterator);
      break;
    }
  }
  if(!moved_node) {
    throw Error::EntryNotFound;
  }

  moved_node->entry_path = new_path;
  update_drive(*moved_node, new_parent->drive);
  update_entry_paths(*moved_node);
  new_parent->children.push_back(std::move(moved_node));
  old_drive->modified = true;
  new_parent->drive->modified = true;
  return node;
}

/**
 * @brief Removes a target node from a subtree.
 * @param parent Subtree root.
 * @param target Node to remove.
 * @return true when the target was removed.
 */
bool TreeManager::remove_from_tree(Node& parent, const Node* target)
{
  for(auto iterator = parent.children.begin(); iterator != parent.children.end(); ++iterator) {
    if(iterator->get() == target) {
      parent.children.erase(iterator);
      return true;
    }
    if(remove_from_tree(**iterator, target)) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Removes a file or folder subtree.
 * @param node Entry to remove.
 * @return true when the entry was removed.
 */
bool TreeManager::remove_entry(Node* node)
{
  if(node == nullptr || node->type == NodeType::Root || node->type == NodeType::Drive) {
    return false;
  }
  Node* drive = node->drive;
  const bool removed = remove_from_tree(root_, node);
  if(removed) {
    drive->modified = true;
  }
  return removed;
}

/**
 * @brief Extracts a file node to the host file system.
 * @param file File node to extract.
 * @param destination_path Host destination path.
 */
void TreeManager::extract_file(
  const Node* file,
  const std::filesystem::path& destination_path) const
{
  if(file == nullptr || file->type != NodeType::File) {
    throw Error::EntryNotFound;
  }

  std::ofstream destination(destination_path, std::ios::binary | std::ios::trunc);
  if(!destination) {
    throw Error::FailedToOpenDestination;
  }
  if(!file->data.empty()) {
    destination.write(
      reinterpret_cast<const char*>(file->data.data()),
      static_cast<std::streamsize>(file->data.size()));
  }
  if(!destination) {
    throw Error::FailedToOpenDestination;
  }
}

/**
 * @brief Serializes a file entry and its data.
 * @param image Destination image stream.
 * @param file File node to serialize.
 */
void TreeManager::write_file(std::ostream& image, const Node& file) const
{
  if(file.data.size() > std::numeric_limits<std::uint32_t>::max()) {
    throw Error::FileTooLarge;
  }

  image.put('F');
  image.put('>');
  write_u32(image, static_cast<std::uint32_t>(file.data.size()));
  write_entry_path(image, file.entry_path);
  if(!file.data.empty()) {
    image.write(
      reinterpret_cast<const char*>(file.data.data()),
      static_cast<std::streamsize>(file.data.size()));
  }
}

/**
 * @brief Serializes an empty folder entry.
 * @param image Destination image stream.
 * @param folder Folder node to serialize.
 */
void TreeManager::write_empty_folder(std::ostream& image, const Node& folder) const
{
  image.put('D');
  image.put('>');
  write_entry_path(image, folder.entry_path);
}

/**
 * @brief Serializes all descendants of a node.
 * @param image Destination image stream.
 * @param parent Subtree root.
 */
void TreeManager::write_tree(std::ostream& image, const Node& parent) const
{
  for(const std::unique_ptr<Node>& child : parent.children) {
    if(child->type == NodeType::File) {
      write_file(image, *child);
      continue;
    }

    if(child->type == NodeType::Folder) {
      if(child->children.empty()) {
        write_empty_folder(image, *child);
      } else {
        write_tree(image, *child);
      }
    }
  }
}

/**
 * @brief Rebuilds and safely replaces a complete drive image.
 * @param drive Drive to save.
 */
void TreeManager::save_drive(Node* drive)
{
  if(drive == nullptr || drive->type != NodeType::Drive || drive->drive != drive) {
    throw Error::DriveNotFound;
  }

  std::filesystem::path temporary_path = drive->image_path;
  temporary_path += ".tmp";
  std::filesystem::path backup_path = drive->image_path;
  backup_path += ".bak";

  {
    std::ofstream image(temporary_path, std::ios::binary | std::ios::trunc);
    if(!image) {
      throw Error::FailedToOpenDestination;
    }

    write_image_header(image);
    write_tree(image, *drive);
    image.put(static_cast<char>(0xff));
    image.close();
    if(!image) {
      std::error_code ignored;
      std::filesystem::remove(temporary_path, ignored);
      throw Error::FailedToOpenDestination;
    }
  }

  std::error_code error;
  std::filesystem::remove(backup_path, error);
  error.clear();
  std::filesystem::rename(drive->image_path, backup_path, error);
  if(error) {
    std::filesystem::remove(temporary_path, error);
    throw Error::FailedToOpenDestination;
  }

  error.clear();
  std::filesystem::rename(temporary_path, drive->image_path, error);
  if(error) {
    std::error_code restore_error;
    std::filesystem::rename(backup_path, drive->image_path, restore_error);
    std::filesystem::remove(temporary_path, restore_error);
    throw Error::FailedToOpenDestination;
  }

  std::filesystem::remove(backup_path, error);
  drive->modified = false;
}

/**
 * @brief Prints a subtree to the standard output.
 *
 * @param parent Parent node whose children are displayed.
 * @param level Current indentation level in the tree.
 */
void TreeManager::display_tree(const Node* parent, int level)
{
  for(const std::unique_ptr<Node>& node : parent->children) {
    switch(level) {
      case 0:
        break;

      case 1:
        printf("├── ");
        break;

      default:
        for(int i = 0; i < level - 1; i++) {
          printf("|   ");
        }
        printf("├── ");
        break;
    }
    printf("%s", node->name.c_str());
    // if(node->type == NODE_TYPES::FOLDER) {
    //   printf("/");
    // }
    printf("\r\n");
    // if(node->children.size() != 0) {
    display_tree(node.get(), level + 1);
    // }
  }
}
