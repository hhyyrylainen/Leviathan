# Leviathan cmake component for setting up linking against all dependencies
include(LeviathanUtility)

# Windows fix for SDL2 (and maybe other library finds, too)
if(WIN32)
  set(CMAKE_PREFIX_PATH "${LEVIATHAN_SRC}/build/ThirdParty")
endif()

# Detect library settings
if(LEVIATHAN_USING_ANGELSCRIPT OR LEVIATHAN_USING_OGRE OR LEVIATHAN_USING_BULLET OR
    LEVIATHAN_USING_CEF OR LEVIATHAN_USING_GUI OR LEVIATHAN_USING_SFML OR
    LEVIATHAN_USING_SDL2 OR LEVIATHAN_USING_LEAP)
    set(LEVIATHAN_USING_DEPENDENCIES ON)
else()
  set(LEVIATHAN_USING_DEPENDENCIES OFF)
endif()

# Find Boost
if(TRUE)

  # Uncomment the next line to get boost debug info
  # set(Boost_DEBUG ON)

  # Might be a good idea to dynamically link Boost
  set(Boost_USE_STATIC_LIBS FALSE)

  set(Boost_ADDITIONAL_VERSIONS "1.66")

  # Other than these that are required are header-only libraries
  set(LEVIATHAN_BOOST_COMPONENTS system filesystem program_options)
  
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
    ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE})

  # Set up referencing of Boost
  include_directories(${Boost_INCLUDE_DIR})

  # All libs are linked by cmake
  add_definitions(-DBOOST_ALL_NO_LIB)
  add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK=1)
  
endif(TRUE)

# Libraries the engine links against
set(LEVIATHAN_ENGINE_LIBRARIES ${Boost_LIBRARIES})

# Plus of course linking against the Engine target
set(LEVIATHAN_APPLICATION_LIBRARIES ${Boost_LIBRARIES})

if(LEVIATHAN_USING_DEPENDENCIES)

  # Require build script to have been ran
  if(NOT EXISTS "${LEVIATHAN_SRC}/build/ThirdParty/lib")
    message(SEND_ERROR "Leviathan build script hasn't installed dependencies")
  endif()

  if(NOT EXISTS "${LEVIATHAN_SRC}/build/ThirdParty/include")
    message(SEND_ERROR "Leviathan build script hasn't installed headers")
  endif()

  find_package(OpenAL REQUIRED)

  # TODO: make this use LEVIATHAN_USING_SDL2
  
  # Set the setup script result directories
  link_directories("${LEVIATHAN_SRC}/build/ThirdParty/lib"
    "${LEVIATHAN_SRC}/build/ThirdParty/lib64"
    "${LEVIATHAN_SRC}/build/ThirdParty/bin"
    )

  include_directories("${LEVIATHAN_SRC}/build/ThirdParty/include"
    # Needed for CEF
    "${LEVIATHAN_SRC}/build/ThirdParty/"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/bullet"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/bsfEngine"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/bsfCore"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/bsfUtility"
    "${LEVIATHAN_SRC}/build/ThirdParty/include/AL"
    "${OPENAL_INCLUDE_DIR}"
    )

  # Find SDL2
  if(LEVIATHAN_USING_SDL2)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR})
  endif()

  # CEF must be linked to first in order for it to be loaded first to avoid a ton of problems
  if(NOT WIN32)
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES cef cef_dll_wrapper)
  else()
    list(APPEND LEVIATHAN_ENGINE_LIBRARIES libcef libcef_dll_wrapper)
  endif()

  list(APPEND LEVIATHAN_ENGINE_LIBRARIES
    LinearMath BulletDynamics Bullet3Dynamics BulletCollision
    sfml-system sfml-network
    # ffmpeg
    avcodec avformat avutil swresample swscale
    alure2 ${OPENAL_LIBRARY}
    ${SDL2_LIBRARY} AngelScriptAddons
    bsf
    )
  

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
  # CEF must also be first here
  # theoretically these aren't always used by the game program but CEF will absolutely
  # break without them being loaded first and that doesn't happen when they aren't here
  if(NOT WIN32)
    list(APPEND LEVIATHAN_APPLICATION_LIBRARIES cef cef_dll_wrapper)
  else()
    list(APPEND LEVIATHAN_APPLICATION_LIBRARIES libcef libcef_dll_wrapper)
  endif()

  # # Currently disabled
  # if(WIN32)
  #   list(APPEND LEVIATHAN_APPLICATION_LIBRARIES cef_sandbox)
  # endif()  
  
  list(APPEND LEVIATHAN_APPLICATION_LIBRARIES bsf
    sfml-system sfml-network AngelScriptAddons)
  
endif()
  

set(LEVIATHAN_APPLICATION_CUSTOMJS_LIBRARIES)
  

if(NOT WIN32)

  list(APPEND LEVIATHAN_APPLICATION_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  
endif()

if(LEVIATHAN_USING_LEAP)
  include_directories("${LEVIATHAN_SRC}/Leap/include")
  include_directories("${LEVIATHAN_SRC}/Leap")
  link_directories("${LEVIATHAN_SRC}/Leap/lib/x64")
endif()


# Breakpad libraries
if(MAKE_RELEASE)

  if(NOT EXISTS "${LEVIATHAN_SRC}/build/ThirdParty/include/breakpad")
    message(SEND_ERROR "Breakpad include dir is missing")
  endif()
  
  if(UNIX)
    list(APPEND LEVIATHAN_APPLICATION_LIBRARIES optimized breakpad_client)
  elseif(WIN32)
    list(APPEND LEVIATHAN_APPLICATION_LIBRARIES optimized exception_handler
      optimized common optimized crash_generation_client)
  else()
    
  endif()

  include_directories("${LEVIATHAN_SRC}/build/ThirdParty/include/breakpad")

  if(UNIX)
    add_definitions("-D__STDC_FORMAT_MACROS")
  endif()
  
endif()


# Copy some macro values
set(LEVIATHAN_NO_DEBUG ${CREATE_SHIPPING})







