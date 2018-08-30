message(STATUS "Copying Leviathan files...")

######### Debugging Symbols
# Create debugging info directory
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/Symbols")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/Symbols/Dumps")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/Test")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/Data")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/Data/Shaders")


# Copy the tools
# Set the symbol tool name
if(UNIX)
  set(SYMBOL_EXTRACTOR "${LEVIATHAN_SRC}/CreateSymbolsForTarget.sh")
elseif(WIN32)
  set(SYMBOL_EXTRACTOR "${LEVIATHAN_SRC}/CreateSymbolsForTarget.bat")
endif()

file(COPY "${LEVIATHAN_SRC}/MoveSymbolFile.sh"
  "${LEVIATHAN_SRC}/MoveSymbolFile.bat" 
  "${LEVIATHAN_SRC}/CreateSymbolsForTarget.sh"
  "${LEVIATHAN_SRC}/CreateSymbolsForTarget.bat"
  DESTINATION "${PROJECT_BINARY_DIR}/Symbols")


# copy data from bin directory
# This will be all the binaries that need to be copied to bin/lib and installed to bin/lib
# Glob them all
if(WIN32)
  file(GLOB ALL_DYNAMIC_LIBRARIES "${LEVIATHAN_SRC}/build/ThirdParty/lib/*.dll")
else()
  file(GLOB ALL_DYNAMIC_LIBRARIES "${LEVIATHAN_SRC}/build/ThirdParty/lib/*.so*")
  file(GLOB LINUX_LIBS_IN64 "${LEVIATHAN_SRC}/build/ThirdParty/lib64/*.so*")
  list(APPEND ALL_DYNAMIC_LIBRARIES ${LINUX_LIBS_IN64})
endif()

# copy data directory
if(NOT LEVIATHAN_SKIP_OPTIONAL_ASSETS)

  # The bin-folder files
  file(GLOB DataMoveFiles "${LEVIATHAN_SRC}/bin/Data/*")
  file(COPY ${DataMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data")

  # The script files folder
  install(DIRECTORY "Scripts" DESTINATION bin/Data)
  install(DIRECTORY "CoreOgreScripts" DESTINATION bin)
  # we need to specifically install the directories
  install(DIRECTORY "bin/Data/Fonts" DESTINATION bin/Data)
  install(DIRECTORY "bin/Data/Models" DESTINATION bin/Data)
  install(DIRECTORY "bin/Data/Sound" DESTINATION bin/Data)
  install(DIRECTORY "bin/Data/Textures" DESTINATION bin/Data)
  install(DIRECTORY "bin/Data/Materials" DESTINATION bin/Data)
  install(DIRECTORY "bin/Data/Screenshots" DESTINATION bin/Data)
  install(DIRECTORY "bin/Data/Cache" DESTINATION bin/Data)
  install(DIRECTORY "bin/Data/Videos" DESTINATION bin/Videos)

  # Copy data from the scripts folder to the bin folder
  file(GLOB ScriptsMoveFiles "${LEVIATHAN_SRC}/Scripts/*")
  file(COPY ${ScriptsMoveFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/Data/Scripts")
  
endif()

if(NOT ONLY_DOCUMENTATION)
  # Copy the crucial Ogre scripts
  file(GLOB CoreOgreScriptsMoveFiles "${LEVIATHAN_SRC}/bin/CoreOgreScripts/*")
  file(COPY ${CoreOgreScriptsMoveFiles} DESTINATION
    "${PROJECT_BINARY_DIR}/bin/CoreOgreScripts")
  install(DIRECTORY "${LEVIATHAN_SRC}/CoreOgreScripts" DESTINATION "bin/CoreOgreScripts")

  # And core shaders and materials
  file(COPY "${LEVIATHAN_SRC}/bin/Data/Shaders/CoreShaders" DESTINATION
    "${PROJECT_BINARY_DIR}/bin/Data/Shaders/")
  install(DIRECTORY "${LEVIATHAN_SRC}/bin/Data/Shaders/CoreShaders" DESTINATION
    "bin/Data/Shaders/")

  file(COPY "${LEVIATHAN_SRC}/bin/Data/Materials/CoreMaterials" DESTINATION
    "${PROJECT_BINARY_DIR}/bin/Data/Materials/")
  install(DIRECTORY "${LEVIATHAN_SRC}/bin/Data/Materials/CoreMaterials" DESTINATION
    "bin/Data/Materials/")

  # GUI resources needed by the editor
  file(COPY "${LEVIATHAN_SRC}/bin/Data/JSVendor" DESTINATION
    "${PROJECT_BINARY_DIR}/bin/Data/")
  install(DIRECTORY "${LEVIATHAN_SRC}/bin/Data/JSVendor" DESTINATION
    "bin/Data/")

  # Copy additional CEF stuff
  file(GLOB CEF_BLOBS "${LEVIATHAN_SRC}/build/ThirdParty/cefextrablobs/*.bin")
  file(COPY ${CEF_BLOBS} DESTINATION "${PROJECT_BINARY_DIR}/bin/")
  file(COPY "${LEVIATHAN_SRC}/build/ThirdParty/swiftshader"
    DESTINATION "${PROJECT_BINARY_DIR}/bin")
  file(GLOB CEF_RESOURCES "${LEVIATHAN_SRC}/build/ThirdParty/Resources/*")
  file(COPY ${CEF_RESOURCES}
    DESTINATION "${PROJECT_BINARY_DIR}/bin/")

endif()

# Boost files
# If we are not using static linking we need to copy everything
if(NOT Boost_USE_STATIC_LIBS)

  if(WIN32)
    
    # We need to replace .lib files with .dll when copying
    foreach(lib ${LEVIATHAN_BOOST_FILECOPY})

      string(REGEX REPLACE "\\.lib" ".dll" lib ${lib})

      list(APPEND ALL_DYNAMIC_LIBRARIES ${lib})
      
    endforeach()
    
  else()
    
    list(APPEND ALL_DYNAMIC_LIBRARIES ${LEVIATHAN_BOOST_FILECOPY})
  endif()

endif()

if(WIN32)

  if(USING_LEAP)
    list(APPEND ALL_DYNAMIC_LIBRARIES "Leap/lib/x64/Leap.dll")
  endif()
  
  file(GLOB THIRD_PARTY_DLLS "${LEVIATHAN_SRC}/build/ThirdParty/lib/**/*.dll"
    # No clue why this line needed to be added for this to work
    "${LEVIATHAN_SRC}/build/ThirdParty/bin/*.dll"
    "${LEVIATHAN_SRC}/build/ThirdParty/bin/**/*.dll")
  list(APPEND ALL_DYNAMIC_LIBRARIES ${THIRD_PARTY_DLLS})
else()

  # linux variants of the copy functions
  if(USING_LEAP)
    list(APPEND ALL_DYNAMIC_LIBRARIES "Leap/lib/x64/libLeap.so")
  endif()
  
  file(GLOB THIRD_PARTY_DLLS "${LEVIATHAN_SRC}/build/ThirdParty/lib/**/*.so*")
  list(APPEND ALL_DYNAMIC_LIBRARIES ${THIRD_PARTY_DLLS})
  
endif(WIN32)

if(UNIX)
  # Go through all the libraries and add all name variants to the moved files
  GlobAllVariants(ALL_DYNAMIC_LIBRARIES SanitizedList)
  set(ALL_DYNAMIC_LIBRARIES ${SanitizedList})
endif()

set(RawFilesToMove ${ALL_DYNAMIC_LIBRARIES} ${SDL_LIBRARIES})
  
MakeUniqueAndSanitizeLibraryList(RawFilesToMove)

# On windows need to filter out second sdl file
if(WIN32)
  list(FILTER RawFilesToMove EXCLUDE REGEX ".*/relwithdebinfo/SDL2.dll")
endif()

# message(STATUS "Required library list is: ${RawFilesToMove}")

if(WIN32)
  # To be able to debug move all the dlls to the bin folder
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
  file(COPY ${RawFilesToMove} DESTINATION "${PROJECT_BINARY_DIR}/bin")
else()
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/bin/lib")
  file(COPY ${RawFilesToMove} DESTINATION "${PROJECT_BINARY_DIR}/bin/lib")
endif()

# Tools
# file(GLOB AllTools "${LEVIATHAN_SRC}/build/ThirdParty/bin/*")

# file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tools")
# file(COPY ${AllTools} DESTINATION "${PROJECT_BINARY_DIR}/tools")
  
