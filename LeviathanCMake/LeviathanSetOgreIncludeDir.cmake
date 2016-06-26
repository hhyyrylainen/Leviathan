# Finds and adds Ogre (+ dependencies) to include_directories

# locating the Ogre folder
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

  find_package(OGRE REQUIRED)
  find_package(OIS REQUIRED)
  find_package(SFML 2 COMPONENTS network system)


  if(NOT OGRE_FOUND)
    message(SEND_ERROR "Failed to find Ogre")
  endif()

  if(NOT OIS_FOUND)
    message(SEND_ERROR "Failed to find OIS")
  endif(NOT OIS_FOUND)
  
  if(NOT SFML_FOUND)
    message(SEND_ERROR "Failed to find SFML 2")
  endif()

  include_directories(${OIS_INCLUDE_DIRS} ${SFML_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS})

endif(UNIX)

