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
  message(SEND_WARNING "On linux you should only try to build with Release or RelWithDebInfo")
  
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
  add_definitions(-DUNICODE -D_UNICODE -fp:except -Zm250)
  
  # program database flag for debug
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI -Gm /W3")
  
  # A policy is needed for launchers to work correctly
  
  # This allows the project target property be read the old way
  #cmake_policy(SET CMP0026 OLD)
  
  # This makes parameter evaluation expand things more aggressively(?)
  #cmake_policy(SET CMP0053 OLD)
  #cmake_policy(SET CMP0054 OLD)
  
else(WIN32)

  add_definitions(-fextended-identifiers)

  # Has to be on one line or else ';'s will be included
  # C++14
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wno-unused-function -Wno-unknown-pragmas -Wno-unused-variable -Wl,--no-undefined -Wl,--no-allow-shlib-undefined -Wno-pragma-once-outside-header")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")
  
  # We need X11 on linux for window class to work
  find_package(X11)
  find_package(Threads)

  if(NOT X11_FOUND)
    message(SEND_ERROR "Failed to find X11")
  endif()

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

