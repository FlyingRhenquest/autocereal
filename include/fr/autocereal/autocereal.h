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

// TODO: We probably don't need all of these, I should look into
// paring the cereal list down a bit.

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

#include <fr/autocereal/ClassSingleton.h>

#include <algorithm>
#include <array>
#include <concepts>
#include <meta>
#include <string.h>
#include <iostream>
#include <vector>

namespace fr::autocereal {

  /**
   * Retrieve a memberinfo for a member, by index
   *
   * Since this is consteval, index must be a template parameter
   */
  
  template <typename Class, size_t idx>
  consteval auto member_info() {
    constexpr auto ctx = std::meta::access_context::unchecked();    
    constexpr auto member = std::meta::nonstatic_data_members_of(^^Class, ctx)[idx];
    
    return member;
  }

  /**
   * Get a ref using the return value of member_info. Somewhat surprisingly,
   * this didn't put up a fight.
   */
  
  template <typename Class, auto info>
  constexpr auto& member_ref(Class &instance) {
    return instance.[:info:];
  }

  /**
   * We also need a const version of member_ref
   */

  template <typename Class, auto info>
  constexpr const auto member_ref_const(const Class &instance) {
    return instance.[:info:];
  }

  /**
   * This will save any parents that need to be saved.
   * This should only be called if there ARE any parents.
   * (And not by you! You don't need to worry about this!)
   */

  template <typename Archive, typename Class, size_t memberCount, size_t index = 0>
  void saveHelper(Archive &ar, const Class &instance);
  
  template <typename Archive, typename Class, size_t index = 0>
  void saveParents(Archive &ar, const Class& instance) {
    const auto& classInstance = fr::autocereal::ClassSingleton<Class>::instance();
    const auto& parentInstance = fr::autocereal::ClassSingleton<Class>::template parent_instance<index>();
    using Parent = typename fr::autocereal::ClassSingleton<Class>::ParentReflectionType<index>;
    const auto& baseInstance = static_cast<const Parent&>(instance);
    saveHelper<Archive, Parent, parentInstance.memberCount()>(ar, baseInstance);
    if constexpr ((index + 1)  < classInstance.baseCount()) {
      saveParents<Archive, Class, index + 1>(ar, instance);
    }
  }

  /**
   * This will load any parents that need to be loaded.
   * This should only be called if there ARE any parents.
   * (And not by you! You don't need to worry about this!)
   */

  template <typename Archive, typename Class, size_t memberCount, size_t index = 0>
  void loadHelper(Archive &ar, Class& instance);

  template <typename Archive, typename Class, size_t index = 0>
  void loadParents(Archive &ar, Class& instance) {
    const auto& classInstance = fr::autocereal::ClassSingleton<Class>::instance();
    const auto& parentInstance = fr::autocereal::ClassSingleton<Class>::template parent_instance<index>();
    using Parent = typename fr::autocereal::ClassSingleton<Class>::ParentReflectionType<index>;
    auto& baseInstance = static_cast<Parent&>(instance);
    loadHelper<Archive, Parent, parentInstance.memberCount()>(ar, baseInstance);
    if constexpr ((index + 1)  < classInstance.baseCount()) {
      loadParents<Archive, Class, index + 1>(ar, instance);
    }      
  }
  
  /**
   * We need to use save_helper to recursively unwind
   * the calls to archive, since we need to send the
   * current index as a template parameter and we
   * can't just use a for loop index to do that.
   *
   * I could probably do this with "template for"
   * now, but this is working fine.
   */

  template <typename Archive, typename Class, size_t memberCount, size_t index>
  void saveHelper(Archive &ar, const Class& instance) {
    const auto& classInstance = ClassSingleton<Class>::instance();
    static_assert(classInstance.memberCount() > 0);
    static_assert(index < classInstance.memberCount());

    if constexpr(index == 0) {
      if constexpr (classInstance.baseCount() > 0) {
        fr::autocereal::saveParents<Archive, Class>(ar, instance);
      }
    }
    
    // Get instance ref for index
    constexpr auto ref_info = fr::autocereal::member_info<Class, index>();
    // Extract the data from the cass
    const auto& constRef = fr::autocereal::member_ref_const<Class, ref_info>(instance);
    ar(cereal::make_nvp(classInstance.memberNames[index], constRef));

    // Recursively unwind until all members are archived
    if constexpr ((index + 1)  < classInstance.memberCount()) {
      saveHelper<Archive, Class, classInstance.memberCount(), index + 1>(ar, instance);
    }    
  }

  /**
   * And a load version of that
   */

  template <typename Archive, typename Class, size_t memberCount, size_t index>
  void loadHelper(Archive &ar, Class& instance) {
    const auto& classInstance = ClassSingleton<Class>::instance();
    static_assert(classInstance.memberCount() > 0);
    static_assert(index < classInstance.memberCount());

    if constexpr(index == 0) {
      if constexpr (classInstance.baseCount() > 0) {
        fr::autocereal::loadParents<Archive, Class>(ar, instance);
      }
    }

    constexpr auto ref_info = fr::autocereal::member_info<Class, index>();
    auto& ref = fr::autocereal::member_ref<Class, ref_info>(instance);

    // We don't use cereal::make_nvp here because it can lead to weirdness
    // As long as the order is the same as the save function, this is fine.
    
    ar(ref);

    if constexpr ((index + 1) < classInstance.memberCount()) {
      loadHelper<Archive, Class, classInstance.memberCount(), index + 1>(ar, instance);
    }
  }

  /**
   * A couple of stream concepts so we can implement to_json and to_xml.
   * You could do YAML too with one of the cereal yaml projects
   */

  template <typename T>
  concept IsInputStream = std::derived_from<T, std::istream>;

  template <typename T>
  concept IsOutputStream = std::derived_from<T, std::ostream>;

  /**
   * And cereal input/output archive concepts too
   */

  template <typename T>
  concept IsInputArchive = std::derived_from<T, cereal::detail::InputArchiveBase>;

  template <typename T>
  concept IsOutputArchive = std::derived_from<T, cereal::detail::OutputArchiveBase>;

  /**
   * Generic to-output-archive. This just writes the archive to the
   * stream.
   */

  template <typename T, typename ArchiveType>
  requires IsOutputArchive<ArchiveType>
  void to_output_archive(const T& obj, ArchiveType &ar) {
    ar(obj);
  }

  /**
   * Generic from-input-archive reads an object from an
   * archive.
   */

  template <typename T, typename ArchiveType>
  requires IsInputArchive<ArchiveType>
  void from_input_archive(T& obj, ArchiveType &ar) {
    ar(obj);
  }

  /**
   * to_json writes json data into an output stream
   */

  template <typename T, typename Stream>
  requires IsOutputStream<Stream>
  void to_json(const T& obj, Stream& stream) {
    cereal::JSONOutputArchive ar(stream);
    to_output_archive(obj, ar);
  }

  /**
   * from_json reads json data into an object
   */

  template <typename T, typename Stream>
  requires IsInputStream<Stream>
  void from_json(T& obj, Stream& stream) {
    cereal::JSONInputArchive ar(stream);
    from_input_archive(obj, ar);
  }

  /**
   * to_xml writes XML data to a stream
   */
  
  template <typename T, typename Stream>
  requires IsOutputStream<Stream>
  void to_xml(const T& obj, Stream& stream) {
    cereal::XMLOutputArchive ar(stream);
    ar(obj);
  }

  /**
   * from_xml reads XML data from a stream
   */
  template <typename T, typename Stream>
  requires IsInputStream<Stream>
  void from_xml(T& obj, Stream& stream) {
    cereal::XMLInputArchive ar(stream);
    ar(obj);
  }

  /**
   * String version of to_json
   */
  template <typename T>
  std::string to_json(const T& obj) {
    std::stringstream stream;
    to_json(obj, stream);
    return stream.str();
  }

  /**
   * String version of from_json
   */
  template <typename T>
  void from_json(T& obj, std::string json) {
    std::stringstream stream(json);
    from_json(obj, stream);
  }

  /**
   * String version of to_xml
   */

  template <typename T>
  std::string to_xml(const T& obj) {
    std::stringstream stream;
    to_xml(obj, stream);
    return stream.str();
  }

  /**
   * String version of from_xml
   */

  template <typename T>
  void from_xml(T& obj, std::string xml) {
    std::stringstream stream(xml);
    from_xml(obj, stream);
  }
  
}

namespace cereal {

  /**
   * Implements a save function that can be used for any class
   * Note that if you have private members you want to load or save
   * you need to follow the "Non-public Serialization" instructions
   * at https://uscilab.github.io/cereal/serialization_functions.html
   * to to enable cereal to access those members.
   */

  template <typename Archive, typename Class>
  void save(Archive &ar, const Class& instance) {
    const auto& classInstance = fr::autocereal::ClassSingleton<Class>::instance();
    fr::autocereal::saveHelper<Archive, Class, classInstance.memberCount()>(ar, instance);
  }

  /**
   * and the load version of that
   */

  template <typename Archive, typename Class>
  void load(Archive &ar, Class &instance) {
    const auto& classInstance = fr::autocereal::ClassSingleton<Class>::instance();
    fr::autocereal::loadHelper<Archive, Class, classInstance.memberCount()>(ar, instance);
  }
  
}
