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
 *
 * Testing core functionality to make autocereal work
 */

#include <gtest/gtest.h>
#include <fr/autocereal/autocereal.h>

// Structure to test with. We could define this inside each
// individual test, but I'm fine with having it here.

struct Pleh {
  int foo;
  std::string bar;
};

struct ChildPleh : public Pleh {
  bool child;
};

TEST(AcCoreTests, ListSize) {
  auto& classInstance = fr::autocereal::ClassSingleton<Pleh>::instance();
  ASSERT_EQ(classInstance.memberCount(), 2);
}

TEST(AcCoreTests, NumberParents) {
  auto& classInstance = fr::autocereal::ClassSingleton<Pleh>::instance();
  auto& childClassInstance = fr::autocereal::ClassSingleton<ChildPleh>::instance();
  ASSERT_EQ(classInstance.baseCount(), 0);
  ASSERT_EQ(childClassInstance.baseCount(), 1);
}

// We need an array of strings with the nonstatic names of the members
// of the class

TEST(AcCoreTests, StringArray) {
  auto& classInstance = fr::autocereal::ClassSingleton<Pleh>::instance();
  
  ASSERT_EQ(classInstance.memberNames.size(), 2);
  ASSERT_EQ(classInstance.memberNames[0], "foo");
  ASSERT_EQ(classInstance.memberNames[1], "bar");
  // Congratulations. We've sucessfully hoisted constexpr data into
  // into run-time!
}

// We need to be able to retrieve a member reference by index
// and use it at runtime

TEST(AcCoreTests, MemberRef) {
  Pleh pleh{1, "PLEH!"};
  constexpr auto ref_info = fr::autocereal::member_info<Pleh, 1>();
  auto& ref = fr::autocereal::member_ref<Pleh, ref_info>(pleh);
  const auto &constRef = fr::autocereal::member_ref_const<Pleh, ref_info>(pleh);
  
  ASSERT_EQ(ref, "PLEH!");
  ASSERT_EQ(constRef, "PLEH!");
  // Now we have (basically) everything we need to implement autocereal.
}

