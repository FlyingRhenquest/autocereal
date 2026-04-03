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

#include <meta>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace fr::autocereal {

  /**
   * This singleton provides several useful operations on class Metadata
   * from the C++26 reflection standard. it's a good place to centralize
   * operations to easily separate compile time from runtime code. This
   * is a good fit for singleton as there will only ever be one class
   * definition per-class and it can be used to avoid repeating metadata
   * queries in multiple places in your code. The information contained
   * in this singleton will be collected at Compile Time but will also
   * be usable at run time.
   *
   */
  
  template <typename Class>
  class ClassSingleton {
    // Reflection context to use use with other reflection queries. Unchecked
    // means private and public members of the class will be reflected.
    // Members inherited from parents will not be reflected -- you'd have
    // to iterate through the base classes to retrieve those.
    constexpr static auto _ctx = std::meta::access_context::unchecked();
    static constexpr size_t _baseCount = std::meta::bases_of(^^Class, _ctx).size();
    static constexpr size_t _memberCount = std::meta::is_class_type(^^Class) ? std::meta::nonstatic_data_members_of(^^Class, _ctx).size() : 0;

  public:
    // You can access parent types via this template
    template <size_t index>
    using ParentReflectionType = [:std::meta::type_of(std::meta::bases_of(^^Class, _ctx)[index]):];

    // You can access ClassSingletons of parent types via this template
    template <size_t index>
    using ParentType = ClassSingleton<ParentReflectionType<index>>;
    
    using ReflectionType = Class;

    std::vector<std::string> memberNames;

    /**
     * Returns the singleton instance of this class
     */
    
    static const ClassSingleton& instance() {
      static ClassSingleton<Class> instance;
      return instance;
    }
    
    /**
     * Get parent ClassSingletons by index
     */

    template <size_t index>
    static constexpr auto parent_instance() {
      // Make sure you're specifying an index that actually exists
      static_assert(index < _baseCount);
      return ClassSingleton<ParentReflectionType<index>>::instance();
    }

    /**
     * Get parent info -- This will probably usually be a namespace
     * but if you have a class defined inside another class or
     * something like that, the info of the containing class should
     * be returned.
     */

    constexpr auto parent() const {
      return parent_of(^^Class);
    }
    
    /**
     * Return number of parents Class has
     */
    
    constexpr size_t baseCount() const {
      return _baseCount;
    }

    /**
     * Return number of members in Class
     */
    
    constexpr size_t memberCount() const {
      return _memberCount;
    }

    /**
     * Returns a member name by index.
     */

    template <size_t index>
    constexpr std::string_view memberNameByIndex() const {
      static_assert(index < _memberCount);
      return std::meta::identifier_of(std::meta::nonstatic_data_members_of(^^Class, _ctx)[index]);
    }

    static constexpr std::string_view className() {
      return std::meta::has_identifier(^^Class) ? std::meta::identifier_of(^^Class) : std::meta::display_string_of(^^Class);
    }

    /**
     * Returns member data by index. Takes a reference to the class
     * being reflected and an index as a template parameter and
     * returns the data held by that member for that object.
     */

    template <size_t index>
    constexpr auto memberData(const Class& object) const {
      constexpr auto field = std::meta::nonstatic_data_members_of(^^Class, _ctx)[index];
      return object.[:field:];
    }

    template <size_t index>
    constexpr void setMemberData(Class& object, const auto& data) const {
      constexpr auto field = std::meta::nonstatic_data_members_of(^^Class, _ctx)[index];
      object.[:field:] = data;
    }

  private:
    ClassSingleton() {
      // Populate memberNames vector with strings (Yay, template for works now!)
      template for (constexpr size_t i : std::views::indices(_memberCount)) {
        memberNames.push_back(std::string(memberNameByIndex<i>()));
      }
    }
  };
  
}
