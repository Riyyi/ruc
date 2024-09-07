/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm> // min
#include <charconv>  // std::to_chars
#include <cstddef>   // size_t
#include <cstdint>   // uint8_t, uint16_t, uint32_t, uint64_t
#include <iomanip>   // setprecision
#include <ios>       // defaultfloat, fixed
#include <limits>    // numeric_limits
#include <sstream>   // stringstream
#include <string>
#include <string_view>

#include "ruc/format/builder.h"
#include "ruc/format/parser.h"
#include "ruc/meta/assert.h"

namespace ruc::format {

void Builder::putLiteral(std::string_view literal)
{
	for (size_t i = 0; i < literal.length(); ++i) {
		putCharacter(literal[i]);
		if (literal[i] == '{' || literal[i] == '}') {
			++i;
		}
	}
}

static constexpr std::string numberToString(size_t value, uint8_t base, bool uppercase)
{
	std::string result;

	if (value > std::numeric_limits<uint32_t>::max()) {
		result.reserve(64);
	}
	else if (value > std::numeric_limits<uint16_t>::max()) {
		result.reserve(32);
	}

	constexpr const auto& lookupLowercase = "0123456789abcdef";
	constexpr const auto& lookupUppercase = "0123456789ABCDEF";

	if (value == 0) {
		result = '0';
	}
	else if (uppercase) {
		while (value > 0) {
			result.insert(0, 1, lookupUppercase[value % base]);
			value /= base;
		}
	}
	else {
		while (value > 0) {
			result.insert(0, 1, lookupLowercase[value % base]);
			value /= base;
		}
	}

	return result;
}

void Builder::putU64(size_t value,
                     uint8_t base,
                     bool uppercase,
                     char fill,
                     Align align,
                     Sign sign,
                     bool alternativeForm,
                     bool zeroPadding,
                     size_t width,
                     bool isNegative) const
{
	std::string number = numberToString(value, base, uppercase);

	// Sign
	std::string prefix = "";
	switch (sign) {
	case Sign::None:
	case Sign::Negative:
		if (isNegative) {
			prefix = '-';
		}
		break;
	case Sign::Both:
		prefix = (isNegative) ? '-' : '+';
		break;
	case Sign::Space:
		prefix = (isNegative) ? '-' : ' ';
		break;
	default:
		VERIFY_NOT_REACHED();
	};

	// Alternative form
	if (alternativeForm) {
		switch (base) {
		case 2:
			prefix += (uppercase) ? "0B" : "0b";
			break;
		case 8:
			prefix += '0';
			break;
		case 10:
			break;
		case 16:
			prefix += (uppercase) ? "0X" : "0x";
			break;
		default:
			VERIFY_NOT_REACHED();
		}
	}

	if (!zeroPadding) {
		number.insert(0, prefix);
	}
	else {
		if (align != Builder::Align::None) {
			number.insert(0, prefix);
		}
		fill = '0';
	}

	size_t length = number.length();
	if (width < length) {
		m_builder.write(number.data(), length);
		return;
	}

	switch (align) {
	case Align::Left:
		m_builder.write(number.data(), length);
		m_builder << std::string(width - length, fill);
		break;
	case Align::Center: {
		size_t half = (width - length) / 2;
		m_builder << std::string(half, fill);
		m_builder.write(number.data(), length);
		m_builder << std::string(width - half - length, fill);
		break;
	}
	case Align::Right:
		m_builder << std::string(width - length, fill);
		m_builder.write(number.data(), length);
		break;
	case Align::None:
		if (zeroPadding) {
			m_builder << prefix;
			m_builder << std::string(width - length - prefix.length(), fill);
		}
		else {
			m_builder << std::string(width - length, fill);
		}
		m_builder.write(number.data(), length);
		break;
	default:
		VERIFY_NOT_REACHED();
	};
}

void Builder::putI64(int64_t value,
                     uint8_t base,
                     bool uppercase,
                     char fill,
                     Align align,
                     Sign sign,
                     bool alternativeForm,
                     bool zeroPadding,
                     size_t width) const
{
	bool isNegative = value < 0;
	value = isNegative ? -value : value;
	putU64(static_cast<uint64_t>(value), base, uppercase, fill, align, sign, alternativeForm, zeroPadding, width, isNegative);
}

void Builder::putF64(double number, uint8_t precision) const
{
	char buffer[512];
	auto conversion = std::to_chars(buffer, buffer + sizeof(buffer), number);
	auto converted = std::string(buffer, conversion.ptr - buffer);

	size_t dot = converted.find('.');
	size_t length = dot + precision + 1;

	// There is no number behind the decimal point
	if (dot == std::string::npos) {
		if (precision > 0) {
			converted += '.' + std::string(precision, '0');
		}
		m_builder << converted;
		return;
	}

	// If there are less numbers behind the decimal point than the precision
	if (converted.length() < length) {
		converted += std::string(length - converted.length(), '0');
		m_builder << converted;
		return;
	}

	// Only round if there are more numbers behind the decimal point than the precision,
	// or the number that comes after the maximum precision is higher than 4
	if (converted.length() > length && converted[length] > '4') {
		for (size_t i = length - 1; i >= 0 && i < converted.length(); --i) {
			if (converted[i] == '.') {
				continue;
			}
			if (converted[i] < '9') { // Overflow stops here
				converted[i]++;
				break;
			}
			converted[i] = '0';
		}
	}

	// Cut off the characters after the requested precision
	if (converted.length() > length) {
		size_t last_included_number = converted.find_last_not_of("0", length - 1);
		// If precision is zero, also cut off the '.', otherwise include the '0' after the '.'
		size_t last_character_is_dot_offset = (converted[last_included_number] == '.') ? (precision > 0 ? 1 : -1) : 0;
		converted = converted.substr(0, last_included_number + last_character_is_dot_offset + 1);
	}

	m_builder << converted;
}

void Builder::putString(std::string_view string, char fill, Align align, size_t width) const
{
	size_t length = string.length();
	if (width < length) {
		m_builder.write(string.data(), length);
		return;
	}

	switch (align) {
	case Align::None:
	case Align::Left:
		m_builder.write(string.data(), length);
		m_builder << std::string(width - length, fill);
		break;
	case Align::Center: {
		size_t half = (width - length) / 2;
		m_builder << std::string(half, fill);
		m_builder.write(string.data(), length);
		m_builder << std::string(width - half - length, fill);
		break;
	}
	case Align::Right:
		m_builder << std::string(width - length, fill);
		m_builder.write(string.data(), length);
		break;
	default:
		VERIFY_NOT_REACHED();
	};
}

} // namespace ruc::format
