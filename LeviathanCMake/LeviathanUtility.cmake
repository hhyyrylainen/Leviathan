# Common macros for Leviathan cmake files to be shorter

# Makes a list unique and removes debug and optimized
macro(MakeUniqueAndSanitizeLibraryList TargetList)
  
  list(REMOVE_DUPLICATES ${TargetList})
  list(REMOVE_ITEM ${TargetList} debug)
  list(REMOVE_ITEM ${TargetList} optimized)

endmacro(MakeUniqueAndSanitizeLibraryList TargetList)

# Removes libraries with a "_d" suffix from a list
macro(RemoveLibsWithDebugSuffix TargetList)
  
  set(PassedItems)

  foreach(clib ${${TargetList}})
    
    string(REGEX MATCH ".+_d" item ${clib})

    if(NOT item)
      
      set(PassedItems ${PassedItems} ${clib})

    endif()
  endforeach()
  
  set(${TargetList} ${PassedItems})

endmacro(RemoveLibsWithDebugSuffix TargetList)

# Removes all .cpp files
macro(RemoveSourceFilesFromList TargetList)
  
  set(PassedItems)

  foreach(clib ${${TargetList}})
    
    string(REGEX MATCH "\\.cpp" item ${clib})
    string(REGEX MATCH "\\.c" item2 ${clib})
    
    if(NOT item AND NOT item2)
      
      set(PassedItems ${PassedItems} ${clib})

    endif()
  endforeach()
  
  set(${TargetList} ${PassedItems})

endmacro(RemoveSourceFilesFromList TargetList)

# Goes through a list and globs all files that have the same name as any list item
# Useful for finding lib.so lib.so.2.5.4 and all other variants
macro(GlobAllVariants InputList Output)
  
  set(TmpRes)

  foreach(clib ${${InputList}})
    
    get_filename_component(BASE_PATH "${clib}" DIRECTORY)
    get_filename_component(BASE_NAME "${clib}" NAME_WE)
    
    if(WIN32)
      file(GLOB ActualNeededFiles "${BASE_PATH}/${BASE_NAME}*dll*")
    else()
      file(GLOB ActualNeededFiles "${BASE_PATH}/${BASE_NAME}*so*")
    endif()
    
    set(TmpRes ${TmpRes} ${ActualNeededFiles})

  endforeach(clib)
  
  set(${Output} ${TmpRes})

endmacro(GlobAllVariants InputList Output)

# Configures leviathan executable main source file
macro(StandardConfigureExecutableMain MainFileName UniquePrefix
    TargetFolder)
  
  # Configure the main file
  configure_file("${LEVIATHAN_SRC}/File Templates/BaseLeviathanProjectMain.cpp.in"
    "${PROJECT_BINARY_DIR}/${UniquePrefix}/${MainFileName}")
  
  # Make a readonly copy of it
  file(COPY "${PROJECT_BINARY_DIR}/${UniquePrefix}/${MainFileName}"
    DESTINATION "${TargetFolder}"
    NO_SOURCE_PERMISSIONS
    FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
  
endmacro()

# Configures leviathan executable header and source file
# so that they are read only in the source directory
# UniquePrefix makes sure that the include files in the binary dir have unique names
macro(StandardConfigureExecutableMainAndInclude IncludeFileName MainFileName UniquePrefix
    TargetFolder)

  # Configure the includes file
  configure_file("${LEVIATHAN_SRC}/File Templates/BaseLeviathanProjectIncludes.h.in" 
    "${PROJECT_BINARY_DIR}/${UniquePrefix}/${IncludeFileName}")

  # Make a readonly copy of it
  file(COPY "${PROJECT_BINARY_DIR}/${UniquePrefix}/${IncludeFileName}"
    DESTINATION "${TargetFolder}"
    NO_SOURCE_PERMISSIONS
    FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
  
  # Configure the main file
  StandardConfigureExecutableMain("${MainFileName}" "${UniquePrefix}" "${TargetFolder}")
   
  
endmacro()

# Helper for installing the contents of a folder
function(InstallContentsOfFolder folder target)

  file(GLOB MIXED "${folder}/*")

  foreach(ITEM ${MIXED})
    if(IS_DIRECTORY "${ITEM}")
      list(APPEND FOUND_DIRS "${ITEM}")
    else()
      list(APPEND FOUND_FILES "${ITEM}")
    endif()
  endforeach()

  install(FILES ${FOUND_FILES} DESTINATION "${target}")
  install(DIRECTORY ${FOUND_DIRS} DESTINATION "${target}")

endfunction()

# Helpers for ruby setup system running
function(AddRubyGeneratedFile)

  cmake_parse_arguments(
    PARSED_ARGS
    ""
    ""
    "OUTPUT;DEPENDS;PARAMS"
    ${ARGN}
    )

  file(GLOB FILE_GEN_PARTS "${LEVIATHAN_SRC}/Helpers/*.rb")

  list(GET PARSED_ARGS_PARAMS 0 PRIMARY_SCRIPT)
  
  add_custom_command(OUTPUT ${PARSED_ARGS_OUTPUT}
    COMMAND "ruby" ${PARSED_ARGS_PARAMS}
    DEPENDS ${FILE_GEN_PARTS} ${PRIMARY_SCRIPT} ${PARSED_ARGS_DEPENDS}
    )
endfunction()

