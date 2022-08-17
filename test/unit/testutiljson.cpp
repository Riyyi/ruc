/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstddef>    // nullptr_t
#include <cstdint>    // uint32_t
#include <functional> // function
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "macro.h"
#include "ruc/json/array.h"
#include "ruc/json/job.h"
#include "ruc/json/json.h"
#include "ruc/json/lexer.h"
#include "ruc/json/parser.h"
#include "ruc/json/serializer.h"
#include "testcase.h"
#include "testsuite.h"

#define DONT_PRINT_PARSER_ERRORS

#ifndef DONT_PRINT_PARSER_ERRORS
	#define EXEC(x) x
#else
	#define EXEC(x)                                   \
		stderr = test::TestSuite::the().outputNull(); \
		x;                                            \
		stderr = test::TestSuite::the().outputErr();
#endif

std::vector<ruc::json::Token> lex(const std::string& input)
{
	EXEC(
		ruc::json::Job job(input);
		ruc::json::Lexer lexer(&job);
		lexer.analyze(););
	return *job.tokens();
}

ruc::Json parse(const std::string& input)
{
	EXEC(
		ruc::json::Job job(input);
		ruc::json::Lexer lexer(&job);
		lexer.analyze(););

	if (!job.success()) {
		return nullptr;
	}

	EXEC(
		ruc::json::Parser parser(&job);
		ruc::Json json = parser.parse(););

	if (!job.success()) {
		return nullptr;
	}

	return json;
}

std::string serialize(const std::string& input, uint32_t indent = 0)
{
	EXEC(
		auto json = ruc::Json::parse(input););
	return json.dump(indent);
}

// -----------------------------------------

TEST_CASE(JsonLexer)
{
	std::vector<ruc::json::Token> tokens;

	// Literal

	tokens = lex("true");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "true");
	EXPECT(tokens[0].type == ruc::json::Token::Type::Literal);

	tokens = lex("false");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "false");
	EXPECT(tokens[0].type == ruc::json::Token::Type::Literal);

	tokens = lex("null");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "null");
	EXPECT(tokens[0].type == ruc::json::Token::Type::Literal);

	// Number

	tokens = lex("3.14");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "3.14");
	EXPECT(tokens[0].type == ruc::json::Token::Type::Number);

	tokens = lex("-3.14e+2");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "-3.14e+2");
	EXPECT(tokens[0].type == ruc::json::Token::Type::Number);

	tokens = lex("+3.14");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "+");
	EXPECT(tokens[0].type == ruc::json::Token::Type::None);

	// String

	tokens = lex(R"("a string")");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "a string");
	EXPECT(tokens[0].type == ruc::json::Token::Type::String);

	tokens = lex(R"("a string""another string")");
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0].symbol, "a string");
	EXPECT_EQ(tokens[1].symbol, "another string");
	EXPECT(tokens[0].type == ruc::json::Token::Type::String);

	tokens = lex("\"a string\nwill break on the newline symbol\"");
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_EQ(tokens[0].symbol, "a string");
	EXPECT(tokens[0].type == ruc::json::Token::Type::String);

	// Array

	tokens = lex("[]");
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0].symbol, "[");
	EXPECT_EQ(tokens[1].symbol, "]");

	tokens = lex("[\n\n\n]");
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0].symbol, "[");
	EXPECT_EQ(tokens[1].symbol, "]");

	// Object

	tokens = lex("{}");
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0].symbol, "{");
	EXPECT_EQ(tokens[1].symbol, "}");

	tokens = lex("{\n\n\n}");
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0].symbol, "{");
	EXPECT_EQ(tokens[1].symbol, "}");
}

TEST_CASE(JsonParser)
{
	ruc::Json json;

	json = parse("null");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("true");
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::Bool);

	json = parse("false");
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::Bool);

	json = parse("3.14");
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::Number);

	json = parse(R"("a string")");
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::String);

	// Array

	json = parse("[");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("[ 123");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("[ 123,");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("[ 123, ]");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("[ 123 456 ]");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("[]");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Array);

	json = parse(R"([ "element", 3.14 ])");
	EXPECT_EQ(json.size(), 2);
	EXPECT_EQ(json.type(), ruc::Json::Type::Array);

	// Object

	json = parse("{");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ "name")");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ "name":)");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ "name":,)");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ "name":"value")");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ "name":"value",)");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ "name":"value", })");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ "name" "value" })");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse(R"({ 123 })");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("{}");
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Object);

	json = parse(R"({ "name": "value", "name2": 3.14 })");
	EXPECT_EQ(json.size(), 2);
	EXPECT_EQ(json.type(), ruc::Json::Type::Object);

	// Multiple root elements

	json = parse("54 false");
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("3.14, 666");
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = parse("true\nfalse");
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);
}

TEST_CASE(JsonToJsonValue)
{
	ruc::Json json;

	json = {};
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = nullptr;
	EXPECT_EQ(json.size(), 0);
	EXPECT_EQ(json.type(), ruc::Json::Type::Null);

	json = true;
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::Bool);

	json = false;
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::Bool);

	json = 666;
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::Number);

	json = 3.14;
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::Number);

	const char* characters = "my string";
	json = characters;
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::String);

	std::string string = "my string";
	json = string;
	EXPECT_EQ(json.size(), 1);
	EXPECT_EQ(json.type(), ruc::Json::Type::String);

	// Nested Array with multiple types
	json = { "element", 3.14, true, nullptr, { "nested element", { "more nesting", { 1, 2, 3, "yes" } } } };
	EXPECT_EQ(json.size(), 5);
	EXPECT_EQ(json.type(), ruc::Json::Type::Array);

	// Nested Object with multiple types
	json = { { "name", "value" }, { "name2", 3.14 }, { "name3", true }, { "name4", nullptr }, { "name5", { { "nested name", "value" } } } };
	EXPECT_EQ(json.size(), 5);
	EXPECT_EQ(json.type(), ruc::Json::Type::Object);

	// Array with singular type
	std::vector<std::string> vector = { "element", "element2", "element3" };
	json = vector;
	EXPECT_EQ(json.size(), 3);
	EXPECT_EQ(json.type(), ruc::Json::Type::Array);

	// Object with singular type
	std::map<std::string, std::string> map = { { "name", "value" }, { "name2", "value2" } };
	json = map;
	EXPECT_EQ(json.size(), 2);
	EXPECT_EQ(json.type(), ruc::Json::Type::Object);

	// Object with singular type
	std::unordered_map<std::string, std::string> unorderedMap = { { "name", "value" }, { "name2", "value2" } };
	json = unorderedMap;
	EXPECT_EQ(json.size(), 2);
	EXPECT_EQ(json.type(), ruc::Json::Type::Object);
}

TEST_CASE(JsonFromJsonValue)
{
	ruc::Json json;

	json = nullptr;
	EXPECT_EQ(json.get<std::nullptr_t>(), nullptr);

	json = true;
	EXPECT_EQ(json.get<bool>(), true);

	json = false;
	EXPECT_EQ(json.get<bool>(), false);

	json = 666;
	EXPECT_EQ(json.get<int>(), 666);

	json = 3.14;
	EXPECT_EQ(json.get<double>(), 3.14);

	std::string string;
	json = "my string";
	json.getTo(string);
	EXPECT_EQ(string, "my string");
	EXPECT_EQ(json.get<std::string>(), "my string");

	// Array with singular type
	json = { "element", "element2" };
	EXPECT_EQ(json[0].get<std::string>(), "element");
	EXPECT_EQ(json.at(1).get<std::string>(), "element2");
	auto array = json.get<std::vector<std::string>>();
	EXPECT_EQ(array.size(), 2);
	EXPECT_EQ(array[0], "element");
	EXPECT_EQ(array[1], "element2");

	// Array with multiple types
	json = { "string", 3.14, true, nullptr };
	EXPECT_EQ(json[0].get<std::string>(), "string");
	EXPECT_EQ(json.at(1).get<double>(), 3.14);
	EXPECT_EQ(json[2].get<bool>(), true);
	EXPECT_EQ(json[3].get<std::nullptr_t>(), nullptr);
	auto valueArray = json.get<std::vector<ruc::Json>>();
	EXPECT_EQ(valueArray.size(), 4);
	EXPECT_EQ(valueArray[0].get<std::string>(), "string");
	EXPECT_EQ(valueArray[1].get<double>(), 3.14);
	EXPECT_EQ(valueArray[2].get<bool>(), true);
	EXPECT_EQ(valueArray[3].get<std::nullptr_t>(), nullptr);

	// Nested Array with multiple types
	json = {
		"value",
		{
			"thing",
			666,
		},
		{
			{
				3.14,
			},
		}
	};
	EXPECT_EQ(json[0].get<std::string>(), "value");
	EXPECT_EQ(json.at(1)[0].get<std::string>(), "thing");
	EXPECT_EQ(json[1].at(1).get<int>(), 666);
	EXPECT_EQ(json[2][0][0].get<double>(), 3.14);

	// Object with singular type
	json = { { "name", "value" }, { "name2", "value2" } };
	EXPECT_EQ(json["name"].get<std::string>(), "value");
	EXPECT_EQ(json.at("name2").get<std::string>(), "value2");
	auto object = json.get<std::map<std::string, std::string>>();
	EXPECT_EQ(object.size(), 2);
	EXPECT_EQ(object["name"], "value");
	EXPECT_EQ(object["name2"], "value2");
	auto unorderedObject = json.get<std::unordered_map<std::string, std::string>>();
	EXPECT_EQ(unorderedObject.size(), 2);
	EXPECT_EQ(unorderedObject["name"], "value");
	EXPECT_EQ(unorderedObject["name2"], "value2");

	// Object with multiple types
	json = { { "name", "value" }, { "name2", 3.14 }, { "name3", true }, { "name4", nullptr } };
	EXPECT_EQ(json["name"].get<std::string>(), "value");
	EXPECT_EQ(json.at("name2").get<double>(), 3.14);
	EXPECT_EQ(json["name3"].get<bool>(), true);
	EXPECT_EQ(json["name4"].get<std::nullptr_t>(), nullptr);
	auto valueObject = json.get<std::map<std::string, ruc::Json>>();
	EXPECT_EQ(valueObject.size(), 4);
	EXPECT_EQ(valueObject["name"].get<std::string>(), "value");
	EXPECT_EQ(valueObject["name2"].get<double>(), 3.14);
	EXPECT_EQ(valueObject["name3"].get<bool>(), true);
	EXPECT_EQ(valueObject["name4"].get<std::nullptr_t>(), nullptr);

	// Nested Object with multiple types
	json = {
		{
			"name",
			"value",
		},
		{
			"nest 1-deep",
			{ {
				"number",
				1,
			} },
		},
		{
			"nest 2-deep",
			{ {
				"nest 1-deep",
				{ {
					"bool",
					true,
				} },
			} },
		},
	};
	EXPECT_EQ(json["name"].get<std::string>(), "value");
	EXPECT_EQ(json["nest 1-deep"]["number"].get<int>(), 1);
	EXPECT_EQ(json["nest 2-deep"]["nest 1-deep"]["bool"].get<bool>(), true);
}

TEST_CASE(JsonImplicitConversion)
{
	ruc::Json array;
	array[0];
	EXPECT_EQ(array.type(), ruc::Json::Type::Array);

	ruc::Json arrayEmplace;
	arrayEmplace.emplace_back("element");
	arrayEmplace.emplace_back({ "nested element" });
	EXPECT_EQ(arrayEmplace.type(), ruc::Json::Type::Array);
	EXPECT_EQ(arrayEmplace[1].type(), ruc::Json::Type::Array);

	ruc::Json object;
	object[""];
	EXPECT_EQ(object.type(), ruc::Json::Type::Object);

	ruc::Json objectEmplace;
	objectEmplace.emplace("name", "value");
	objectEmplace.emplace("name2", { { "nested name", "value" } });
	EXPECT_EQ(objectEmplace.type(), ruc::Json::Type::Object);
	EXPECT_EQ(objectEmplace["name2"].type(), ruc::Json::Type::Object);
}

TEST_CASE(JsonSerializer)
{
	EXPECT_EQ(serialize(""), "null");
	EXPECT_EQ(serialize("null"), "null");
	EXPECT_EQ(serialize("true"), "true");
	EXPECT_EQ(serialize("false"), "false");
	EXPECT_EQ(serialize("3.14"), "3.14");
	EXPECT_EQ(serialize(R"("string")"), R"("string")");

	EXPECT_EQ(serialize("\n\n\n"), "null");
	EXPECT_EQ(serialize("null\n"), "null");
	EXPECT_EQ(serialize("true\n"), "true");
	EXPECT_EQ(serialize("false\n"), "false");
	EXPECT_EQ(serialize("3.14\n"), "3.14");
	// clang-format off
	EXPECT_EQ(serialize(R"("string")" "\n"), R"("string")");
	// clang-format on

	EXPECT_EQ(serialize("[\n\n\n]"), "[]");
	EXPECT_EQ(serialize("[null]"), "[null]");
	EXPECT_EQ(serialize("[true]"), "[true]");
	EXPECT_EQ(serialize("[false]"), "[false]");
	EXPECT_EQ(serialize("[3.14]"), "[3.14]");
	EXPECT_EQ(serialize(R"(["string"])"), R"(["string"])");

	EXPECT_EQ(serialize("[\n\n\n]", 4), "[\n]");
	EXPECT_EQ(serialize("[null]", 4), "[\n    null\n]");
	EXPECT_EQ(serialize("[true]", 4), "[\n    true\n]");
	EXPECT_EQ(serialize("[false]", 4), "[\n    false\n]");
	EXPECT_EQ(serialize("[3.14]", 4), "[\n    3.14\n]");
	// clang-format off
	EXPECT_EQ(serialize(R"(["string"])", 4), "[\n    " R"("string")" "\n]");
	// clang-format on

	// Check for trailing comma on last array element
	EXPECT_EQ(serialize(R"([1])"), R"([1])");
	EXPECT_EQ(serialize(R"([1,2])"), R"([1,2])");
	EXPECT_EQ(serialize(R"([1,2,3])"), R"([1,2,3])");

	// Check for trailing comma on last object member
	EXPECT_EQ(serialize(R"({"n1":"v1"})"), R"({"n1":"v1"})");
	EXPECT_EQ(serialize(R"({"n1":"v1", "n2":"v2"})"), R"({"n1":"v1","n2":"v2"})");
	EXPECT_EQ(serialize(R"({"n1":"v1", "n2":"v2", "n3":"v3"})"), R"({"n1":"v1","n2":"v2","n3":"v3"})");

	// clang-format off
	EXPECT_EQ(serialize(R"({
	"object member one": [
		"array element one"
	],
	"object member two": [
		"array element one",
		"array element two"
	],
	"object member three": [
		"array element one",
		2,
		3.0,
		4.56,
		true,
		false,
		null
	],
	"object member four": 3.14,
	"object member five": "value five",
	"object member six": null,
	"object member seven": { "no": 0 }
})", 4), R"({
    "object member five": "value five",
    "object member four": 3.14,
    "object member one": [
        "array element one"
    ],
    "object member seven": {
        "no": 0
    },
    "object member six": null,
    "object member three": [
        "array element one",
        2,
        3,
        4.56,
        true,
        false,
        null
    ],
    "object member two": [
        "array element one",
        "array element two"
    ]
})");
	// clang-format on
}
