/**
 * Copyright 2026 Bruce Ide
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <gtest/gtest.h>
#include <fr/autocereal/JsonSchema.h>
#include <string>
#include <vector>

// Here's a simple namespaced class
namespace namespaced {
  struct NamespacedSchema {
    std::string foo;
    std::vector<long> bar;
  };
}

// Verify the JsonSchema object is collecting the correct data
TEST(SchemaTest, Basic) {

  struct BasicSchema {
    std::string foo;
    long bar;
    double baz;
    std::vector<std::string> strings;
  };

  fr::autocereal::JsonSchema <BasicSchema> schema;
  ASSERT_EQ(schema.elements.at("foo"), "string");
  ASSERT_EQ(schema.elements.at("bar"), "integer");
  ASSERT_EQ(schema.elements.at("baz"), "number");
  ASSERT_EQ(schema.elements.at("strings"), "array");
  ASSERT_EQ(schema.items.at("strings"), "string");
  // Object namespace. In this case there isn't one.
  ASSERT_EQ(schema.ns, "");
  ASSERT_EQ(schema.id, "https://fr.autocereal.internal/BasicSchema");
  fr::autocereal::JsonSchema<namespaced::NamespacedSchema> nsSchema;

  ASSERT_EQ(nsSchema.ns, "namespaced");
  ASSERT_EQ(nsSchema.id, "https://fr.autocereal.internal/namespaced/NamespacedSchema");
  
}

// Make sure Namespace is detected and output correctly with to_string
TEST(SchemaTest, NamespacedToString) {

  fr::autocereal::JsonSchema<namespaced::NamespacedSchema> schema("Namespaced schema to_string test");
  std::string expectedString{
"{\n\"$id\": \"https://fr.autocereal.internal/namespaced/NamespacedSchema\",\n\
\"$schema\": \"https://json-schema.org/draft/2020-12/schema\",\n\
\"description\": \"Namespaced schema to_string test\",\n\
\"type\": \"object\",\n\
\"properties\": {\n\
\"bar\": { \"type\": \"array\",\n\
 \"items\": { \"type\": \"integer\" }},\n\
\"foo\": { \"type\": \"string\"}\n\
}\n\
}\n"};
  
  ASSERT_EQ(schema.to_string(), expectedString);
  std::cout << schema.to_string();
  
}

// Some structures for an inheritance test

struct ParentSchema {
  long id;
  std::string foo;
};

struct ChildSchema : public ParentSchema {
  std::string bar;
  double baz;
  float bait;
  long quux;
  int florble;
  std::vector<bool> worble;
};

// This should pick up id and foo from the parent
TEST(SchemaTest, InheritanceToString) {

  fr::autocereal::JsonSchema<ChildSchema> schema{};

  // Note that only the child members would show up
  // in schema.elements -- properties recurses up the
  // inhertiance chain to get the correct strings for
  // the parent class properties, but the parent class
  // elements are stored in the JsonSchema for the
  // parent, not here.
  //
  // Also note that schema may not be in the same order
  // defined in C++ on account of me using an unordered
  // map here. This could be switched to a vector of pairs
  // if that's a problem.
  
  std::string expectedString{
"{\n\
\"$id\": \"https://fr.autocereal.internal/ChildSchema\",\n\
\"$schema\": \"https://json-schema.org/draft/2020-12/schema\",\n\
\"type\": \"object\",\n\
\"properties\": {\n\
\"foo\": { \"type\": \"string\"},\n\
\"id\": { \"type\": \"integer\"},\n\
\"worble\": { \"type\": \"array\",\n\
 \"items\": { \"type\": \"boolean\" }},\n\
\"florble\": { \"type\": \"integer\"},\n\
\"quux\": { \"type\": \"integer\"},\n\
\"bait\": { \"type\": \"number\"},\n\
\"baz\": { \"type\": \"number\"},\n\
\"bar\": { \"type\": \"string\"}\n\
}\n\
}\n"};

  ASSERT_EQ(schema.to_string(), expectedString);

  std::cout << schema.to_string();
  
}

TEST(SchemaTest, PrimitiveType) {

  // That's all well and good but what if we

  fr::autocereal::JsonSchema<int> schema("An int");
  std::cout << schema.to_string();
  
}
