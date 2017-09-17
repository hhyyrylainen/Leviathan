# Leviathan cmake component for setting up linking against all dependencies
include(LeviathanUtility)

# Find Boost
if(USE_BOOST)
  # Might be a good idea to dynamically link Boost
  set(Boost_USE_STATIC_LIBS FALSE)

  set(Boost_ADDITIONAL_VERSIONS "1.55" "1.53")

  # Other than these that are required are header-only libraries
  set(LEVIATHAN_BOOST_COMPONENTS chrono system filesystem)
  
  find_package(Boost COMPONENTS ${LEVIATHAN_BOOST_COMPONENTS} QUIET)

  if(NOT Boost_FOUND)
    message(SEND_ERROR "Failed to find Boost libraries: " ${REQUIRED_BOOST_COMPONENTS})
  endif(NOT Boost_FOUND)

  # Boost is found or the configuration has already failed

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
  link_directories("${LEVIATHAN_SRC}/build/ThirdParty/lib")
  
  include_directories("${LEVIATHAN_SRC}/build/ThirdParty/include")
  include_directories("${LEVIATHAN_SRC}/build/ThirdParty/include/newton")
  include_directories("${LEVIATHAN_SRC}/build/ThirdParty/include/OGRE")
  include_directories("${LEVIATHAN_SRC}/build/ThirdParty/include/cegui-9999")

  # Find SDL2
  if(USE_SDL2)
    # TODO windows version
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR})
  endif()


  set(LEVIATHAN_ENGINE_LIBRARIES Newton angelscript OgreMain CEGUIBase-9999
    CEGUICommonDialogs-9999
    # CEGUICoreWindowRendererSet CEGUIExpatParser CEGUISILLYImageCodec
    CEGUIOgreRenderer-9999 sfml-system sfml-audio sfml-network
    # ffmpeg
    avcodec avformat avutil swresample swscale
    ${Boost_LIBRARIES} ${SDL2_LIBRARY})

  if(WIN32)
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES optimized angelscript64)
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES debug angelscript64d)
    
  else()
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES optimized angelscript)
  endif()

  if(USING_LEAP)
    list(APPEND LINK_LIBS_ENGINE_RELEASE optimized Leap)
    #list(APPEND LINK_LIBS_ENGINE_DEBUG debug Leapd)
  endif()
  
  # Leviathan application libraries
  set(LEVIATHAN_APPLICATION_LIBRARIES Newton ${Boost_LIBRARIES} OgreMain)
  
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







