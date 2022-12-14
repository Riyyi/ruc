/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <algorithm> // transform
#include <cstddef>   // nullptr_t, size_t
#include <cstdint>   // int32_t, int64_t, uint32_t
#include <map>
#include <string>
#include <unordered_map>
#include <utility> // forward
#include <vector>

#include "ruc/json/array.h"
#include "ruc/json/object.h"
#include "ruc/meta/assert.h"
#include "ruc/meta/concepts.h"
#include "ruc/meta/odr.h"

namespace ruc::json {

namespace detail {

// Required for containers with Json::Value type
template<typename Json>
void fromJson(const Json& json, Json& value)
{
	value = json;
}

template<typename Json>
void fromJson(const Json& json, std::nullptr_t& null)
{
	VERIFY(json.type() == Json::Type::Null);
	null = nullptr;
}

template<typename Json>
void fromJson(const Json& json, bool& boolean)
{
	VERIFY(json.type() == Json::Type::Bool);
	boolean = json.asBool();
}

template<typename Json, Integral T>
void fromJson(const Json& json, T& number)
{
	VERIFY(json.type() == Json::Type::Number);
	number = static_cast<T>(json.asDouble());
}

template<typename Json, FloatingPoint T>
void fromJson(const Json& json, T& number)
{
	VERIFY(json.type() == Json::Type::Number);
	number = json.asDouble();
}

template<typename Json>
void fromJson(const Json& json, std::string& string)
{
	VERIFY(json.type() == Json::Type::String);
	string = json.asString();
}

template<typename Json, typename T>
void fromJson(const Json& json, std::vector<T>& array)
{
	VERIFY(json.type() == Json::Type::Array);
	array.resize(json.size());
	std::transform(
		json.asArray().elements().begin(),
		json.asArray().elements().end(),
		array.begin(),
		[](const Json& json) {
			return json.template get<T>(); // (missing-dependent-template-keyword)
		});
}

template<typename Json, typename T>
void fromJson(const Json& json, std::map<std::string, T>& object)
{
	VERIFY(json.type() == Json::Type::Object);
	object.clear();
	for (const auto& [name, value] : json.asObject().members()) {
		object.emplace(name, value.template get<T>());
	}
}

template<typename Json, typename T>
void fromJson(const Json& json, std::unordered_map<std::string, T>& object)
{
	VERIFY(json.type() == Json::Type::Object);
	object.clear();
	for (const auto& [name, value] : json.asObject().members()) {
		object.emplace(name, value.template get<T>());
	}
}

struct fromJsonFunction {
	template<typename Json, typename T>
	auto operator()(const Json& json, T&& value) const
	{
		return fromJson(json, std::forward<T>(value));
	}
};

} // namespace detail

// Anonymous namespace prevents multiple definition of the reference
namespace {
// Function object
constexpr const auto& fromJson = ruc::detail::staticConst<detail::fromJsonFunction>; // NOLINT(misc-definitions-in-headers,clang-diagnostic-unused-variable)
} // namespace

} // namespace ruc::json

// Customization Points
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4381.html

// Json::fromJson is a function object, the type of which is
// Json::Detail::fromJsonFunction. In the Json::Detail namespace are the
// fromJson free functions. The function call operator of fromJsonFunction makes
// an unqualified call to fromJson which, since it shares the Detail namespace
// with the fromJson free functions, will consider those in addition to any
// overloads that are found by argument-dependent lookup.

// Variable templates are linked externally, therefor every translation unit
// will see the same address for Detail::staticConst<Detail::fromJsonFunction>.
// Since Json::fromJson is a reference to the variable template, it too will
// have the same address in all translation units.
