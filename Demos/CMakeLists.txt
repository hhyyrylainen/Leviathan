# Demos of using Leviathan to do things

# Version
set(PROGRAM_VERSION_STABLE 0)
set(PROGRAM_VERSION_MAJOR 1)
set(PROGRAM_VERSION_MINOR 0)
set(PROGRAM_VERSION_PATCH 0)

set(PROGRAM_VERSION_STR ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}.${PROGRAM_VERSION_MINOR}.${PROGRAM_VERSION_PATCH})
set(PROGRAM_VERSION ${PROGRAM_VERSION_STABLE}.${PROGRAM_VERSION_MAJOR}${PROGRAM_VERSION_MINOR}${PROGRAM_VERSION_PATCH})

set(PROGRAM_VERSIONS "\"${PROGRAM_VERSION_STR}\"")
set(PROGRAM_VERSIONS_ANSI "\"${PROGRAM_VERSION_STR}\"")

set(BaseProgramName "Demos")
set(BaseIncludeFileName "DemoVersion.h")
set(BaseSubFolder "Demos")

# Set all the settings
set(ProgramIncludesHeader "${BaseIncludeFileName}")
set(ProgramAppHeader "Demos.h")
set(ProgramNamespace Demos)

set(WorldFactoryClass "GameWorldFactory")
set(WorldFactoryInclude "Entities/GameWorldFactory.h")

# ------------------ ProgramConfiguration ------------------ #
set(PROGRAMCLASSNAME				DemosApplication)
set(PROGRAMLOG						Demos)
set(ENGINECONFIGURATION				"./EngineConf.conf")
set(PROGRAMCONFIGURATION			"./Demos.conf")
set(PROGRAMKEYCONFIGURATION			"./DemosKeys.conf")
set(PROGRAMCHECKCONFIGFUNCNAME		"DemosApplication::CheckGameConfigurationVariables")
set(PROGRAMCHECKKEYCONFIGFUNCNAME	"DemosApplication::CheckGameKeyConfigVariables")
set(PROGRAMMASTERSERVERINFO			"MasterServerInformation()")
set(WINDOWTITLEGENFUNCTION			"DemosApplication::GenerateWindowTitle()")

set(USERREADABLEIDENTIFICATION		"\"Leviathan Demos version \" GAME_VERSIONS")
set(GAMENAMEIDENTIFICATION			Demos)
set(GAMEVERSIONIDENTIFICATION		GAME_VERSIONS)

set(PROGRAMUSE_CUSTOMJS 0)

# Configure the files
StandardConfigureExecutableMainAndInclude("${BaseIncludeFileName}" "Main.cpp"
  "${BaseSubFolder}" "${PROJECT_SOURCE_DIR}/${BaseSubFolder}")

# get all files in their groups
set(DemoCoreFiles
  "DemoNetHandler.cpp" "DemoNetHandler.h"
  "Demos.cpp" "Demos.h"
  "DemoVersion.h"
  "SampleCommon.h"
  "Main.cpp"
  )

set(DemoSample1
  "Sample1.cpp" "Sample1.h"
  ) 

# resource files
file(GLOB DemoResourceFiles "resource.h" "*.rc")

# file grouping
source_group("Core" FILES ${DemoCoreFiles})
source_group("Sample1" FILES ${DemoSample1})
source_group("resources" FILES ${PResourceFiles})

include_directories(${CMAKE_CURRENT_LIST_DIR})

# all files in one list
set(AllProjectFiles ${DemoCoreFiles} ${DemoSample1} ${DemoResourceFiles} )

set(CurrentProjectName Demos)

# Include the common file
include(LeviathanCoreProject)

# The project is now defined


