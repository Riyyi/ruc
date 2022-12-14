/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>  // FILE, fflush, fputs, stdout, stderr
#include <iomanip> // setprecision
#include <ios>     // defaultfloat, fixed
#include <sstream> // stringstream
#include <string>
#include <string_view>

#include "ruc/format/print.h"

namespace ruc::format {

void variadicPrint(FILE* file, std::string_view format, TypeErasedParameters& parameters)
{
	std::stringstream stream;
	variadicFormat(stream, format, parameters);

	std::string string = stream.str();
	fputs(string.c_str(), file);
	fflush(file);
}

// -----------------------------------------

PrintOperatorStyle::PrintOperatorStyle(FILE* file)
	: m_file(file)
	, m_stream()
	, m_builder(m_stream)
{
}

PrintOperatorStyle::~PrintOperatorStyle()
{
	std::string string = m_stream.str();
	fputs(string.c_str(), m_file);
}

PrintOperatorStyle print()
{
	return PrintOperatorStyle(stdout);
}

PrintOperatorStyle print(FILE* file)
{
	return PrintOperatorStyle(file);
}

} // namespace ruc::format
