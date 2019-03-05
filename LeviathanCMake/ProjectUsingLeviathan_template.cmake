# A template for a project that uses leviathan


# CMake main file for Game_Name
cmake_minimum_required(VERSION 3.0)

project(ProjectName)

# Options
option(MAKE_RELEASE "Enabled breakpad crash reporting and some game specific options" OFF)
option(COPY_BOOST_TO_PACKAGE "If on copies all boost libraries to package" ON)


set(LEVIATHAN_SRC "${PROJECT_SOURCE_DIR}/../leviathan" CACHE FILEPATH
  "Path to leviathan source folder")

# Set up cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMake"
  "${LEVIATHAN_SRC}/CMake"
  "${LEVIATHAN_SRC}/LeviathanCMake")

include(LeviathanCompileOptions)
include(LeviathanSetRPath)

# Find required libraries
set(LEVIATHAN_USING_ANGELSCRIPT ON)
set(LEVIATHAN_USING_OGRE ON)
set(LEVIATHAN_USING_BULLET ON)
set(LEVIATHAN_USING_CEF ON)
set(LEVIATHAN_USING_GUI ON)
set(LEVIATHAN_USING_SFML ON)
set(LEVIATHAN_USING_SDL2 ON)
set(LEVIATHAN_USING_LEAP ON)

include(LeviathanFindLibraries)

include(cotire)
include(LeviathanUtility)

# Leviathan Required settings
include(LeviathanDefaultFileCopy)
include(LeviathanSetRPath)

# -Wl,-rpath-link is used to suppress warnings from games linking gainst libEngine.so
if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath-link,${CMAKE_BINARY_DIR}/bin/lib")
endif()

# Links to libs

# Copy engine lib
if(WIN32)
  file(COPY "${LEVIATHAN_SRC}/build/bin/Engine.dll" DESTINATION "${CMAKE_BINARY_DIR}/bin")
  file(COPY "${LEVIATHAN_SRC}/build/bin/Engine.pdb" DESTINATION "${CMAKE_BINARY_DIR}/bin")
else()
  file(COPY "${LEVIATHAN_SRC}/build/bin/libEngine.so" DESTINATION "${CMAKE_BINARY_DIR}/bin")
endif()

# Link own bin directory
link_directories("${CMAKE_BINARY_DIR}/bin")

# And engine
link_directories("${LEVIATHAN_SRC}/build/bin")

# Version
set(PROGRAM_VERSION_STABLE 0)
set(PROGRAM_VERSION_MAJOR 1)
set(PROGRAM_VERSION_MINOR 0)
set(PROGRAM_VERSION_PATCH 0)

set(PROGRAM_VERSION_STR ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}.${PROGRAM_VERSION_MINOR}.${PROGRAM_VERSION_PATCH})
set(PROGRAM_VERSION ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}${PROGRAM_VERSION_MINOR}${PROGRAM_VERSION_PATCH})

set(PROGRAM_VERSIONS "\"${PROGRAM_VERSION_STR}\"")
set(PROGRAM_VERSIONS_ANSI "\"${PROGRAM_VERSION_STR}\"")

set(ENGINECONFIGURATION				"./EngineConf.conf")
set(GAMENAMEIDENTIFICATION			Game_Name)
set(GAMEVERSIONIDENTIFICATION		GAME_VERSIONS)
set(ProgramNamespace Game_Name)
set(PROGRAMMASTERSERVERINFO			"MasterServerInformation(\"Game_NameMasterServers.txt\", \"Game_Name_\" GAME_VERSIONS, \"http://boostslair.com/\", \"/Game_Name/MastersList.txt\", \"Game_NameCrecentials.txt\", false)")

# Configure files

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)

  configure_file("${PROJECT_SOURCE_DIR}/Game_NameDoxy.in"
    "${PROJECT_BINARY_DIR}/Game_NameDoxy" @ONLY)
    add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Game_NameDoxy
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()


# Include dirs
include_directories("${LEVIATHAN_SRC}/Engine")
include_directories("${CMAKE_SOURCE_DIR}/Common")

set(DEPENDENT_LIBS ${LEVIATHAN_APPLICATION_LIBRARIES})

add_subdirectory(Common)

add_subdirectory(Client)

add_subdirectory(Server)

add_subdirectory(Test)


#
# Split new files from this following template to the Client, Server etc. folders
#
# Test application for Game_Name CMake

set(BaseProgramName "Client")
set(BaseIncludeFileName "CommonVersionInclude.h")
set(BaseSubFolder "Client")

# Set all the settings
set(ProgramIncludesHeader "${BaseIncludeFileName}")
set(ProgramAppHeader "ClientGame.h")


# ------------------ ProgramConfiguration ------------------ #
set(PROGRAMCLASSNAME				Client)
set(PROGRAMNETWORKINGNAME			ClientNetHandler)
set(PROGRAMLOG						Client)
set(PROGRAMCONFIGURATION			"./Game_NameClient.conf")
set(PROGRAMKEYCONFIGURATION			"./Game_NameKeys.conf")
set(PROGRAMCHECKCONFIGFUNCNAME		"Client::CheckGameConfigurationVariables")
set(PROGRAMCHECKKEYCONFIGFUNCNAME	"Client::CheckGameKeyConfigVariables")
set(WINDOWTITLEGENFUNCTION			"Client::GenerateWindowTitle()")
set(USERREADABLEIDENTIFICATION		"\"Game_Name client version \" GAME_VERSIONS")

# Configure the files
StandardConfigureExecutableMainAndInclude("${BaseIncludeFileName}" "Main.cpp"
  "${BaseSubFolder}" "${PROJECT_SOURCE_DIR}/${BaseSubFolder}")


file(GLOB CoreGroup "*.cpp" "*.h")

source_group("Core" FILES ${CoreGroup})

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(CurrentProjectName Client)
set(AllProjectFiles
  ${CoreGroup}
  )

# Include the common file
set(CREATE_CONSOLE_APP OFF)
include(LeviathanUsingProject)
# include(Game_NameProject)

# The project is now defined
