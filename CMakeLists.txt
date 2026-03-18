cmake_minimum_required(VERSION 4.2)

project(autocereal LANGUAGES CXX)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AUTOCRUD_BUILD_MODULES "Build autocereal as a C++ module" OFF)
OPTION(AUTOCEREAL_BUILD_TESTS "Build autocereal unit tests" ON)

find_package(cereal CONFIG REQUIRED)

if (AUTOCRUD_BUILD_MODULES)
  add_library(autocereal)
  add_library(FR::autocereal ALIAS autocereal)
  
  target_sources(autocereal
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fr/autocereal/fr.autocereal.cppm
  )
else()
  add_library(autocereal INTERFACE)
  add_library(FR::autocereal ALIAS autocereal)
endif()

target_include_directories(autocereal INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(autocereal INTERFACE cereal::cereal)

target_compile_features(autocereal INTERFACE cxx_std_26)

if (AUTOCEREAL_BUILD_TESTS)
  add_subdirectory(test)
endif()
