cmake_minimum_required(VERSION 4.2)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)
include(GoogleTest)

set(TEST_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/AcCore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/AcSerialize.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ClassSingleton.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ToFromFunctions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SchemaTest.cpp
)

add_executable(test
  ${TEST_SRC}
)

target_link_libraries(test PUBLIC
  GTest::GTest
  GTest::Main
  FR::autocereal
)
