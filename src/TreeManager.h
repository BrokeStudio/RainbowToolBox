#pragma once

#include <string>

enum class TREE_MANAGER_ERRORS
{
	DRIVE_NOT_FOUND,
	FILE_ALREADY_MOUNTED,
	FILE_ALREADY_EXISTS,
	FILE_NOT_FOUND,
	FAILED_TO_CREATE_FILE,
	FAILED_TO_OPEN_SOURCE_FILE,
	FAILED_TO_OPEN_DESTINATION_FILE,
	FAILED_TO_OPEN_TEMP_FILE,
	FILE_FORMAT_UNKNOWN,
	FILE_VERSION_NOT_SUPPORTED,
	FILE_MALFORMED,
	NOT_A_FILE,
	COUNT
};

const char *const tree_manager_errors_text[static_cast<int>(TREE_MANAGER_ERRORS::COUNT)] = {
		"Drive not found",
		"File already mounted.",
		"File already exists.",
		"File not found.",
		"Failed to create file.",
		"Failed to open source file.",
		"Failed to open destination file.",
		"Failed to open temporary file.",
		"File format unknown.",
		"File version not supported.",
		"File malformed.",
		"Node is not a file.",
};

typedef struct file_header
{
	std::string name = "";
	size_t size = 0;
} file_header;

enum class NODE_TYPES
{
	UNKNOWN,
	ROOT,
	DRIVE,
	FOLDER,
	FILE,
	COUNT
};

const char *const node_types[static_cast<int>(NODE_TYPES::COUNT)] = {
		"Unknown",
		"Root",
		"Drive",
		"Folder",
		"File",
};

typedef struct node
{
	NODE_TYPES type = NODE_TYPES::UNKNOWN;
	std::string drive = "";
	std::string label = "";
	std::string path = "";
	int size = -1;
	std::string filename = "";
	node **children = NULL;
	size_t children_count = 0;
} node;

node *get_tree();

void refresh_tree();

bool create_drive(std::string name, std::string filename);
bool mount_drive(std::string name, std::string filename);
void rename_drive(node *cur_node, std::string name);
void unmount_drive(std::string name);

bool add_file(node *parent, std::string filename);
bool delete_node(node *file_node);
bool rename_node(node *file_node, std::string new_path);

bool get_file_content(node *file_node, uint8_t **data, int *data_size);
