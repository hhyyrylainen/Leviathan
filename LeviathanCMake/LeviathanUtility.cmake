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

