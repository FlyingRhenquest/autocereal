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
#include <fr/autocereal/ClassSingleton.h>
#include <type_traits>

/**
 * Verifies we can get a member name by index
 */

TEST(ClassSingletonTests, MemberAt) {
  class Pleh {
    int foo;
    int bar;
  public:
    const std::string baz;
    int quux;
  };

  auto const &plehInstance = fr::autocereal::ClassSingleton<Pleh>::instance();
  std::string name(plehInstance.className());
  ASSERT_EQ(plehInstance.memberCount(), 4);
  ASSERT_EQ(name, "Pleh");
  // plehInstance returns string views for indexed queries, so we need to make strings
  // out of them for assert
  std::string fooMember(plehInstance.memberNameByIndex<0>());
  std::string barMember(plehInstance.memberNameByIndex<1>());
  std::string bazMember(plehInstance.memberNameByIndex<2>());
  std::string quuxMember(plehInstance.memberNameByIndex<3>());
  ASSERT_EQ(fooMember, "foo");
  ASSERT_EQ(barMember, "bar");
  ASSERT_EQ(bazMember, "baz");
  ASSERT_EQ(quuxMember, "quux");
  // Or look in the memberNames vector
  ASSERT_EQ(plehInstance.memberNames[0], "foo");
  ASSERT_EQ(plehInstance.memberNames[1], "bar");
  ASSERT_EQ(plehInstance.memberNames[2], "baz");
  ASSERT_EQ(plehInstance.memberNames[3], "quux");
}

TEST(ClassSingletonTests, Inheritance) {

  struct Parent {
    int foo;
    int bar;
  };

  struct Child : public Parent {
    int baz;
  };

  auto const &childInstance = fr::autocereal::ClassSingleton<Child>::instance();
  auto const &parentInstance = fr::autocereal::ClassSingleton<Child>::parent_instance<0>();

  ASSERT_EQ(childInstance.memberCount(), 1);
  ASSERT_EQ(parentInstance.memberCount(), 2);
  std::string parentName(parentInstance.className());
  std::string childName(childInstance.className());
  ASSERT_EQ(parentName, "Parent");
  ASSERT_EQ(childName, "Child");
  if constexpr(!std::is_same_v<Parent, fr::autocereal::ClassSingleton<Child>::ParentType<0>::ReflectionType>) {
    FAIL() << "Singleton ParentType reflection type is not the same as parent.";
  }
  if constexpr(!std::is_same_v<Parent, fr::autocereal::ClassSingleton<Child>::ParentReflectionType<0>>) {
    FAIL() << "Singleton ParentReflectionType is not the same as parent";
  }
}

TEST(ClassSingletonTests, Instance) {

  // This would be tricker for classes with inheritance, since you'd need to
  // traverse the inheritance tree to get all the associated data for all the
  // parent classes. memberData will only give you data defined directly in
  // the class you're reflecting on.
  
  class TestClass {
  public:
    std::string someData;
    
    TestClass(const std::string& data) : someData(data) {}
  };

  TestClass instance("Some data");
  auto const &singleton = fr::autocereal::ClassSingleton<TestClass>::instance();

  ASSERT_EQ(singleton.memberData<0>(instance), "Some data");
  singleton.setMemberData<0>(instance, "Some more data");
  ASSERT_EQ(instance.someData, "Some more data");
  
}
