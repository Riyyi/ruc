/*
 * Copyright (C) 2021-2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdint> // int32_t
#include <filesystem>
#include <fstream> // std::ifstream, std::ofstream
#include <memory>  // std::make_shared, std::make_unique, std::shared_ptr
#include <string>
#include <string_view>

#include "ruc/file.h"
#include "ruc/meta/assert.h"

namespace ruc {

File::File(std::string_view path)
	: m_path(path)
{
	// Create input stream object and open file
	std::ifstream file(path.data());
	VERIFY(file.is_open(), "failed to open file: '{}'", path);

	// Get length of the file
	int32_t size = File::length(path, file);

	// Allocate memory filled with zeros
	auto buffer = std::make_unique<char[]>(size);

	// Fill buffer with file contents
	file.read(buffer.get(), size);
	file.close();

	m_data = std::string(buffer.get(), size);
}

File::~File()
{
}

// -----------------------------------------

File File::create(std::string_view path)
{
	if (!std::filesystem::exists(path)) {
		std::ofstream { path.data() };
	}

	return File(path);
}

int32_t File::length(std::string_view path, std::ifstream& file)
{
	file.seekg(0, std::ios::end);
	int32_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	VERIFY(length != -1, "failed to read file length: '{}'", path);

	return length;
}

// FIXME: Deduplicate code with constructor, this broke binary files only
std::shared_ptr<char[]> File::raw(std::string_view path)
{
	// Create input stream object and open file
	std::ifstream file(path.data());
	VERIFY(file.is_open(), "failed to open file: '{}'", path);

	// Get length of the file
	int32_t size = File::length(path, file);

	// Allocate memory filled with zeros
	auto buffer = std::make_shared<char[]>(size + 1);

	// Fill buffer with file contents
	file.read(buffer.get(), size);
	file.close();

	// Null termination
	buffer[size] = '\0';

	return buffer;
}

// -----------------------------------------

void File::clear()
{
	m_data.clear();
}

File& File::append(std::string_view data)
{
	m_data.append(data);

	return *this;
}

File& File::replace(size_t index, size_t length, std::string_view data)
{
	m_data.replace(index, length, data);

	return *this;
}

File& File::flush()
{
	// Create output stream object and open file
	std::ofstream file(m_path.data(), std::ios::out | std::ios::trunc);
	VERIFY(file.is_open(), "failed to open file: '{}'", m_path);

	// Write data to disk
	file.write(m_data.c_str(), m_data.size());

	return *this;
}

} // namespace ruc
