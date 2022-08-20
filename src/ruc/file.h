/*
 * Copyright (C) 2021-2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <string_view>

namespace ruc {

class File {
public:
	File(std::string_view path);
	virtual ~File();

	static File create(std::string_view path);

	void clear();
	File& append(std::string_view data);
	File& replace(size_t index, size_t length, std::string_view data);
	File& flush();

	const char* c_str() const { return m_data.c_str(); }
	const std::string& data() const { return m_data; }
	std::string_view path() const { return m_path; }

private:
	std::string_view m_path;
	std::string m_data;
};

} // namespace ruc
