#include <cstdint>
#include <stdio.h>
#include <fstream>
#include <string>
#include <cstring>
#include "TreeManager.h"

// functions declarations
node *create_empty(std::string drive, std::string label, std::string path, NODE_TYPES type);
node *build_tree(std::string filename, std::string name);
file_header get_file_header(std::ifstream *src_file);
int add_element(node *parent, node *elt);
void remove_element(node *arr, size_t index);
int get_node_index(node *arr, std::string path, NODE_TYPES type);
void release_node(node *node);

// initialize root node
node *root = create_empty("/", "root", "", NODE_TYPES::ROOT);

std::string clean_path(std::string path)
{
	// clean name
	while (path.find("//") != static_cast<size_t>(-1))
	{
		size_t p = path.find("//");
		path = path.substr(0, p) + path.substr(p + 1, path.length() - p + 1);
	}
	// check if path is valid
	if (path.at(0) != '/')
		path = "/" + path;
	if (path.at(path.length() - 1) == '/')
		path = path.substr(0, path.length() - 1);

	return path;
}

bool check_file_separator(std::istream *src_file)
{
	if (src_file->get() != 'F')
		return false;
	if (src_file->get() != '>')
		return false;

	return true;
}

bool check_file_header(std::istream *src_file)
{
	src_file->seekg(0);
	if (src_file->get() != 'R')
		return false;
	if (src_file->get() != 'N')
		return false;
	if (src_file->get() != 'B')
		return false;
	if (src_file->get() != 'W')
		return false;
	if (src_file->get() != 'F')
		return false;
	if (src_file->get() != 'S')
		return false;
	if (src_file->get() != 0x1a)
		return false;

	return true;
}

/**
 * @brief Get the tree object
 *
 * @return node*
 */
node *get_tree()
{
	return root;
}

/**
 * @brief Refresh tree by unmounting/mounting every drives
 *
 */
void refresh_tree()
{
	for (size_t i = 0; i < root->children_count; i++)
	{
		std::string drive_filename = root->children[i]->filename;
		std::string drive_name = root->children[i]->label;
		std::string drive_path = root->children[i]->path;
		unmount_drive(drive_path);
		mount_drive(drive_name, drive_filename);
	}
}

/**
 * @brief Rename a drive
 *
 * @param cur_node drive node
 * @param name new drive name
 */
void rename_drive(node *cur_node, std::string name)
{
	cur_node->drive = name;
	cur_node->label = name;
}

/**
 * @brief Create a new drive
 *
 * @param name drive name
 * @param filename file name to be used
 * @return true
 * @return false
 */
bool create_drive(std::string name, std::string filename)
{
	// create file
	std::ofstream dest_file(filename, std::fstream::binary);

	if (dest_file.fail())
	{
		throw TREE_MANAGER_ERRORS::FAILED_TO_CREATE_FILE;
	}

	// write header
	dest_file.put('R');
	dest_file.put('N');
	dest_file.put('B');
	dest_file.put('W');
	dest_file.put('F');
	dest_file.put('S');
	dest_file.put(0x1a);

	// write version
	dest_file.put(0x00);

	// close file
	dest_file.close();

	// mount drive
	try
	{
		mount_drive(name, filename);
	}
	catch (...)
	{
		throw;
	}

	return true;
}

/**
 * @brief Mount a new drive
 *
 * @param name drive name
 * @param filename file name to be used
 * @return true
 * @return false
 */
bool mount_drive(std::string name, std::string filename)
{
	// let's check if the file is already mounted under another name
	for (size_t i = 0; i < root->children_count; i++)
	{
		if (root->children[i]->filename == filename)
		{
			throw TREE_MANAGER_ERRORS::FILE_ALREADY_MOUNTED;
		}
	}

	// mount the drive/file
	node *new_node = NULL;
	try
	{
		new_node = build_tree(filename, name);
	}
	catch (...)
	{
		throw;
	}
	add_element(root, new_node);
	return true;
}

/**
 * @brief Unmount a drive
 *
 * @param name drive name
 */
void unmount_drive(std::string path)
{
	path = clean_path(path);

	// get drive index in root
	int driveIdx = get_node_index(root, path, NODE_TYPES::DRIVE);
	if (driveIdx < 0)
		return;

	// remove drive
	release_node(root->children[driveIdx]);
	remove_element(root, driveIdx);
}

/**
 * @brief Delete the passed file name and rename ./tmp.bin file to the passed name
 *
 * @param filename
 */
bool rename_tmp(std::string filename)
{
	remove(filename.c_str());
	if (std::rename("./tmp.bin", filename.c_str()) != 0)
		return false;

	return true;
}

/**
 * @brief Create a empty node
 *
 * @param drive drive name
 * @param label node label
 * @param type node type (root, drive, folder, file)
 * @return node*
 */
node *create_empty(std::string drive, std::string label, std::string path, NODE_TYPES type = NODE_TYPES::UNKNOWN)
{
	node *new_node = new node();
	new_node->type = type;
	new_node->drive = drive;
	new_node->label = label;
	new_node->path = path;
	return new_node;
}

/**
 * @brief Add an element to a node's children array in alphabetical order
 *
 * @param parent parent node
 * @param elt element to add
 * @return int
 */
int add_element(node *parent, node *elt)
{
	int index = -1;
	size_t size = parent->children_count;
	node **new_arr = (node **)malloc(sizeof(node *) * (size + 1));

	if (new_arr == NULL)
		return index;

	// loop throught children and check if we need to insert the new element
	for (size_t i = 0; i < size; i++)
	{
		if (strcmp(parent->children[i]->label.c_str(), elt->label.c_str()) < 0)
		{
			new_arr[i] = parent->children[i];
		}
		else
		{
			index = i;
			new_arr[i] = elt;
			for (size_t j = i; j < size; j++)
			{
				new_arr[j + 1] = (node *)parent->children[j];
			}
			break;
		}
	}

	// if it hasn't been added, add it to the end
	if (index == -1)
	{
		new_arr[size] = elt;
		index = size;
	}

	// update parent children count and array
	parent->children_count++;
	free(parent->children);
	parent->children = new_arr;

	return index;
}

/**
 * @brief Remove an elementfrom a node's children array
 *
 * @param parent
 * @param index
 */
void remove_element(node *parent, size_t index)
{
	size_t size = parent->children_count;
	if (size == 0)
		return;
	node **new_arr = (node **)malloc(sizeof(node *) * (size - 1));

	if (new_arr == NULL)
		return;

	size_t j = 0;
	for (size_t i = 0; i < size; i++)
	{
		if (i != index)
		{
			new_arr[j] = (node *)parent->children[i];
			j++;
		}
	}

	// update parent children count and array
	parent->children_count--;
	free(parent->children);
	parent->children = new_arr;
}

/**
 * @brief Find a node in another node's children
 *
 * @param parent parent node
 * @param label label to be found
 * @return node* returns NULL if not found
 */
node *get_node_by_label(node *parent, std::string label)
{
	for (size_t i = 0; i < parent->children_count; i++)
	{
		if (parent->children[i]->label == label)
			return parent->children[i];
	}
	return NULL;
}

/**
 * @brief Get index of a node in parent's children
 *
 * @param parent parent node
 * @param label label to be found
 * @param type node type
 * @return int returns 1 if not noud
 */
int get_node_index(node *parent, std::string path, NODE_TYPES type)
{
	// sanity check
	std::string full_path = clean_path(path);

	// do we need to add the drive to the path?
	if (path.substr(0, parent->path.length()) != parent->path)
		full_path = parent->path + path;

	for (size_t i = 0; i < parent->children_count; i++)
	{
		if (parent->children[i]->path == full_path && parent->children[i]->type == type)
			return i;
	}
	return -1;
}

/**
 * @brief
 *
 * @param parent
 * @param path
 * @return true
 * @return false
 */
bool path_exists_in_node(node *parent, std::string path)
{
	for (size_t i = 0; i < parent->children_count; i++)
	{
		// check current node
		if (parent->children[i]->path == path)
			return true;

		// do we need to check children?
		if (parent->children[i]->children_count != 0)
			if (path_exists_in_node(parent->children[i], path))
				return true;
	}

	return false;
}

/**
 * @brief Recursively add a node to a parent's children
 *
 * @param parent parent node
 * @param path full path to be parsed recursively
 * @param size file size (will be passed recursively until the file is met)
 */
void add_node(node *parent, std::string path, int size)
{
	// if a slash is found then it's a folder else it's a file
	size_t slash_pos = path.find("/", 1);

	// folder
	if (slash_pos != std::string::npos)
	{
		// check if folder already exists in the parent's children
		std::string sub_path = path.substr(0, slash_pos);
		int index = get_node_index(parent, sub_path, NODE_TYPES::FOLDER);
		if (index >= 0)
		{
			// add next node
			add_node((parent->children)[index], path.substr(slash_pos), size);
		}
		else
		{
			// create node and add next one
			std::string label = path.substr(1, slash_pos - 1);
			std::string full_path = parent->path + path.substr(0, slash_pos);
			node *new_node = create_empty(parent->drive, label, full_path, NODE_TYPES::FOLDER);
			int index = add_element(parent, new_node);
			add_node((parent->children)[index], path.substr(slash_pos), size);
		}
	}
	// file
	else
	{
		// create node
		std::string label = path.substr(1);
		std::string full_path = parent->path + path;
		node *new_node = create_empty(parent->drive, label, full_path, NODE_TYPES::FILE);
		new_node->size = size;
		add_element(parent, new_node);
	}
}

/**
 * @brief Delete a node (file or folder) from a drive
 *
 * @param cur_node
 * @return true
 * @return false
 */
bool delete_node(node *cur_node)
{
	// get source drive name and node
	std::string src_drive = cur_node->drive;
	node *src_drive_node = get_node_by_label(root, src_drive);
	if (src_drive_node == NULL)
		throw TREE_MANAGER_ERRORS::DRIVE_NOT_FOUND;

	// open source file
	std::ifstream src_file(src_drive_node->filename, std::fstream::binary);

	// error check
	if (src_file.fail())
		throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_SOURCE_FILE;

	// open temporary destination file
	std::ofstream tmp_file("./tmp.bin", std::fstream::binary);

	// error check
	if (tmp_file.fail())
		throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_TEMP_FILE;

	// copy header bytes 'RNBWFS' 0x1a 0x00
	for (size_t i = 0; i < 8; i++)
	{
		tmp_file.put(src_file.get());
	}

	// remove drive from file path
	std::string needle = cur_node->path.substr(cur_node->drive.length() + 1);

	// main loop
	while (src_file.peek() != EOF)
	{
		// check file separator
		if (!check_file_separator(&src_file))
			throw TREE_MANAGER_ERRORS::FILE_MALFORMED;

		// get file header
		file_header fh = get_file_header(&src_file);

		// is it what we're looking for?
		bool match = false;
		if (cur_node->type == NODE_TYPES::FILE)
		{
			// do we have a match?
			if (fh.name == needle)
				match = true;
		}
		else if (cur_node->type == NODE_TYPES::FOLDER)
		{
			// do we have a match?
			if (fh.name.length() > needle.length() && fh.name.substr(0, needle.length()) == needle)
				match = true;
		}

		// do we have a match?
		if (match)
		{
			// skip file
			src_file.seekg(fh.size, std::fstream::cur);
		}
		else
		{
			// copy file
			std::streampos off = -(7 + static_cast<std::streampos>(fh.name.length()));
			src_file.seekg(off, std::fstream::cur);
			size_t end = 7 + fh.name.length() + fh.size;
			for (size_t i = 0; i < end; i++)
			{
				tmp_file.put(src_file.get());
			}
		}
	}

	// close files
	tmp_file.close();
	src_file.close();

	// reload source drive
	std::string drive_filename = src_drive_node->filename;
	std::string drive_label = src_drive_node->label;
	std::string drive_path = src_drive_node->path;
	rename_tmp(drive_filename);
	unmount_drive(drive_path);
	try
	{
		mount_drive(drive_label, drive_filename);
	}
	catch (...)
	{
		throw;
	}

	return true;
}

/**
 * @brief Rename a node (file or folder) in a drive
 *
 * if the source drive and the destination drives are the same,
 * then we just need to rename the file in the drive file.
 *
 * if the source drive and the destination drives are different,
 * then we need to copy the file in the destination drive,
 * and remove it from the source file.
 *
 * @param cur_node
 * @param new_path
 * @return true
 * @return false
 */
bool rename_node(node *cur_node, std::string new_path)
{
	// sanity check
	new_path = clean_path(new_path);

	// get source drive name and node
	std::string src_drive = cur_node->drive;
	node *src_drive_node = get_node_by_label(root, src_drive);
	if (src_drive_node == NULL)
		throw TREE_MANAGER_ERRORS::DRIVE_NOT_FOUND;

	// open source file
	std::ifstream src_file(src_drive_node->filename, std::fstream::binary);

	// error check
	if (src_file.fail())
		throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_SOURCE_FILE;

	// remove drive from file path
	std::string needle = cur_node->path.substr(cur_node->drive.length() + 1);

	// check if the source drive is the same as the destination drive
	// and open the corresponding destination file
	std::string dest_drive = new_path.substr(1, new_path.find("/", 1) - 1);
	std::ofstream dest_file;
	std::ofstream *tmp_file;
	node *dest_drive_node = NULL;

	// source and destination drives are the same
	if (src_drive == dest_drive)
	{
		// check if destination drive already contains a file with the same path
		if (path_exists_in_node(src_drive_node, new_path))
			throw TREE_MANAGER_ERRORS::FILE_ALREADY_EXISTS;

		// open temporary destination file
		dest_file.open("./tmp.bin", std::fstream::binary);

		// error check
		if (dest_file.fail())
		{
			throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_DESTINATION_FILE;
		}

		// temporary file is the same as the destination file;
		tmp_file = &dest_file;

		// copy header bytes 'RNBWFS' 0x1a 0x00
		for (size_t i = 0; i < 8; i++)
		{
			dest_file.put(src_file.get());
		}
	}
	// source and destination drives are different
	else
	{
		// get destination drive node
		dest_drive_node = get_node_by_label(root, dest_drive);
		if (dest_drive_node == NULL)
			throw TREE_MANAGER_ERRORS::DRIVE_NOT_FOUND;

		// check if destination drive already contains a file with the same path
		if (path_exists_in_node(dest_drive_node, new_path))
			throw TREE_MANAGER_ERRORS::FILE_ALREADY_EXISTS;

		// open destination file
		dest_file.open(dest_drive_node->filename, std::fstream::app | std::fstream::binary);

		// error check
		if (dest_file.fail())
		{
			throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_DESTINATION_FILE;
		}

		// open temporary file to copy source file without the file
		tmp_file = new std::ofstream("./tmp.bin", std::fstream::binary);

		// error check
		if (tmp_file->fail())
		{
			throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_TEMP_FILE;
		}

		// copy header bytes 'RNBWFS' 0x1a 0x00
		for (size_t i = 0; i < 8; i++)
		{
			tmp_file->put(src_file.get());
		}
	}

	// main loop
	while (src_file.peek() != EOF)
	{
		// check file separator
		if (!check_file_separator(&src_file))
			throw TREE_MANAGER_ERRORS::FILE_MALFORMED;

		// get file header
		file_header fh = get_file_header(&src_file);

		// is it what we're looking for?
		bool match = false;
		std::string path;
		if (cur_node->type == NODE_TYPES::FILE)
		{
			// do we have a match?
			if (fh.name == needle)
			{
				match = true;

				// prepare new file name
				path = new_path.substr(new_path.find("/", 1));
			}
		}
		else if (cur_node->type == NODE_TYPES::FOLDER)
		{
			// do we have a match?
			if (fh.name.length() > needle.length() && fh.name.substr(0, needle.length()) == needle)
			{
				match = true;

				// prepare new file name
				path = new_path.substr(new_path.find("/", 1));
				path += fh.name.substr(needle.length());
			}
		}

		if (match)
		{
			// write file separator
			dest_file.put('F');
			dest_file.put('>');

			// put file name length
			dest_file.put(static_cast<char>(path.length()));

			// put file name
			for (size_t i = 0; i < path.length(); i++)
			{
				dest_file.put(path[i]);
			}

			// put file size + data
			src_file.seekg(-4, std::fstream::cur);
			for (size_t i = 0; i < 4 + fh.size; i++)
			{
				dest_file.put(src_file.get());
			}
		}
		// not the file we're looking for, just copy the data
		else
		{
			std::streampos off = -(7 + static_cast<std::streampos>(fh.name.length()));
			src_file.seekg(off, std::fstream::cur);
			for (size_t i = 0; i < 7 + fh.name.length() + fh.size; i++)
			{
				tmp_file->put(src_file.get());
			}
		}
	}

	// close files
	src_file.close();
	if (src_drive != dest_drive)
		tmp_file->close();
	dest_file.close();

	// reload source drive
	std::string drive_filename = src_drive_node->filename;
	std::string drive_label = src_drive_node->label;
	std::string drive_path = src_drive_node->path;
	rename_tmp(drive_filename);
	unmount_drive(drive_path);
	mount_drive(drive_label, drive_filename);

	// reload destination drive if needed
	if (src_drive != dest_drive && dest_drive_node != NULL)
	{
		drive_filename = dest_drive_node->filename;
		drive_label = dest_drive_node->label;
		drive_path = dest_drive_node->path;
		unmount_drive(drive_path);
		mount_drive(drive_label, drive_filename);
	}

	return true;
}

/**
 * @brief Decode file header (name and size)
 *
 * @param src_file
 * @return file_header
 */
file_header get_file_header(std::ifstream *src_file)
{
	// file cursor must be set at the beginning of a header
	file_header tmp_file_header;
	int name_length = src_file->get();

	// get file name
	for (int i = 0; i < name_length; i++)
	{
		tmp_file_header.name.push_back(src_file->get());
	}

	// get file size
	tmp_file_header.size = src_file->get() << 24;
	tmp_file_header.size |= src_file->get() << 16;
	tmp_file_header.size |= src_file->get() << 8;
	tmp_file_header.size |= src_file->get();

	return tmp_file_header;
}

/**
 * @brief Release/free a node
 *
 * @param node
 */
void release_node(node *node)
{

	for (size_t i = 0; i < node->children_count; i++)
	{
		release_node(node->children[i]);
	}
	free(node->children);
	free(node);
	node = NULL;
}

/**
 * @brief Build the tree of a passed file system file
 *
 * @param filename
 * @param name
 * @return node*
 */
node *build_tree(std::string filename, std::string name)
{
	// open source file
	std::ifstream src_file(filename.c_str(), std::fstream::binary);

	// error check
	if (src_file.fail())
		throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_SOURCE_FILE;

	// check file header
	if (!check_file_header(&src_file))
		throw TREE_MANAGER_ERRORS::FILE_FORMAT_UNKNOWN;

	// check version
	int version = src_file.get();
	if (version != 0)
		throw TREE_MANAGER_ERRORS::FILE_VERSION_NOT_SUPPORTED;

	// create drive root node
	node *parent = create_empty(name, name, "/" + name, NODE_TYPES::DRIVE);
	parent->filename = filename;

	// loop through file
	while (src_file.peek() != EOF)
	{
		// check file separator
		if (!check_file_separator(&src_file))
			throw TREE_MANAGER_ERRORS::FILE_MALFORMED;

		// get file header
		file_header fh = get_file_header(&src_file);

		// add node
		add_node(parent, fh.name, fh.size);

		// skip data
		src_file.seekg(fh.size, std::fstream::cur);
	}

	// close file
	src_file.close();

	// return tree root node
	return parent;
}

/**
 * @brief Add a file to a folder or drive root
 *
 * @param parent
 * @param filename
 * @return true
 * @return false
 */
bool add_file(node *parent, std::string filename)
{
	// get source drive name and node
	std::string drive = parent->drive;
	node *drive_node = get_node_by_label(root, drive);
	if (drive_node == NULL)
		throw TREE_MANAGER_ERRORS::DRIVE_NOT_FOUND;

	// open destination file
	std::fstream dest_file(drive_node->filename, std::fstream::in | std::fstream::out | std::fstream::app | std::fstream::binary);

	// error check
	if (dest_file.fail())
		throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_DESTINATION_FILE;

	// check file header
	if (!check_file_header(&dest_file))
		throw TREE_MANAGER_ERRORS::FILE_FORMAT_UNKNOWN;
	dest_file.seekg(std::fstream::end);

	// create path
	std::string path_new_file = parent->path.substr(parent->drive.length() + 1); // remove drive
	path_new_file += filename.substr(filename.rfind("/"));

	// check if file already exists
	if (get_node_index(drive_node, path_new_file, NODE_TYPES::FILE) != -1)
		throw TREE_MANAGER_ERRORS::FILE_ALREADY_EXISTS;

	// open new file to be added
	std::ifstream src_file(filename, std::fstream::ate | std::fstream::binary);

	// error check
	if (src_file.fail())
		throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_SOURCE_FILE;

	// get new file size
	std::streampos size = src_file.tellg();
	src_file.seekg(0);

	// write file separator
	dest_file.put('F');
	dest_file.put('>');

	// write file path length + path
	dest_file.put(static_cast<char>(path_new_file.length()));
	for (size_t i = 0; i < path_new_file.length(); i++)
	{
		dest_file.put(path_new_file[i]);
	}

	// write file size
	dest_file.put((size >> 24) & 0xff);
	dest_file.put((size >> 16) & 0xff);
	dest_file.put((size >> 8) & 0xff);
	dest_file.put(size & 0xff);

	// write file data
	while (src_file.peek() != EOF)
	{
		dest_file.put(src_file.get());
	}

	// close files
	dest_file.close();
	src_file.close();

	// reload drive
	std::string drive_filename = drive_node->filename;
	std::string drive_label = drive_node->label;
	std::string drive_path = drive_node->path;
	unmount_drive(drive_path);
	mount_drive(drive_label, drive_filename);

	return true;
}

/**
 * @brief Get a file content from a drive
 *
 * @param drive
 * @param path
 * @param data
 * @param data_size
 * @return true
 * @return false
 */
bool get_file_content(node *file_node, uint8_t **data, int *data_size)
{
	// check if passed node is a file
	if (file_node->type != NODE_TYPES::FILE)
		throw TREE_MANAGER_ERRORS::NOT_A_FILE;

	// get drive node
	node *drive_node = get_node_by_label(root, file_node->drive);
	if (drive_node == NULL)
		throw TREE_MANAGER_ERRORS::DRIVE_NOT_FOUND;

	// try to open file
	std::ifstream src_file(drive_node->filename, std::fstream::binary);

	// check if error
	if (src_file.fail())
		throw TREE_MANAGER_ERRORS::FAILED_TO_OPEN_SOURCE_FILE;

	// check file header
	if (!check_file_header(&src_file))
		throw TREE_MANAGER_ERRORS::FILE_FORMAT_UNKNOWN;

	// check version
	int version = src_file.get();
	if (version != 0)
		throw TREE_MANAGER_ERRORS::FILE_VERSION_NOT_SUPPORTED;

	// remove drive from path
	std::string path = file_node->path.substr(file_node->drive.length() + 1);

	// loop through file in file system
	while (src_file.peek() != EOF)
	{
		// check file separator
		if (!check_file_separator(&src_file))
			throw TREE_MANAGER_ERRORS::FILE_MALFORMED;

		// get file header
		file_header fh = get_file_header(&src_file);

		if (fh.name == path)
		{
			if (*data != NULL)
				free(*data);
			*data = (uint8_t *)malloc(fh.size);
			*data_size = fh.size;
			uint8_t *pData = (uint8_t *)*data;
			if (pData)
			{
				for (size_t i = 0; i < fh.size; i++)
				{
					*pData = src_file.get();
					pData++;
				}
			}
			return true;
		}
		else
		{
			src_file.seekg(fh.size, std::fstream::cur);
		}
	}

	throw TREE_MANAGER_ERRORS::FILE_NOT_FOUND;

	// close file
	src_file.close();
	return true;
}
