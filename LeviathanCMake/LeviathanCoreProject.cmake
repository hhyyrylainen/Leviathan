# File that defines common project based on variables

message(STATUS "Adding project: " ${CurrentProjectName})
# Define the project
if(WIN32)
    add_executable(${CurrentProjectName} WIN32 ${AllProjectFiles})
else(WIN32)
    add_executable(${CurrentProjectName} ${AllProjectFiles})
endif(WIN32)

set_target_properties(${CurrentProjectName} PROPERTIES DEBUG_POSTFIX D)

install(TARGETS ${CurrentProjectName} DESTINATION bin)

# Strip symbols
if(STRIP_SYMBOLS_ON_INSTALL AND UNIX)
  
  # Install code that strips the symbols
  install(CODE "execute_process(COMMAND strip ${CurrentProjectName} WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\")")

endif()

# linking to engine
target_link_libraries(${CurrentProjectName} Engine)

set(FinalTargetLinkLibraries ${LINK_LIBS_TOENGINE_RELEASE} ${LINK_LIBS_TOENGINE_DEBUG})

target_link_libraries(${CurrentProjectName} ${FinalTargetLinkLibraries})

# speed up build
cotire(${CurrentProjectName})

# If the output directory is wrong then we need to use this
if(WIN32 AND NOT MINGW)

    include(CreateLaunchers)

    message(STATUS "Creating Windows launchers for project: " ${CurrentProjectName})
    # set working directory when we want to run this
    create_target_launcher(${CurrentProjectName} RUNTIME_LIBRARY_DIRS "${PROJECT_BINARY_DIR}/bin/"
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")

    # post build copy
    ADD_CUSTOM_COMMAND(TARGET ${CurrentProjectName} POST_BUILD COMMAND copy ARGS 
        "\"$(SolutionDir)bin\\$(Configuration)\\$(TargetName).exe\" \"$(SolutionDir)bin\\$(TargetName).exe\""
    )
endif(WIN32 AND NOT MINGW)

# Creating symbols after building
# When not USE_BREAKPAD this won't be ran automatically
add_custom_target(${CurrentProjectName}_Symbols ${SYMBOL_EXTRACTOR} "${CMAKE_BINARY_DIR}/bin/${CurrentProjectName}"
  DEPENDS ${CurrentProjectName} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Symbols VERBATIM)





