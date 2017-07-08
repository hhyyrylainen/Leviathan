# File that defines common project based on variables

message(STATUS "Adding project: " ${CurrentProjectName})

add_definitions(-DLEVIATHAN_BUILD)

# Define the project
if(WIN32 AND NOT CREATE_CONSOLE_APP)
    add_executable(${CurrentProjectName} WIN32 ${AllProjectFiles})
else()
    add_executable(${CurrentProjectName} ${AllProjectFiles})
endif()

set_target_properties(${CurrentProjectName} PROPERTIES DEBUG_POSTFIX D)

install(TARGETS ${CurrentProjectName} DESTINATION bin)

# Strip symbols
if(STRIP_SYMBOLS_ON_INSTALL AND UNIX)
  
  # Install code that strips the symbols
  install(CODE "execute_process(COMMAND strip ${CurrentProjectName} WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\")")

endif()

# linking to engine
target_link_libraries(${CurrentProjectName} Engine)

target_link_libraries(${CurrentProjectName} ${LEVIATHAN_APPLICATION_LIBRARIES})

# speed up build
#cotire(${CurrentProjectName})

# If the output directory is wrong then we need to use this
if(WIN32 AND NOT MINGW)

    # post build copy
    #ADD_CUSTOM_COMMAND(TARGET ${CurrentProjectName} POST_BUILD COMMAND copy ARGS 
    #    "\"$(SolutionDir)bin\\$(Configuration)\\$(TargetName).exe\" \"$(SolutionDir)bin\\$(TargetName).exe\""
    #)
endif(WIN32 AND NOT MINGW)

# Creating symbols after building
# When not USE_BREAKPAD this won't be ran automatically
if(UNIX)
add_custom_target(${CurrentProjectName}_Symbols ${SYMBOL_EXTRACTOR} "${CMAKE_BINARY_DIR}/bin/${CurrentProjectName}"
  DEPENDS ${CurrentProjectName} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Symbols VERBATIM)
endif()


