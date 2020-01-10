# CMake file for generating all project files
# Newer version required for VS_DEBUGGER_WORKING_DIRECTORY
if(WIN32)
  cmake_minimum_required(VERSION 3.10)
else()
  cmake_minimum_required(VERSION 3.6)
endif()

project(Leviathan)

# version number
set(LEVIATHAN_VERSION_STABLE 0)
set(LEVIATHAN_VERSION_MAJOR 8)
set(LEVIATHAN_VERSION_MINOR 0)
set(LEVIATHAN_VERSION_PATCH 0)

set(LEVIATHAN_VERSION_STR ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}.${LEVIATHAN_VERSION_MINOR}.${LEVIATHAN_VERSION_PATCH})
set(LEVIATHAN_VERSION ${LEVIATHAN_VERSION_STABLE}.${LEVIATHAN_VERSION_MAJOR}${LEVIATHAN_VERSION_MINOR}${LEVIATHAN_VERSION_PATCH})

set(WORK_DIR "${PROJECT_BINARY_DIR}/bin")
file(TO_NATIVE_PATH ${PROJECT_BINARY_DIR} PROJECT_NATIVE_BINARY_DIR)

# Leviathan base folder which is required for many folders to be correctly found
set(LEVIATHAN_SRC "${CMAKE_CURRENT_LIST_DIR}")

# modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake" "${PROJECT_SOURCE_DIR}/LeviathanCMake")

# load cotire
include(cotire)

# Load utility files
include(LeviathanUtility)

# Just in case some unity builds are broken
#set_property(DIRECTORY PROPERTY COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY")

######### Options

option(CREATE_STATIC "Set to ON to create static libraries instead of shared" OFF)

option(CREATE_SHIPPING "SET THIS ON WHEN BUILDING END USER BINARIES. Will disable some expensive checks" OFF)

# Copies boost libraries to distributions
option(COPY_BOOST_TO_PACKAGE "For copying all boost libraries to run on system that don't have it" OFF)

# Whether to use Javascript or php search in doxygen
option(DOCUMENTATION_LOCAL "Set to OFF to create web documentation" ON)

option(BUILD_SAMPLES "Set to OFF to disable building demo programs" ON)
option(BUILD_TESTS "Set to OFF to disable building tests" ON)



# Option for only building the documentation (for lighter jenkins)
option(ONLY_DOCUMENTATION "If true only documentation can be built correctly" OFF)

option(MAKE_RELEASE "Enabled breakpad crash reporting and some game specific options" OFF)

option(LEVIATHAN_USE_ACTUAL_OBJECT_POOLS
  "Set to false to use new/delete instead of object pools. Provided for debugging memory issues with valgrind" ON)

# Library configuration
# This is how to slim down Leviathan if you only need some part
option(USE_ANGELSCRIPT "Enable features needing AngelScript" ON)
# option(USE_BOOST "Enable features needing Boost" ON) this is mandatory
option(USE_OGRE "Enable features needing Ogre" ON)
option(USE_BULLET "Enable features needing Bullet" ON)
option(USE_CEF "Enable features needing CEF" ON)
option(USE_GUI "Enable Leviathan GUI" ON)
option(USE_SFML "Enable features needing SFML" ON)
option(USE_SDL2 "Enable features needing SDL2" ON)

option(USE_LEAP "Set to on when using Leap motion, will auto disable if files are not copied" OFF)

# Builds Leviathan as an utility library
option(LEVIATHAN_AS_LIBRARY "Builds standalone compatible Leviathan parts only" OFF)

# Build configuration
# TODO: this still needs a lot of work, LEVIATHAN_USING_DEPENDENCIES is basically now used
# to switch between Leviathan library and full engine

if(ONLY_DOCUMENTATION OR LEVIATHAN_AS_LIBRARY)

  set(MAKE_RELEASE OFF)
  set(LEVIATHAN_USING_ANGELSCRIPT OFF)
  # set(LEVIATHAN_USING_BOOST OFF)
  set(LEVIATHAN_USING_OGRE OFF)
  set(LEVIATHAN_USING_BULLET OFF)
  set(LEVIATHAN_USING_CEF OFF)
  set(LEVIATHAN_USING_GUI OFF)
  set(LEVIATHAN_USING_SFML OFF)
  set(LEVIATHAN_USING_SDL2 OFF)
  set(LEVIATHAN_USING_LEAP OFF)

  set(BUILD_SAMPLES OFF)
    
else()
  set(CMAKE_INSTALL_PREFIX "./Install")

  set(LEVIATHAN_USING_ANGELSCRIPT ${USE_ANGELSCRIPT})
  # set(LEVIATHAN_USING_BOOST ${USE_BOOST})
  set(LEVIATHAN_USING_OGRE ${USE_OGRE})
  set(LEVIATHAN_USING_BULLET ${USE_BULLET})
  set(LEVIATHAN_USING_CEF ${USE_CEF})
  set(LEVIATHAN_USING_GUI ${USE_GUI})
  set(LEVIATHAN_USING_SFML ${USE_SFML})
  set(LEVIATHAN_USING_SDL2 ${USE_SDL2})
  set(LEVIATHAN_USING_LEAP ${USE_LEAP})
endif()

# Configuration checks
if(LEVIATHAN_USING_OGRE AND NOT LEVIATHAN_USING_SDL2)
  message(SEND_ERROR "When using Ogre SDL2 must also be used")
endif()

# Check that leap motion can be enabled
if(LEVIATHAN_USING_LEAP)
  if(EXISTS "${LEVIATHAN_SRC}/Leap/include/Leap.h")

    message(STATUS "Leap motion includes found")
    
  else()

    message(WARNING
      "Leap motion files aren't installed in leviathan/Leap, Leap motion support is disabled")
    
    set(LEVIATHAN_USING_LEAP OFF)
    
  endif()
endif()


include(LeviathanCompileOptions)
include(LeviathanSetRPath)

if(NOT ONLY_DOCUMENTATION)
  # Find required libraries
  include(LeviathanFindLibraries)
endif()

# Load platform definitions for configuring Include.h
include(DiligentPlatform)

# Configure engine include file
configure_file("${LEVIATHAN_SRC}/Engine/Include.h.in" "${PROJECT_BINARY_DIR}/Include.h")
# Make a readonly copy of it
file(COPY "${PROJECT_BINARY_DIR}/Include.h" DESTINATION "${LEVIATHAN_SRC}/Engine"
  NO_SOURCE_PERMISSIONS
  FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)




# configure the doxygen file
find_package(Doxygen)
if(DOXYGEN_FOUND)

  if(DOCUMENTATION_LOCAL)
    set(DOXYGEN_SERVER_SEARCH "FALSE")
  else()
    set(DOXYGEN_SERVER_SEARCH "TRUE")
  endif()

  if(LEVIATHAN_AS_LIBRARY)
    set(LEVIATHAN_DOC_COMMAND_SUFFIX "-leviathan")
  else()
    set(LEVIATHAN_DOC_COMMAND_SUFFIX "")
  endif()
  
  configure_file("${PROJECT_SOURCE_DIR}/LeviathanDoxy.in"
    "${PROJECT_BINARY_DIR}/LeviathanDoxy" @ONLY)
  add_custom_target(doc${LEVIATHAN_DOC_COMMAND_SUFFIX}
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/LeviathanDoxy
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

else()
  if(ONLY_DOCUMENTATION)
    message(SEND_ERROR "Documentation only build is missing doxygen")
  endif()
endif()

if(NOT ONLY_DOCUMENTATION)

  if(LEVIATHAN_USING_DEPENDENCIES)
    include(LeviathanDefaultFileCopy)
  endif()

  # Controls how LeviathanCoreProject.cmake works
  set(CREATE_CONSOLE_APP OFF)

  # First project must be the file generator
  add_subdirectory(FileGenerator)
  
  # engine project 
  add_subdirectory(Engine)
  
  # test project
  if(BUILD_TESTS)
    add_subdirectory(LeviathanTest)

    # Set the tests as the default startup project
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY
      VS_STARTUP_PROJECT LeviathanTest)
  endif()
  
  if(LEVIATHAN_USING_DEPENDENCIES)

    # Standalone editor
    add_subdirectory(Editor)
  endif()

  if(BUILD_SAMPLES)
    # example pong master server
    add_subdirectory(PongMasterServer)
    
    # example pong server
    add_subdirectory(PongServer)

    # pong example game
    add_subdirectory(Pong)

    # demo examples
    add_subdirectory(Demos)
  endif()  
endif()
