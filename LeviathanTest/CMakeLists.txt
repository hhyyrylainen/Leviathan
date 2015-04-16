# LeviathanTest application CMake

set(TestSources
  resource.h
  TestMain.cpp
  TestFiles/StringIterator.cpp
  TestFiles/Misc.cpp
  TestFiles/StringOperations.cpp
  TestFiles/IDFactory.cpp
  TestFiles/MultiFlag.cpp
  TestFiles/Script.cpp
  TestFiles/Tokenize.cpp
  TestFiles/HashTesting.cpp
  TestFiles/ObjectFiles.cpp
  TestFiles/Floats.cpp
  TestFiles/Regex.cpp
  TestFiles/DeltaStates.cpp
  )

include_directories(catch)

set(CurrentProjectName LeviathanTest)

set(AllProjectFiles ${TestSources})

# Include the common file
include(LeviathanCoreProject)

# The project is now defined

# testing

if(WIN32)
  add_custom_target(check COMMAND "${PROJECT_BINARY_DIR}/bin/LeviathanTest.exe"
    DEPENDS LeviathanTest
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
else()
  add_custom_target(check COMMAND "${PROJECT_BINARY_DIR}/bin/LeviathanTest"
    DEPENDS LeviathanTest
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif()


  
