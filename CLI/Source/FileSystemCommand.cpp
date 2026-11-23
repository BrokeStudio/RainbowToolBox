#include "FileSystemCommand.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "termcolor.hpp"

#include <RainbowFileSystem/TreeManager.h>

namespace Commands
{
  namespace
  {
    struct FileSystemOptions
    {
      std::string create_image;
      std::string list_image;
      std::string add_image;
      std::string add_source;
      std::string add_destination;
      std::string mkdir_image;
      std::string mkdir_path;
      std::string extract_image;
      std::string extract_path;
      std::string extract_output;
      std::string remove_image;
      std::string remove_path;
      std::string validate_image;
      bool remove_recursive = false;
    };

    FileSystemOptions options;

    /**
     * @brief Prints a command error.
     * @param message Error text to display.
     * @return Command failure exit code.
     */
    int report_error(const std::string& message)
    {
      std::cerr << termcolor::bright_red << "Error: " << termcolor::reset << message << std::endl;
      return 1;
    }

    /**
     * @brief Converts an internal path to an absolute slash-separated path.
     * @param path Path to normalize.
     * @return Normalized internal path.
     */
    std::string normalize_path(std::string path)
    {
      std::replace(path.begin(), path.end(), '\\', '/');
      if(path.empty()) {
        return "/";
      }
      if(path.front() != '/') {
        path.insert(path.begin(), '/');
      }
      while(path.size() > 1 && path.back() == '/') {
        path.pop_back();
      }
      return path;
    }

    /**
     * @brief Compares two ASCII strings without case sensitivity.
     * @param left First string.
     * @param right Second string.
     * @return true when both strings are equal.
     */
    bool equals_ignore_case(const std::string& left, const std::string& right)
    {
      if(left.size() != right.size()) {
        return false;
      }
      for(size_t i = 0; i < left.size(); i++) {
        char a = left[i];
        char b = right[i];
        if(a >= 'a' && a <= 'z') a -= 'a' - 'A';
        if(b >= 'a' && b <= 'z') b -= 'a' - 'A';
        if(a != b) return false;
      }
      return true;
    }

    /**
     * @brief Finds a direct child by name.
     * @param parent Node whose children are searched.
     * @param name Entry name to find.
     * @return Matching child, or nullptr.
     */
    Node* find_child(Node* parent, const std::string& name)
    {
      if(parent == nullptr) {
        return nullptr;
      }
      for(std::unique_ptr<Node>& child : parent->children) {
        if(equals_ignore_case(child->name, name)) {
          return child.get();
        }
      }
      return nullptr;
    }

    /**
     * @brief Splits an internal path into entry names.
     * @param path Internal path to split.
     * @return Ordered path components.
     */
    std::vector<std::string> split_path(const std::string& path)
    {
      std::vector<std::string> parts;
      size_t start = 0;
      while(start < path.size()) {
        while(start < path.size() && path[start] == '/') start++;
        if(start == path.size()) break;
        size_t end = path.find('/', start);
        parts.push_back(path.substr(start, end - start));
        if(end == std::string::npos) break;
        start = end + 1;
      }
      return parts;
    }

    /**
     * @brief Finds an entry from its internal path.
     * @param drive Drive used as the path root.
     * @param path Internal entry path.
     * @return Matching node, or nullptr.
     */
    Node* find_node(Node* drive, const std::string& path)
    {
      Node* current = drive;
      std::vector<std::string> parts = split_path(normalize_path(path));
      for(const std::string& part : parts) {
        current = find_child(current, part);
        if(current == nullptr) return nullptr;
      }
      return current;
    }

    /**
     * @brief Creates all missing folders in an internal path.
     * @param manager Tree manager used to create folders.
     * @param drive Drive used as the path root.
     * @param path Internal folder path.
     * @return Final folder, or the drive for the root path.
     */
    Node* create_folders(TreeManager& manager, Node* drive, const std::string& path)
    {
      Node* current = drive;
      std::vector<std::string> parts = split_path(normalize_path(path));
      for(const std::string& part : parts) {
        Node* child = find_child(current, part);
        if(child == nullptr) {
          child = manager.create_folder(current, part);
        } else if(child->type != NodeType::Folder) {
          throw Error::InvalidPath;
        }
        current = child;
      }
      return current;
    }

    /**
     * @brief Mounts an image using its file name as drive name.
     * @param manager Tree manager receiving the image.
     * @param image_path Host image path.
     * @return Mounted drive node.
     */
    Node* mount_image(TreeManager& manager, const std::string& image_path)
    {
      std::filesystem::path image = image_path;
      return manager.mount_drive(image.filename().string(), image);
    }

    /**
     * @brief Prints a node subtree to standard output.
     * @param parent Subtree root.
     * @param level Current indentation level.
     */
    void print_tree(const Node* parent, int level)
    {
      for(const std::unique_ptr<Node>& child : parent->children) {
        for(int i = 0; i < level; i++) std::cout << "  ";
        if(child->type == NodeType::Folder) {
          std::cout << "[" << child->name << "]" << std::endl;
          print_tree(child.get(), level + 1);
        } else {
          std::cout << child->name << " (" << child->data.size() << " bytes)" << std::endl;
        }
      }
    }

    /**
     * @brief Accumulates entry counts and file-data size for a subtree.
     * @param parent Subtree root.
     * @param files File counter to update.
     * @param folders Folder counter to update.
     * @param data_size File-data size counter to update.
     */
    void count_entries(const Node* parent, size_t& files, size_t& folders, uint64_t& data_size)
    {
      for(const std::unique_ptr<Node>& child : parent->children) {
        if(child->type == NodeType::File) {
          files++;
          data_size += child->data.size();
        } else if(child->type == NodeType::Folder) {
          folders++;
          count_entries(child.get(), files, folders, data_size);
        }
      }
    }

    /**
     * @brief Executes the file-system create command.
     * @return Process exit code.
     */
    int create_image()
    {
      try {
        std::filesystem::path image = options.create_image;
        TreeManager manager;
        manager.create_drive(image.filename().string(), image);
        std::cout << "File system image created: " << image.string() << std::endl;
        return 0;
      } catch(Error error) {
        return report_error(get_error_message(error));
      }
    }

    /**
     * @brief Executes the file-system list command.
     * @return Process exit code.
     */
    int list_image()
    {
      try {
        TreeManager manager;
        Node* drive = mount_image(manager, options.list_image);
        std::cout << drive->name << std::endl;
        print_tree(drive, 1);
        return 0;
      } catch(Error error) {
        return report_error(get_error_message(error));
      }
    }

    /**
     * @brief Executes the file-system add command.
     * @return Process exit code.
     */
    int add_file()
    {
      try {
        TreeManager manager;
        Node* drive = mount_image(manager, options.add_image);
        std::filesystem::path source = options.add_source;
        std::string destination = normalize_path(options.add_destination);
        std::string destination_name = source.filename().string();
        std::string parent_path = destination;

        Node* existing_destination = find_node(drive, destination);
        if(existing_destination != nullptr && existing_destination->type == NodeType::Folder) {
          parent_path = destination;
        } else if(!options.add_destination.empty() && options.add_destination.back() != '/' && options.add_destination.back() != '\\') {
          size_t separator = destination.find_last_of('/');
          destination_name = destination.substr(separator + 1);
          parent_path = separator == 0 ? "/" : destination.substr(0, separator);
        }

        Node* parent = create_folders(manager, drive, parent_path);
        Node* file = manager.add_entry(parent, source);
        if(file->name != destination_name) {
          manager.rename_entry(file, destination_name);
        }
        manager.save_drive(drive);
        std::cout << "File added: " << file->entry_path << std::endl;
        return 0;
      } catch(Error error) {
        return report_error(get_error_message(error));
      }
    }

    /**
     * @brief Executes the file-system mkdir command.
     * @return Process exit code.
     */
    int create_directory()
    {
      try {
        TreeManager manager;
        Node* drive = mount_image(manager, options.mkdir_image);
        Node* folder = create_folders(manager, drive, options.mkdir_path);
        manager.save_drive(drive);
        std::cout << "Directory created: " << folder->entry_path << std::endl;
        return 0;
      } catch(Error error) {
        return report_error(get_error_message(error));
      }
    }

    /**
     * @brief Executes the file-system extract command.
     * @return Process exit code.
     */
    int extract_file()
    {
      try {
        TreeManager manager;
        Node* drive = mount_image(manager, options.extract_image);
        Node* file = find_node(drive, options.extract_path);
        if(file == nullptr || file->type != NodeType::File) {
          throw Error::EntryNotFound;
        }
        std::filesystem::path output = options.extract_output.empty() ? file->name : options.extract_output;
        manager.extract_file(file, output);
        std::cout << "File extracted: " << output.string() << std::endl;
        return 0;
      } catch(Error error) {
        return report_error(get_error_message(error));
      }
    }

    /**
     * @brief Executes the file-system remove command.
     * @return Process exit code.
     */
    int remove_entry()
    {
      try {
        TreeManager manager;
        Node* drive = mount_image(manager, options.remove_image);
        Node* entry = find_node(drive, options.remove_path);
        if(entry == nullptr || entry == drive) {
          throw Error::EntryNotFound;
        }
        if(entry->type == NodeType::Folder && !entry->children.empty() && !options.remove_recursive) {
          return report_error("The directory is not empty. Use --recursive to remove it.");
        }
        manager.remove_entry(entry);
        manager.save_drive(drive);
        std::cout << "Entry removed: " << normalize_path(options.remove_path) << std::endl;
        return 0;
      } catch(Error error) {
        return report_error(get_error_message(error));
      }
    }

    /**
     * @brief Executes the file-system validate command.
     * @return Process exit code.
     */
    int validate_image()
    {
      try {
        TreeManager manager;
        Node* drive = mount_image(manager, options.validate_image);
        size_t files = 0;
        size_t folders = 0;
        uint64_t data_size = 0;
        count_entries(drive, files, folders, data_size);
        std::cout << termcolor::bright_green << "Valid RNBWFS image" << termcolor::reset << std::endl
                  << "Files:     " << files << std::endl
                  << "Folders:   " << folders << std::endl
                  << "Data size: " << data_size << " bytes" << std::endl;
        return 0;
      } catch(Error error) {
        return report_error(get_error_message(error));
      }
    }
  }

  /**
   * @brief Registers the file-system command tree in CLI11.
   * @param app Root CLI11 application.
   * @param command_result Receives the command exit code.
   */
  void register_file_system(CLI::App& app, int& command_result)
  {
    CLI::App* file_system = app.add_subcommand("filesystem", "Create and modify Rainbow file system images.");
    file_system->alias("fs");
    file_system->require_subcommand(1);

    CLI::App* create = file_system->add_subcommand("create", "Create an empty RNBWFS image.");
    create->add_option("image", options.create_image, "Image file to create")->required();
    create->callback([&command_result]() { command_result = create_image(); });

    CLI::App* list = file_system->add_subcommand("list", "Display the complete image tree.");
    list->add_option("image", options.list_image, "RNBWFS image")->required()->check(CLI::ExistingFile);
    list->callback([&command_result]() { command_result = list_image(); });

    CLI::App* add = file_system->add_subcommand("add", "Add a file to an image.");
    add->add_option("image", options.add_image, "RNBWFS image")->required()->check(CLI::ExistingFile);
    add->add_option("source", options.add_source, "Source file")->required()->check(CLI::ExistingFile);
    add->add_option("destination", options.add_destination, "Internal file or directory path");
    add->callback([&command_result]() { command_result = add_file(); });

    CLI::App* mkdir = file_system->add_subcommand("mkdir", "Create a directory and missing parents.");
    mkdir->add_option("image", options.mkdir_image, "RNBWFS image")->required()->check(CLI::ExistingFile);
    mkdir->add_option("path", options.mkdir_path, "Internal directory path")->required();
    mkdir->callback([&command_result]() { command_result = create_directory(); });

    CLI::App* extract = file_system->add_subcommand("extract", "Extract a file from an image.");
    extract->add_option("image", options.extract_image, "RNBWFS image")->required()->check(CLI::ExistingFile);
    extract->add_option("path", options.extract_path, "Internal file path")->required();
    extract->add_option("-o,--output", options.extract_output, "Destination file");
    extract->callback([&command_result]() { command_result = extract_file(); });

    CLI::App* remove = file_system->add_subcommand("remove", "Remove a file or directory from an image.");
    remove->add_option("image", options.remove_image, "RNBWFS image")->required()->check(CLI::ExistingFile);
    remove->add_option("path", options.remove_path, "Internal entry path")->required();
    remove->add_flag("-r,--recursive", options.remove_recursive, "Remove a non-empty directory");
    remove->callback([&command_result]() { command_result = remove_entry(); });

    CLI::App* validate = file_system->add_subcommand("validate", "Validate an RNBWFS image.");
    validate->add_option("image", options.validate_image, "RNBWFS image")->required()->check(CLI::ExistingFile);
    validate->callback([&command_result]() { command_result = validate_image(); });
  }
}
