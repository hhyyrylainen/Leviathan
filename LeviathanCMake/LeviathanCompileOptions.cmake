# For making libs with right names
set(CMAKE_DEBUG_POSTFIX "D")

# When no configuration is set, enable RelWithDebInfo
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Set the build type, usually Release or RelWithDebInfo" FORCE)
endif(CMAKE_BUILD_TYPE STREQUAL "")

if(NOT WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Not recommended configuration
  message(STATUS "Configuration is set to: " ${CMAKE_BUILD_TYPE})
  message(WARNING "On linux the Debug configuration may not work!")
  
endif()

if(NOT WIN32)
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
endif()


# Linking and platform specific
if(WIN32)
  # we want to build with unicode setting, also add minimal rebuild,
  # and exceptions from floating point operations, 
  # higher memory for compiling precompiled headers
  # Currently it seems that Zm250 should be enough but it might not be
  # -Zm250 is probably no longer needed
  add_definitions(-DUNICODE -D_UNICODE)

  # According to https://developercommunity.visualstudio.com/content/problem/162020/c3199-including-in-155.html
  # this has already been fixed, but we need to wait for a new visual studio release
  # -fp:except causes issues with c++17 enabled
  # error C3199: invalid use of floating-point pragmas: exceptions are
  # not supported in non-precise mode
  
  # program database flag for debug
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI -Gm /W3")

  # Multi core compilation
  add_definitions("/MP")

  # Enable c++17
  # This is instead set per target by set_property(TARGET target PROPERTY CXX_STANDARD 17) and
  # set_property(TARGET target PROPERTY CXX_EXTENSIONS OFF)
  # add_definitions("/stdc:c++17")
  
  # A policy is needed for launchers to work correctly
  
  # This allows the project target property be read the old way
  #cmake_policy(SET CMP0026 OLD)
  
  # This makes parameter evaluation expand things more aggressively(?)
  #cmake_policy(SET CMP0053 OLD)
  #cmake_policy(SET CMP0054 OLD)
  
else(WIN32)

  # add_definitions(-fextended-identifiers)

  # Has to be on one line or else ';'s will be included
  # C++17
  # To work with ld.gold we must prevent RUNPATH (instead of RPATH) from being added.
  # Otherwise the dynamic library loading completely implodes. That could be worked around by
  # setting LD_LIBRARY_PATH before running (for example with a launch script) but that
  # would make debugging not work...
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wno-unused-function -Wno-unknown-pragmas -Wno-unused-variable -Wl,--no-undefined -Wl,--no-allow-shlib-undefined -Wl,--disable-new-dtags -Wno-pragma-once-outside-header")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")

  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  
  # We need X11 on linux for window class to work
  find_package(X11 REQUIRED)
  find_package(Threads REQUIRED)

  # We need XMU X11 extension for clipboard
  if(NOT X11_Xmu_FOUND)
    message(SEND_ERROR "Failed to find X11-Xmu")
  endif()
  
  include_directories(${X11_INCLUDE_DIR})
endif(WIN32)

# bin folder
# create the bin directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Tests can crash if this folder doesn't exist
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/Test")

# Leviathan include directories

# additional include directories
include_directories("${PROJECT_SOURCE_DIR}/Engine/")

