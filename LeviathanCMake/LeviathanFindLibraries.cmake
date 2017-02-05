include(LeviathanUtility)

# Ogre is required
if(USE_OGRE AND UNIX)
  
  if(NOT USE_BOOST)
    message(SEND_ERROR "USE_BOOST required for Ogre")
  endif()
  
  if(NOT USE_NEWTON)
    message(SEND_ERROR "USE_NEWTON required for Ogre")
  endif()

  # locating the Ogre folder
  if(WIN32)
	set(CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/CMake/;${CMAKE_MODULE_PATH}")
  endif(WIN32)
  
  if(UNIX)
	if(EXISTS "/usr/local/lib/OGRE/cmake")

	  set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")

	elseif(EXISTS "/usr/lib/OGRE/cmake")

	  set(CMAKE_MODULE_PATH "/usr/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
	else()
	  message(SEND_ERROR "Failed to find Ogre module path.")
	endif()
  endif(UNIX)
  
  # Components that need linking (NB does not include header-only components like bind)
  set(OGRE_BOOST_COMPONENTS thread date_time)

  find_package(OGRE REQUIRED Plugin_ParticleFX Plugin_CgProgramManager Plugin_OctreeZone
    Overlay Paging RenderSystem_GL)

  # OIS is required for input
  find_package(OIS REQUIRED)

  # check if it failed
  if(NOT OIS_FOUND)
    message(SEND_ERROR "Failed to find OIS")
  endif(NOT OIS_FOUND)

  # Find CEGUI
  set(CEGUI_VERSION_MAJOR_DEFAULT 9999)
  find_package(CEGUI REQUIRED OgreRenderer CoreWindowRendererSet ExpatParser FreeImageImageCodec)
  # TODO: allow other parsers/codecs

  if(NOT CEGUI_FOUND)
    message(SEND_ERROR "Failed to locate CEGUI")
  endif()

  set(ALL_CEGUI_LIBRARIES ${CEGUI_LIBRARIES} ${CEGUI_OgreRenderer_LIBRARIES}
    ${CEGUI_CoreWindowRendererSet_LIBRARIES} 
    ${CEGUI_ExpatParser_LIBRARIES} ${CEGUI_FreeImageImageCodec_LIBRARIES})

  # set some additional libraries
  set(ADDITIONAL_OGRE ${OGRE_Plugin_ParticleFX_LIBRARIES} ${OGRE_Plugin_CgProgramManager_LIBRARIES}
    ${OGRE_Plugin_OctreeZone_LIBRARIES} ${OGRE_Overlay_LIBRARIES} ${OGRE_Paging_LIBRARIES}
    ${OGRE_RenderSystem_GL_LIBRARIES})

  # Find sfml
  if(USE_SFML)
    find_package(SFML 2 COMPONENTS network system)

    if(NOT SFML_FOUND)
      message(SEND_ERROR "Failed to find SFML 2")
    endif()
  endif()

  # Find cAudio
  if(USE_CAUDIO)

    find_package(cAudio)
    
    if(NOT cAudio_FOUND)
      message(SEND_ERROR "Failed to find cAudio")
    endif()
  endif()
  
  
endif()

# Find Boost
if(USE_BOOST)
  # Might be a good idea to dynamically link Boost
  set(Boost_USE_STATIC_LIBS FALSE)

  set(Boost_ADDITIONAL_VERSIONS "1.55" "1.53")

  # Other than these that are required are header-only libraries
  set(LEVIATHAN_BOOST_COMPONENTS chrono system filesystem)

  set(REQUIRED_BOOST_COMPONENTS ${LEVIATHAN_BOOST_COMPONENTS} ${OGRE_BOOST_COMPONENTS})

  find_package(Boost COMPONENTS ${REQUIRED_BOOST_COMPONENTS} QUIET)

  if(NOT Boost_FOUND)
    message("Boost not found, retrying with different settings")
    # Try again with the other type of libs
    set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
    find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS})
    
    if(NOT Boost_FOUND)
      message(SEND_ERROR "Failed to find Boost libraries: " ${REQUIRED_BOOST_COMPONENTS})        
    endif(NOT Boost_FOUND)
  endif(NOT Boost_FOUND)

  # Boost is found or the configuration has already failed

  # Set up referencing of Boost
  include_directories(${Boost_INCLUDE_DIR})

  # All libs are linked by cmake
  add_definitions(-DBOOST_ALL_NO_LIB)
endif(USE_BOOST)

#
# Worlds dirtiest hacks for windows
#
# This is because it is such a pain
if(WIN32)

  set(OGRE_LIBRARIES OgreMain.lib RenderSystem_GL.lib RenderSystem_GL3Plus.lib Plugin_ParticleFX.lib
    OIS.lib CEGUIBase-9999.lib CEGUICommonDialogs-9999.lib CEGUICoreWindowRendererSet.lib 
    CEGUIExpatParser.lib CEGUIOgreRenderer-9999.lib CEGUISILLYImageCodec.lib
    )
  
  set(SFML_LIBRARIES optimized sfml-network.lib optimized sfml-system.lib
    debug sfml-network-d.lib debug sfml-system-d.lib
    )
  set(cAudio_LIBRARIES cAudio.lib)
  
  include_directories("${LEVIATHAN_SRC}/Windows/ThirdParty/include")
  include_directories("${LEVIATHAN_SRC}/Windows/ThirdParty/include/cAudio")
  include_directories("${LEVIATHAN_SRC}/Windows/ThirdParty/include/OIS")
  include_directories("${LEVIATHAN_SRC}/Windows/ThirdParty/include/OGRE")
  link_directories("${LEVIATHAN_SRC}/Windows/ThirdParty/lib")
  

endif()

if(NOT WIN32)

  list(APPEND LINK_LIBS_TOENGINE_RELEASE ${CMAKE_THREAD_LIBS_INIT})
endif()

# Library linking
# And Debug libraries for windows
if(USE_NEWTON)
  list(APPEND LINK_LIBS_TOENGINE_RELEASE optimized Newton)
  list(APPEND LINK_LIBS_TOENGINE_DEBUG debug Newton)
  
  list(APPEND LINK_LIBS_ENGINE_RELEASE optimized Newton)
  list(APPEND LINK_LIBS_ENGINE_DEBUG debug Newton)
endif()

if(USE_ANGELSCRIPT)

  if(WIN32)
    list(APPEND LINK_LIBS_ENGINE_RELEASE optimized angelscript64)
    list(APPEND LINK_LIBS_ENGINE_DEBUG debug angelscript64d)
    
  else()
    list(APPEND LINK_LIBS_ENGINE_RELEASE optimized angelscript)
  endif()
endif()

if(USING_LEAP)
  list(APPEND LINK_LIBS_ENGINE_RELEASE optimized Leap)
  #list(APPEND LINK_LIBS_ENGINE_DEBUG debug Leapd)
endif()

if(USE_BOOST)

  list(APPEND LINK_LIBS_TOENGINE_RELEASE ${Boost_LIBRARIES})

endif()

# Breakpad libraries
if(USE_BREAKPAD)

  if(NOT EXISTS "${LEVIATHAN_SRC}/Breakpad/include")
    message(SEND_ERROR "Breakpad include dir is missing")
  endif()
  
  if(UNIX)
    list(APPEND LINK_LIBS_TOENGINE_RELEASE optimized breakpad_client)
  elseif(WIN32)
    list(APPEND LINK_LIBS_TOENGINE_RELEASE optimized exception_handler
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

DefinePreprocessorMacro(USE_BREAKPAD LEVIATHAN_USING_BREAKPAD)

DefinePreprocessorMacro(CREATE_SHIPPING LEVIATHAN_NO_DEBUG)


DefinePreprocessorMacro(CREATE_UE4_PLUGIN LEVIATHAN_CREATE_UE4_PLUGIN)

if(NOT WIN32)

  if(USE_CAUDIO)
    include_directories(${cAudio_INCLUDE_DIRS})
  endif()

  include_directories(${SFML_INCLUDE_DIR})

  if(USE_BREAKPAD)
    include_directories("${LEVIATHAN_SRC}/Breakpad/include")
  endif()


  if(USE_OGRE)
    include_directories(${OIS_INCLUDE_DIRS}	${CEGUI_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS})
  endif()
endif()

include_directories("${LEVIATHAN_SRC}/libraries/include")
link_directories("${LEVIATHAN_SRC}/libraries/lib")


if(USING_LEAP)
  include_directories("${LEVIATHAN_SRC}/Leap/include")
  include_directories("${LEVIATHAN_SRC}/Leap")
  link_directories("${LEVIATHAN_SRC}/Leap/lib/x64")
endif()





