# File that defines common project based on variables

# Potential precompiled header
if(WIN32)
    if(${UsePrecompiledHeaderForProject})
        # precompiled headers
        # remove from list before adding back
        list(REMOVE_ITEM AllProjectFiles ${PrecompiledSourceName})

        # precompiled header
        ADD_MSVC_PRECOMPILED_HEADER(${PrecompiledHeaderName} ${PrecompiledSourceName} AllProjectFiles)
    endif(${UsePrecompiledHeaderForProject})
endif(WIN32)

message(STATUS "Adding project: " ${CurrentProjectName})
# Define the project
if(WIN32)
    add_executable(${CurrentProjectName} WIN32 ${AllProjectFiles})
else(WIN32)
    add_executable(${CurrentProjectName} ${AllProjectFiles})
endif(WIN32)

set_target_properties(${CurrentProjectName} PROPERTIES DEBUG_POSTFIX D)

# Possible debug installs
if(NOT INSTALL_ONLY_RELEASE OR INSTALL_CREATE_SDK)
    set(InstallConfigs "Release" "Debug")
else()
    set(InstallConfigs "Release")
endif()

install(TARGETS ${CurrentProjectName} DESTINATION bin CONFIGURATIONS ${InstallConfigs})

# linking to engine
target_link_libraries(${CurrentProjectName} Engine)

target_link_libraries(${CurrentProjectName} optimized ${LINK_LIBS_TOENGINE_RELEASE})
target_link_libraries(${CurrentProjectName} debug ${LINK_LIBS_TOENGINE_DEBUG})


# If the output directory is wrong then we need to use this
if(WIN32 AND NOT MINGW)

    message(STATUS "Creating Windows launchers for project: " ${CurrentProjectName})
    # set working directory when we want to run this
    create_target_launcher(${CurrentProjectName} RUNTIME_LIBRARY_DIRS "${PROJECT_BINARY_DIR}/bin/" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")

    # post build copy
    ADD_CUSTOM_COMMAND(TARGET ${CurrentProjectName} POST_BUILD COMMAND copy ARGS 
        "\"$(SolutionDir)bin\\$(Configuration)\\$(TargetName).exe\" \"$(SolutionDir)bin\\$(TargetName).exe\""
    )
endif(WIN32 AND NOT MINGW)





