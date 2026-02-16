cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE WEBGAME_TEST_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

if(NOT WEBGAME_TEST_SOURCES)
  message(STATUS "WebGameTests: no sources found under ${CMAKE_CURRENT_SOURCE_DIR}")
  return()
endif()

add_executable(WebGameTests ${WEBGAME_TEST_SOURCES})
target_link_libraries(WebGameTests PRIVATE WebGameCore gtest_main)
add_test(NAME WebGameTests COMMAND WebGameTests)
