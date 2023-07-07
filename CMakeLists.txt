set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 3.10)
project(Kraken)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
#set(CMAKE_BUILD_TYPE Debug) 

find_package(Boost COMPONENTS REQUIRED)
include_directories(include)
add_subdirectory(src)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG_BUILD")
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )

  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_subdirectory(test)

endif()