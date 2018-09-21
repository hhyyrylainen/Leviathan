# Leviathan cmake component for setting up linking against all dependencies
include(LeviathanUtility)

# Windows fix for SDL2 (and maybe other library finds, too)
if(WIN32)
  set(CMAKE_PREFIX_PATH "${LEVIATHAN_SRC}/build/ThirdParty")
endif()

# Find Boost
if(USE_BOOST)
  # Uncomment the next line to get boost debug info
  # set(Boost_DEBUG ON)

  # Might be a good idea to dynamically link Boost
  set(Boost_USE_STATIC_LIBS FALSE)

  set(Boost_ADDITIONAL_VERSIONS "1.66")

  # Other than these that are required are header-only libraries
  set(LEVIATHAN_BOOST_COMPONENTS system filesystem)
  
  # set(Boost_DEBUG ON)  
  find_package(Boost COMPONENTS ${LEVIATHAN_BOOST_COMPONENTS})

  if(NOT Boost_FOUND)
    # Automatically print stuff if it failed
    set(Boost_DEBUG ON)
    find_package(Boost COMPONENTS ${LEVIATHAN_BOOST_COMPONENTS})
    message(FATAL_ERROR "Failed to find Boost libraries: " ${REQUIRED_BOOST_COMPONENTS})
  endif(NOT Boost_FOUND)

  # Boost is found or the configuration has already failed

  # Needed for file copy
  set(LEVIATHAN_BOOST_FILECOPY ${Boost_SYSTEM_LIBRARY_RELEASE}
        ${Boost_FILESYSTEM_LIBRARY_RELEASE})

  # Set up referencing of Boost
  include_directories(${Boost_INCLUDE_DIR})

  # All libs are linked by cmake
  add_definitions(-DBOOST_ALL_NO_LIB)
  
endif(USE_BOOST)

if(LEVIATHAN_FULL_BUILD)

  # Require build script to have been ran
  if(NOT EXISTS "${LEVIATHAN_SRC}/build/ThirdParty/lib")
    message(SEND_ERROR "Leviathan build script hasn't installed dependencies")
  endif()

  if(NOT EXISTS "${LEVIATHAN_SRC}/build/ThirdParty/include")
    message(SEND_ERROR "Leviathan build script hasn't installed headers")
  endif()

  # Set the setup script result directories
  link_directories("${LEVIATHAN_SRC}/build/ThirdParty/lib"
    "${LEVIATHAN_SRC}/build/ThirdParty/lib64"
    "${LEVIATHAN_SRC}/build/ThirdParty/bin"
    )
  
  include_directories("${LEVIATHAN_SRC}/build/ThirdParty/include"
    # Needed for CEF
    "${LEVIATHAN_SRC}/build/ThirdParty/"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/newton"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/OGRE"
    )

  # Find SDL2
  if(USE_SDL2)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR})
  endif()


  set(LEVIATHAN_ENGINE_LIBRARIES Newton
    OgreMain OgreHlmsUnlit OgreHlmsPbs
    sfml-system sfml-network
    # ffmpeg
    avcodec avformat avutil swresample swscale
    cAudio
    ${Boost_LIBRARIES} ${SDL2_LIBRARY} AngelScriptAddons
    )

  if(NOT WIN32)
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES cef cef_dll_wrapper)
  else()
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES libcef libcef_dll_wrapper)
  endif()

  # Angelscript is named angelscript64 on windows if 64 bit (which we are using)
  # Now it is named the same as we are using the cmake build for angelscript
  if(WIN32)
    # When using angelscript on windows /LTCG should be specified as a flag
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES optimized angelscript)
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES debug angelscriptd)
    
  else()
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES angelscript)
  endif()

  if(USING_LEAP)
    list(APPEND LINK_LIBS_ENGINE_RELEASE optimized Leap)
    #list(APPEND LINK_LIBS_ENGINE_DEBUG debug Leapd)
  endif()
  
  # Leviathan application libraries
  set(LEVIATHAN_APPLICATION_LIBRARIES Newton ${Boost_LIBRARIES} OgreMain
    OgreHlmsUnlit OgreHlmsPbs
    sfml-system sfml-network AngelScriptAddons)

  set(LEVIATHAN_APPLICATION_CUSTOMJS_LIBRARIES)

  if(NOT WIN32)
    list(APPEND LEVIATHAN_APPLICATION_CUSTOMJS_LIBRARIES cef cef_dll_wrapper)
  else()
    list(APPEND LEVIATHAN_APPLICATION_CUSTOMJS_LIBRARIES libcef libcef_dll_wrapper)
  endif()
  
  # # Currently disabled
  # if(WIN32)
  #   list(APPEND LEVIATHAN_APPLICATION_LIBRARIES cef_sandbox)
  # endif()
  
else()

  set(LEVIATHAN_ENGINE_LIBRARIES)

  # Plus of course linking against the Engine target
  set(LEVIATHAN_APPLICATION_LIBRARIES)
  
  
endif()

if(NOT WIN32)

  list(APPEND LEVIATHAN_APPLICATION_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  
endif()

if(USING_LEAP)
  include_directories("${LEVIATHAN_SRC}/Leap/include")
  include_directories("${LEVIATHAN_SRC}/Leap")
  link_directories("${LEVIATHAN_SRC}/Leap/lib/x64")
endif()


# Breakpad libraries
if(USE_BREAKPAD)

  if(NOT EXISTS "${LEVIATHAN_SRC}/Breakpad/include")
    message(SEND_ERROR "Breakpad include dir is missing")
  endif()

  include_directories("${LEVIATHAN_SRC}/Breakpad/include")
  
  if(UNIX)
    list(APPEND LEVIATHAN_APPLICATION_LIBRARIES optimized breakpad_client)
  elseif(WIN32)
    list(APPEND LEVIATHAN_APPLICATION_LIBRARIES optimized exception_handler
      optimized common optimized crash_generation_client)
  else()
    
  endif()

  # Link dir
  link_directories("${LEVIATHAN_SRC}/Breakpad/lib")
  
endif()


# Setup predefined macros
DefinePreprocessorMacro(USE_ANGELSCRIPT LEVIATHAN_USING_ANGELSCRIPT)
DefinePreprocessorMacro(USE_BOOST LEVIATHAN_USING_BOOST)
DefinePreprocessorMacro(USE_OGRE LEVIATHAN_USING_OGRE)
DefinePreprocessorMacro(USE_NEWTON LEVIATHAN_USING_NEWTON)
DefinePreprocessorMacro(USE_SFML LEVIATHAN_USING_SFML)
DefinePreprocessorMacro(USE_LEAP LEVIATHAN_USING_LEAP)
DefinePreprocessorMacro(USE_SDL2 LEVIATHAN_USING_SDL2)

DefinePreprocessorMacro(USE_BREAKPAD LEVIATHAN_USING_BREAKPAD)

DefinePreprocessorMacro(CREATE_SHIPPING LEVIATHAN_NO_DEBUG)


DefinePreprocessorMacro(CREATE_UE4_PLUGIN LEVIATHAN_CREATE_UE4_PLUGIN)







