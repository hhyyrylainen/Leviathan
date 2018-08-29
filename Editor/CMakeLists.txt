# Standalone editor for running the Leviathan editor without opening a game

# The editor doesn't have a separate version from the engine as this
# basically lets the engine do everything and has very little code

set(BaseProgramName "LeviathanEditor")
set(BaseIncludeFileName "EditorConfiguredHeader.h")
set(BaseSubFolder "Editor")

# Set all the settings
set(ProgramIncludesHeader "${BaseIncludeFileName}")
set(ProgramAppHeader "EditorApplication.h")
set(ProgramNamespace Editor)

set(WorldFactoryClass "GameWorldFactory")
set(WorldFactoryInclude "Entities/GameWorldFactory.h")

# ------------------ ProgramConfiguration ------------------ #
set(PROGRAMCLASSNAME				EditorApplication)
set(PROGRAMLOG						StandaloneEditor)
set(ENGINECONFIGURATION				"./EngineConf.conf")
set(PROGRAMCONFIGURATION			"./Editor.conf")
set(PROGRAMKEYCONFIGURATION			"./EditorKeys.conf")
set(PROGRAMCHECKCONFIGFUNCNAME		"EditorApplication::CheckGameConfigurationVariables")
set(PROGRAMCHECKKEYCONFIGFUNCNAME	"EditorApplication::CheckGameKeyConfigVariables")
set(PROGRAMMASTERSERVERINFO			"MasterServerInformation()")
set(WINDOWTITLEGENFUNCTION			"EditorApplication::GenerateWindowTitle()")

set(USERREADABLEIDENTIFICATION		"\"Leviathan Standalone Editor for version \" LEVIATHAN_VERSION_ANSIS")
set(GAMENAMEIDENTIFICATION			StandaloneEditor)
set(GAMEVERSIONIDENTIFICATION		GAME_VERSIONS)

set(PROGRAMUSE_CUSTOMJS 0)

# Configure the files
StandardConfigureExecutableMainAndInclude("${BaseIncludeFileName}" "Main.cpp"
  "${BaseSubFolder}" "${PROJECT_SOURCE_DIR}/${BaseSubFolder}")

# get all files in their groups
set(EditorCoreFiles
  "EditorNetHandler.cpp" "EditorNetHandler.h"
  "EditorApplication.cpp" "EditorApplication.h"
  "EditorConfiguredHeader.h"
  "Main.cpp"
  )

# resource files
file(GLOB EditorResourceFiles "resource.h" "*.rc")

# file grouping
source_group("Core" FILES ${EditorCoreFiles})
source_group("resources" FILES ${PResourceFiles})

include_directories(${CMAKE_CURRENT_LIST_DIR})

# all files in one list
set(AllProjectFiles ${EditorCoreFiles} ${EditorResourceFiles})

set(CurrentProjectName LeviathanEditor)

# Include the common file
include(LeviathanCoreProject)

# The project is now defined


