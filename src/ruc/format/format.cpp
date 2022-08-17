/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#include <sstream> // stringstream
#include <string>
#include <string_view>

#include "ruc/format/builder.h"
#include "ruc/format/format.h"
#include "ruc/format/parser.h"
#include "ruc/meta/assert.h"

namespace ruc::format {

void variadicFormatImpl(Builder& builder, Parser& parser, TypeErasedParameters& parameters)
{
	// Consume everything until '{' or EOF
	const auto literal = parser.consumeLiteral();
	builder.putLiteral(literal);

	// Reached end of format string
	if (parser.isEOF()) {
		return;
	}

	// Consume index + ':'
	auto indexMaybe = parser.consumeIndex();

	// Get parameter at index, or next
	size_t index = indexMaybe.has_value() ? indexMaybe.value() : parameters.tell();
	VERIFY(index < parameters.size(), "argument not found at index '%zu'", index);
	auto& parameter = parameters.parameter(index);

	// Format the parameter
	parameter.format(builder, parser, parameter.value);

	// Go to next parameter
	parameters.ignore();

	// Recurse
	if (!parser.isEOF()) {
		variadicFormatImpl(builder, parser, parameters);
	}
}

void variadicFormat(std::stringstream& stream, std::string_view format, TypeErasedParameters& parameters)
{
	Builder builder { stream };
	Parser parser { format, parameters.size() };

	variadicFormatImpl(builder, parser, parameters);
}

// -----------------------------------------

FormatOperatorStyle::FormatOperatorStyle(std::string& output)
	: m_output(output)
	, m_stream()
	, m_builder(m_stream)
{
}

FormatOperatorStyle::~FormatOperatorStyle()
{
	m_output = m_stream.str();
}

FormatOperatorStyle formatTo(std::string& output)
{
	return FormatOperatorStyle(output);
}

} // namespace ruc::format
