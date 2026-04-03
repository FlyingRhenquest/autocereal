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

#pragma once
#include <fr/autocereal/ClassSingleton.h>
#include <format>
#include <unordered_map>
#include <string>

namespace fr::autocereal {

  namespace SchemaTypes {

    /**
     * Type conversions from C++ types to JSON Schema types. If a type is
     * not explicitly listed here, it will default to "object"
     */
    
    template <typename T>
    struct Type {
      static constexpr char const* name = std::define_static_string("object");
    };

    template <>
    struct Type<bool> {
      static constexpr char const* name = std::define_static_string("boolean");
    };

    template <>
    struct Type<int> {
      static constexpr char const* name = std::define_static_string("integer");
    };

    template <>
    struct Type<long> {
      static constexpr char const* name = std::define_static_string("integer");
    };

    template <>
    struct Type<float> {
      static constexpr char const* name = std::define_static_string("number");
    };

    template <>
    struct Type<double> {
      static constexpr char const* name = std::define_static_string("number");
    };
    
    template <>
    struct Type<std::string> {
      static constexpr char const* name = std::define_static_string("string");
    };

    // Handle Vectors
    template<typename T, typename Alloc>
    struct Type<std::vector<T, Alloc>> {
      static constexpr const char* name = std::define_static_string("array");
      static constexpr const char* items = Type<T>::name;
    };
    
  }
  
  /**
   * Create a JsonSchema with a template parameter being the object
   * you want to create the schema for. This generates one schema for
   * one object.  I may later provide an object capable of storing
   * multiple JsonSchema objects and generating references between the
   * ones it knows about.
   *
   * I'm not going to guarantee that this version of this will work
   * correctly for classes defined inside other classes.
   */
  
  template <typename T>
  class JsonSchema {
    constexpr static auto _ctx = std::meta::access_context::unchecked();

    void computeNamespace(const ClassSingleton<T>& singleton) {
      if constexpr(std::meta::has_parent(^^T)) {
        constexpr auto parent = singleton.parent();
        if constexpr (!std::meta::is_namespace(parent)
                      || parent == ^^::) {
          ns = "";
        } else {
          ns = std::meta::identifier_of(parent);
        }
      } else {
        ns = "";
      }
    }

    constexpr void populateClassName(const ClassSingleton<T>& singleton) {
      className = singleton.className();
    }
    
    void populateId(const ClassSingleton<T>& singleton) {
      if (ns.empty()) {
        id = std::format("https://fr.autocereal.internal/{}", className);
      } else {
        id = std::format("https://fr.autocereal.internal/{}/{}", ns, className);
      }
    }

    constexpr void populateMembers() {

      if constexpr(std::meta::is_class_type(^^T)) {
        template for(constexpr auto& member : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, _ctx))) {
          using ThisSchemaType = [: std::meta::type_of(member) :];
          std::string thisIdentifier{std::meta::identifier_of(member)};
          elements[thisIdentifier] = SchemaTypes::Type<ThisSchemaType>::name;
          if constexpr ( requires { SchemaTypes::Type<ThisSchemaType>::items; }) {
            items[thisIdentifier] = SchemaTypes::Type<ThisSchemaType>::items;
          }
        }
      }
    }

    // Returns the header info of the schema (Stuff other than all the types)
    
    std::string header() {
      std::string ret = std::format("\"$id\": \"{}\",\n", id);
      ret.append(std::format("\"$schema\": \"{}\",\n", schema));
      if (!description.empty()) {
        ret.append(std::format("\"description\": \"{}\",\n", description));
      }
      ret.append(std::format("\"type\": \"{}\"", SchemaTypes::Type<T>::name));
      if (elements.size() > 0) {
        ret.append(",");
      }
      ret.append("\n");
      return ret;
    }

    std::string propertiesHeader() {
      
      std::string ret;
      if (elements.size() > 0) {
        ret.append("\"properties\": {\n");
      }
      return ret;
    }

    std::string propertiesFooter() {
      
      std::string ret;
      if (elements.size() > 0) {
        ret.append("\n}\n");
      }
      return ret;
    }
        

    
  public:

    std::string schema;
    std::string description;
    std::string id;
    std::string ns;
    std::string className;
    std::unordered_map<std::string, std::string> elements;
    // map for container types.
    std::unordered_map<std::string, std::string> items;

    /**
     * If you add an optional description string, that will be added to the schema
     */
    
    JsonSchema(const std::string& desc = "") : description(desc) {
      schema = "https://json-schema.org/draft/2020-12/schema";
      const auto& singleton = ClassSingleton<T>::instance();
      computeNamespace(singleton);
      populateClassName(singleton);
      populateId(singleton);
      populateMembers();
    };

    // The guts of a properties block
    
    std::string properties(bool &comma) {
      std::string ret;
      
      // Get properties strings for parent classes if there are any

      if constexpr(std::meta::is_class_type(^^T)) {
        template for (constexpr auto& parentClass : std::define_static_array(std::meta::bases_of(^^T, _ctx))) {
          using ParentClass = [: std::meta::type_of(parentClass) :];
          JsonSchema<ParentClass>  parentSchema{};
          // Comma will be set correctly by the first properties that actually adds
          // some JSON to the return string
          ret.append(parentSchema.properties(comma));
        }
      }
      
      for (auto [name, classType] : elements) {
        if (comma) {
          ret.append(",\n");
        } else {
          comma = true;
        }
        ret.append(std::format("\"{}\": {{ \"type\": \"{}\"", name, classType));
        if (items.contains(name)) {
          ret.append(std::format(",\n \"items\": {{ \"type\": \"{}\" }}", items[name]));
        }
        ret.append("}");
      }
      return ret;
    }

    std::string to_string() {
      bool comma = false;
      std::string ret = "{\n";
      ret.append(header());
      ret.append(propertiesHeader());
      ret.append(properties(comma));
      ret.append(propertiesFooter());
      ret.append("}\n");
      return ret;
    }
    
  };
  
}
