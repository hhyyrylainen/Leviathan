# Sandboxie application CMake


# get all files in their groups
file(GLOB SBSourceFiles *.cpp)
file(GLOB SBHeaderFiles *.h) 


# resource files
file(GLOB SBResourceFiles "resource.h" "*.rc")

# file grouping
source_group("Source" FILES ${SBSourceFiles})
source_group("Header" FILES ${SBHeaderFiles})
source_group("resources" FILES ${SBResourceFiles})



# all Sandboxie files in one list
set(Sandboxiefiles ${SBSourceFiles} ${SBHeaderFiles} ${SBResourceFiles})

if(WIN32)
add_executable(Sandboxie WIN32 ${Sandboxiefiles})
else(WIN32)
add_executable(Sandboxie ${Sandboxiefiles})
endif(WIN32)
set_target_properties(Sandboxie PROPERTIES DEBUG_POSTFIX D)

# Possible debug installs
set(InstallConfigurationNames "Release")
if(NOT INSTALL_ONLY_RELEASE)
# Append the Debug configuration if we don't want only release release
list(APPEND InstallConfigurationNames "Debug")
endif(NOT INSTALL_ONLY_RELEASE)

install(TARGETS Sandboxie DESTINATION bin CONFIGURATIONS ${InstallConfigurationNames})

# linking to engine
target_link_libraries(Sandboxie Engine)

if(NOT WIN32)
target_link_libraries(Sandboxie ${LINUX_LINK_LIBS_TOENGINE})
endif(NOT WIN32)

if(WIN32)
# set working directory when we want to run this
create_target_launcher(Sandboxie RUNTIME_LIBRARY_DIRS "${PROJECT_BINARY_DIR}/bin/" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")

# post build copy
ADD_CUSTOM_COMMAND(TARGET Sandboxie POST_BUILD COMMAND copy ARGS 
    "\"$(SolutionDir)Project_sandboxie\\$(Configuration)\\$(TargetName).exe\" \"$(SolutionDir)bin\\$(TargetName).exe\""
)
else(WIN32)
	#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
endif(WIN32)
