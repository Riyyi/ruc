/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#include "ruc/json/array.h"
#include "ruc/json/value.h"

namespace ruc::json {

void Array::emplace_back(Value element)
{
	m_elements.emplace_back(std::move(element));
}

Value& Array::operator[](size_t index)
{
	if (index + 1 > m_elements.size()) {
		m_elements.resize(index + 1);
	}

	return m_elements[index];
}

} // namespace ruc::json
