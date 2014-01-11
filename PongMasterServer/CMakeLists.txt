# Pong master server CMake


# get all files in their groups
file(GLOB PSourceFiles *.cpp)
file(GLOB PHeaderFiles *.h) 


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


# all PongMaster files in one list
set(PongFiles ${PSourceFiles} ${PHeaderFiles} ${PResourceFiles} ${Additional})

if(WIN32)
# precompiled headers
# remove from list before adding back
file(GLOB PrecompiledHeaderName "PongMasterServerIncludes.h")
file(GLOB PrecompiledSourceName "PreCompiled.cpp")
list(REMOVE_ITEM PongFiles ${PrecompiledSourceName})

# precompiled header
ADD_MSVC_PRECOMPILED_HEADER("PongMasterServerIncludes.h" ${PrecompiledSourceName} PongFiles)
endif(WIN32)

if(WIN32)
add_executable(PongMasterServer WIN32 ${PongFiles})
else(WIN32)
add_executable(PongMasterServer ${PongFiles})
endif(WIN32)

set_target_properties(PongMasterServer PROPERTIES DEBUG_POSTFIX D)

# Possible debug installs
set(InstallConfigurationNames "Release")
if(NOT INSTALL_ONLY_RELEASE)
# Append the Debug configuration if we don't want only release release
list(APPEND InstallConfigurationNames "Debug")
endif(NOT INSTALL_ONLY_RELEASE)

install(TARGETS PongMasterServer DESTINATION bin CONFIGURATIONS ${InstallConfigurationNames})

# linking to engine
target_link_libraries(PongMasterServer Engine)

if(NOT WIN32)
target_link_libraries(PongMasterServer ${LINUX_LINK_LIBS_TOENGINE})
endif(NOT WIN32)

if(WIN32)
# set working directory when we want to run this
create_target_launcher(PongMasterServer RUNTIME_LIBRARY_DIRS "${PROJECT_BINARY_DIR}/bin/" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")

# post build copy
ADD_CUSTOM_COMMAND(TARGET PongMasterServer POST_BUILD COMMAND copy ARGS 
    "\"$(SolutionDir)PongMasterServer\\$(Configuration)\\$(TargetName).exe\" \"$(SolutionDir)bin\\$(TargetName).exe\""
)
else(WIN32)

	#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(WIN32)