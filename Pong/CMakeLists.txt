# Pong game CMake


# get all files in their groups
file(GLOB PSourceFiles *.cpp)
file(GLOB PHeaderFiles *.h) 


# resource files
file(GLOB PResourceFiles "resource.h" "*.rc")

# additional .txts
file(GLOB Additional "*.txt")
file(GLOB CMakeListFile "CmakeLists.txt")

list(REMOVE_ITEM Additional ${CMakeListFile})

# file grouping
source_group("Source" FILES ${PSourceFiles})
source_group("Header" FILES ${PHeaderFiles})
source_group("resources" FILES ${PResourceFiles})
source_group("Additional" FILES ${Additional})


# all Pong files in one list
set(PongFiles ${PSourceFiles} ${PHeaderFiles} ${PResourceFiles} ${Additional})

# precompiled headers
# remove from list before adding back
file(GLOB PrecompiledHeaderName "PongIncludes.h")
file(GLOB PrecompiledSourceName "PreCompiled.cpp")
list(REMOVE_ITEM PongFiles ${PrecompiledSourceName})

# precompiled header
ADD_MSVC_PRECOMPILED_HEADER("PongIncludes.h" ${PrecompiledSourceName} PongFiles)

add_executable(Pong WIN32 ${PongFiles})
set_target_properties(Pong PROPERTIES DEBUG_POSTFIX D)

# Possible debug installs
set(InstallConfigurationNames "Release")
if(NOT INSTALL_ONLY_RELEASE)
# Append the Debug configuration if we don't want only release release
list(APPEND InstallConfigurationNames "Debug")
endif(NOT INSTALL_ONLY_RELEASE)

install(TARGETS Pong DESTINATION bin CONFIGURATIONS ${InstallConfigurationNames})

# linking to engine
target_link_libraries(Pong Engine)

# set working directory when we want to run this
create_target_launcher(Pong RUNTIME_LIBRARY_DIRS "${PROJECT_BINARY_DIR}/bin/" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")

# post build copy
ADD_CUSTOM_COMMAND(TARGET Pong POST_BUILD COMMAND copy ARGS 
    "\"$(SolutionDir)Pong\\$(Configuration)\\$(TargetName).exe\" \"$(SolutionDir)bin\\$(TargetName).exe\""
)