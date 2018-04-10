# Pong game CMake

# Version
set(PROGRAM_VERSION_STABLE 0)
set(PROGRAM_VERSION_MAJOR 3)
set(PROGRAM_VERSION_MINOR 3)
set(PROGRAM_VERSION_PATCH 0)

set(PROGRAM_VERSION_STR ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}.${PROGRAM_VERSION_MINOR}.${PROGRAM_VERSION_PATCH})
set(PROGRAM_VERSION ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}${PROGRAM_VERSION_MINOR}${PROGRAM_VERSION_PATCH})

set(PROGRAM_VERSIONS "\"${PROGRAM_VERSION_STR}\"")
set(PROGRAM_VERSIONS_ANSI "\"${PROGRAM_VERSION_STR}\"")

set(BaseProgramName "PONG")
set(BaseIncludeFileName "PongIncludes.h")
set(BaseSubFolder "Pong")

# Set all the settings
set(ProgramIncludesHeader "${BaseIncludeFileName}")
set(ProgramAppHeader "PongGame.h")
set(ProgramNamespace Pong)

set(WorldFactoryClass "GameWorldFactory")
set(WorldFactoryInclude "Entities/GameWorldFactory.h")

# ------------------ ProgramConfiguration ------------------ #
set(PROGRAMCLASSNAME				PongGame)
set(PROGRAMLOG						Pong)
set(ENGINECONFIGURATION				"./EngineConf.conf")
set(PROGRAMCONFIGURATION			"./Pong.conf")
set(PROGRAMKEYCONFIGURATION			"./PongKeys.conf")
set(PROGRAMCHECKCONFIGFUNCNAME		"PongGame::CheckGameConfigurationVariables")
set(PROGRAMCHECKKEYCONFIGFUNCNAME	"PongGame::CheckGameKeyConfigVariables")
set(PROGRAMMASTERSERVERINFO			"MasterServerInformation(\"PongMasters.txt\", \"Pong_\" GAME_VERSIONS, \"http://boostslair.com/\", \"/Pong/MastersList.txt\", \"PongCrecentials.txt\", false)")
set(WINDOWTITLEGENFUNCTION			"PongGame::GenerateWindowTitle()")

set(USERREADABLEIDENTIFICATION		"\"Pong client version \" GAME_VERSIONS")
set(GAMENAMEIDENTIFICATION			Pong)
set(GAMEVERSIONIDENTIFICATION		GAME_VERSIONS)

set(PROGRAMUSE_CUSTOMJS 0)

# Configure the files
StandardConfigureExecutableMainAndInclude("${BaseIncludeFileName}" "Main.cpp"
  "${BaseSubFolder}" "${PROJECT_SOURCE_DIR}/${BaseSubFolder}")

# get all files in their groups
file(GLOB PSourceFiles "*.cpp")
file(GLOB PHeaderFiles "*.h") 
file(GLOB Common "../PongCommon/*.h" "../PongCommon/*.cpp")

# resource files
file(GLOB PResourceFiles "resource.h" "*.rc")

# additional .txts
file(GLOB Additional "*.txt")
file(GLOB CMakeListFile "CMakeLists.txt")

list(REMOVE_ITEM Additional ${CMakeListFile})

# file grouping
source_group("Source" FILES ${PSourceFiles})
source_group("Header" FILES ${PHeaderFiles})
source_group("resources" FILES ${PResourceFiles})
source_group("Additional" FILES ${Additional})
source_group("Common" FILES ${Common})

include_directories("${PROJECT_SOURCE_DIR}/PongCommon")

include_directories(${CMAKE_CURRENT_LIST_DIR})

# all Pong files in one list
set(AllProjectFiles ${PSourceFiles} ${PHeaderFiles} ${PResourceFiles} ${Additional} ${Common})

set(CurrentProjectName Pong)

# Include the common file
include(LeviathanCoreProject)

# The project is now defined


