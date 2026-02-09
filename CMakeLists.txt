cmake_minimum_required(VERSION 3.16)
project(WebGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- 默认构建类型（单配置生成器时生效，比如 Linux Make/Ninja）----
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# ---- LogManager ----
set(LOGMANAGER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../LogManager")
if(NOT EXISTS "${LOGMANAGER_SOURCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "LogManager project not found at ${LOGMANAGER_SOURCE_DIR}")
endif()
add_subdirectory("${LOGMANAGER_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/LogManager")

# ---- Sources ----
file(GLOB_RECURSE GAMECLIENT_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER GAMECLIENT_SOURCES EXCLUDE REGEX "src/network/ConnectProc\\.cpp$")
list(APPEND GAMECLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_executable(WebGame ${GAMECLIENT_SOURCES})
target_link_libraries(WebGame PRIVATE LogM)

set(PROJECT_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

file(GLOB_RECURSE PROJECT_HEADER_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.hpp"
)

foreach(header ${PROJECT_HEADER_FILES})
  get_filename_component(header_dir "${header}" DIRECTORY)
  list(APPEND PROJECT_INCLUDE_DIRS "${header_dir}")
endforeach()
list(REMOVE_DUPLICATES PROJECT_INCLUDE_DIRS)

target_include_directories(WebGame PRIVATE ${PROJECT_INCLUDE_DIRS})

file(GLOB_RECURSE ALL_HEADERS 
  LIST_DIRECTORIES false
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"    # src/下所有.h（含子文件夹）
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h"    # lib/下所有.h（含子文件夹）
)

target_sources(WebGame PRIVATE ${ALL_HEADERS})

# ---- 平台链接 ----
if(WIN32)
  target_link_libraries(WebGame PRIVATE ws2_32)
elseif(UNIX)
  find_package(Threads REQUIRED)
  set(CURSES_NEED_WIDE TRUE)
  find_package(Curses REQUIRED)
  find_library(NCURSESW_LIBRARY NAMES ncursesw)
  if(NCURSESW_LIBRARY)
    set(CURSES_LIBRARIES ${NCURSESW_LIBRARY})
  endif()
  target_link_libraries(WebGame PRIVATE Threads::Threads ${CURSES_LIBRARIES})
  target_include_directories(WebGame PRIVATE ${CURSES_INCLUDE_DIR})
endif()

# ---- Debug / RelWithDebInfo 生成符号，方便 gdb/core ----
if(NOT MSVC)
  target_compile_options(WebGame PRIVATE
    $<$<CONFIG:Debug>:-O0 -g3 -fno-omit-frame-pointer>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g2 -fno-omit-frame-pointer>
  )
endif()