################################################################################
# FindLeviathan
#
# Locate Leviathan files
#
# This module defines
#    LEVIATHAN_FOUND, true when everything was found
#    LEVIATHAN_INCLUDE_DIR, where to find headers.
#    LEVIATHAN_LIBRARIES, the LIBRARIES to link against, will include Boost
#    LEVIATHAN_SDK_DIR, the main directory for sdk, this is where to find Newton
#
# 
# Copyright (c) 2014 Henri Hyyryläinen
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Note: this still needs a lot of work to be more useful
#
################################################################################
include(LibFindMacros)

# If the SDK directory is set we will use it
if(EXISTS "$ENV{LEVIATHAN_HOME}")
  
  # Make sure prerequisites are installed
  # Find Boost
  set(Boost_USE_STATIC_LIBS FALSE)

  set(Boost_ADDITIONAL_VERSIONS "1.55" "1.53")

  find_package(Boost COMPONENTS chrono system regex filesystem thread date_time QUIET)

  if(NOT ${Boost_FOUND})
      message(SEND_ERROR "Failed to find Boost libraries: " ${REQUIRED_BOOST_COMPONENTS})        
  endif()

  # locating the Ogre folder
  if(WIN32)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{OGRE_HOME}/CMake/")
    set(OGRE_BASEDIR "$ENV{OGRE_HOME}/")
  endif(WIN32)

  if(UNIX)
    if(EXISTS "/usr/local/lib/OGRE/cmake")

      set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/lib/OGRE/cmake/")
      set(OGRE_BASEDIR "/usr/local/lib/OGRE/")
      
    elseif(EXISTS "/usr/lib/OGRE/cmake")
      
      set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/lib/OGRE/cmake/")
      set(OGRE_BASEDIR "/usr/lib/OGRE/")
    else(EXISTS "/usr/local/lib/OGRE")
      message(SEND_ERROR "Failed to find Ogre module path")
    endif(EXISTS "/usr/local/lib/OGRE/cmake")
  endif(UNIX)

  # Ogre is required
  find_package(OGRE REQUIRED Plugin_ParticleFX Plugin_CgProgramManager Plugin_OctreeZone Overlay Paging)

  # OIS is required
  find_package(OIS REQUIRED)

  # CEGUI is required
  set(CEGUI_VERSION_MAJOR_DEFAULT 9999)
  find_package(CEGUI REQUIRED OgreRenderer CoreWindowRendererSet ExpatParser FreeImageImageCodec)

  if(UNIX)
    find_package(X11)
    find_package(Threads)
  endif()

#  find_package(PkgConfig)
#  pkg_check_modules(PC_LIBXML QUIET libxml-2.0)
#  set(LIBXML2_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

  find_path(LEVIATHAN_INCLUDE_DIR Engine/LeviathanMainDll.h
    HINTS ${PC_LEVIATHAN_INCLUDEDIR} ${PC_LEVIATHAN_INCLUDE_DIRS}
    "$ENV{LEVIATHAN_HOME}"
    )

  # The found path is in the base directory of the SDK folder, we want the include
  # to be in the Engine subdirectory
  if(EXISTS ${LEVIATHAN_INCLUDE_DIR})
    set(LEVIATHAN_INCLUDE_DIR ${LEVIATHAN_INCLUDE_DIR}/Engine)
  endif()

  find_library(LEVIATHAN_LIBRARY NAMES Engine libEngine
    HINTS ${PC_LEVIATHAN_LIBDIR} ${PC_LEVIATHAN_LIBRARY_DIRS} 
    "$ENV{LEVIATHAN_HOME}/build/bin"
    )

  find_library(LEVIATHAN_NEWTON_LIBRARY NAMES Newton libNewton
    HINTS ${PC_LEVIATHAN_LIBDIR} ${PC_LEVIATHAN_LIBRARY_DIRS} 
    "$ENV{LEVIATHAN_HOME}/Newton/lib"
    )

  set(LEVIATHAN_NEWTON_INCLUDE_DIR "$ENV{LEVIATHAN_HOME}/Newton/include")
  set(LEVIATHAN_ANGELSCRIPT_INCLUDE_DIR "$ENV{LEVIATHAN_HOME}/AngelScript/include" "$ENV{LEVIATHAN_HOME}/AngelScript")
  
  # Add boost to the leviathan libraries
  set(LEVIATHAN_LIBRARY ${LEVIATHAN_LIBRARY} ${Boost_LIBRARIES} ${LEVIATHAN_NEWTON_LIBRARY})
  
  # Add Ogre includes to the include folders
  set(LEVIATHAN_INCLUDE_DIR ${LEVIATHAN_INCLUDE_DIR} ${OGRE_INCLUDE_DIR} ${OIS_INCLUDE_DIR} ${CEGUI_INCLUDE_DIR}
    ${LEVIATHAN_NEWTON_INCLUDE_DIR} ${LEVIATHAN_ANGELSCRIPT_INCLUDE_DIR})

  set(LEVIATHAN_LIBRARIES ${LEVIATHAN_LIBRARY})
  set(LEVIATHAN_INCLUDE_DIRS ${LEVIATHAN_INCLUDE_DIR})

  include(FindPackageHandleStandardArgs)
  # handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
  # if all listed variables are TRUE
  find_package_handle_standard_args(Leviathan  DEFAULT_MSG
    LEVIATHAN_LIBRARY LEVIATHAN_INCLUDE_DIR LEVIATHAN_NEWTON_INCLUDE_DIR LEVIATHAN_NEWTON_LIBRARY)

  mark_as_advanced(LEVIATHAN_INCLUDE_DIR LEVIATHAN_LIBRARY)
  
else()

  message(SEND_ERROR "FindLeviathan.cmake could not find SDK directory (LEVIATHAN_HOME) and looking for installed version is not supported")


endif()



