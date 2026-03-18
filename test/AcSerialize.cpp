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
#include <cereal/access.hpp>
#include <fr/autocereal/autocereal.h>
#include <sstream>

struct SerialPleh {
  int foo;
  std::string bar;
};

// Or is this Unicode, I don't know...
TEST(AcSerializeTest, Utf8) {
  struct EmojiStruct {
    std::string 👍;
  };

  EmojiStruct emoji;
  emoji.👍 = "Sure thing buddy";
  EmojiStruct copy;
  std::stringstream stream;
  {
    cereal::JSONOutputArchive archive(stream);
    archive(emoji);
  }

  {
    cereal::JSONInputArchive archive(stream);
    archive(copy);
  }
  ASSERT_EQ(emoji.👍, copy.👍);
  // 😎👍💩
}

TEST(AcSerializeTest, SerializeDeserialize) {

  SerialPleh pleh{1, "PLEH!"};
  SerialPleh copy;

  std::stringstream stream;
  // You want to do cereal operations in their own
  // scope so they clean up after themselves.
  {
    cereal::JSONOutputArchive archive(stream);
    archive(pleh);
  }

  {
    cereal::JSONInputArchive archive(stream);
    archive(copy);
  }

  ASSERT_EQ(pleh.foo, copy.foo);
  ASSERT_EQ(pleh.bar, copy.bar);

  // Mic Drop!
  
}

TEST(AcSerializeTest, AcSerializePrivates) {

  class PrivateParts {
    int _parts;
  public:
    std::string publicThing;
    
    PrivateParts() : _parts(0) {}

    int getParts() const {
      return _parts;
    }

    void setParts(int parts) {
      _parts = parts;
    }
  };

  const auto &instance = fr::autocereal::ClassSingleton<PrivateParts>::instance();
  ASSERT_EQ(instance.memberCount(), 2);
  PrivateParts parts;
  PrivateParts copy;
  parts.setParts(42);
  parts.publicThing = "Public";
  std::stringstream stream;

  {
    cereal::JSONOutputArchive archive(stream);
    archive(parts);
  }

  {
    cereal::JSONInputArchive archive(stream);
    archive(copy);
  }

  ASSERT_EQ(parts.getParts(), copy.getParts());
  ASSERT_EQ(parts.publicThing, copy.publicThing);
  
}

TEST(ACSerializeTest, Inheritance) {

  struct Parent {
    int foo;

    Parent() : foo(0) {}
    virtual ~Parent() {}

  };

  struct Child : public Parent {
    int bar;

    Child() : Parent(), bar(0) {}
    virtual ~Child() {};
  };

  Child child;
  Child copy;
  child.foo = 22;
  child.bar = 42;

  std::stringstream stream;
  {
    cereal::JSONOutputArchive archive(stream);
    archive(child);
  }

  {
    cereal::JSONInputArchive archive(stream);
    archive(copy);
  }

  ASSERT_EQ(child.bar, copy.bar);
  ASSERT_EQ(child.foo, copy.foo);
  
}

TEST(ACSerializeTest, SharedPtr) {
  auto pleh = std::make_shared<SerialPleh>(42, "- Pleh -");
  auto copy = std::make_shared<SerialPleh>();

  std::stringstream stream;
  {
    cereal::JSONOutputArchive archive(stream);
    archive(pleh);
  }

  {
    cereal::JSONInputArchive archive(stream);
    archive(copy);
  }

  ASSERT_EQ(pleh->foo, copy->foo);
  ASSERT_EQ(pleh->bar, copy->bar);
}

TEST(ACSerializeTest, SharedPtrInClass) {

  struct Wibble {
    std::shared_ptr<std::string> wobble;
  };
  std::string womble("womble");
  
  Wibble wibble;
  wibble.wobble = std::make_shared<std::string>(womble);

  std::stringstream stream;
  {
    cereal::JSONOutputArchive archive(stream);
    archive(wibble);
  }

  Wibble copy;
  {
    cereal::JSONInputArchive archive(stream);
    archive(copy);
  }

  ASSERT_EQ(*wibble.wobble, *copy.wobble);
  
}
