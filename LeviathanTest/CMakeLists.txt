# LeviathanTest application CMake

if(NOT CREATE_UE4_PLUGIN)
set(TestSources
  resource.h
  TestMain.cpp
  TestFiles/PartialEngine.h TestFiles/PartialEngine.cpp

  TestFiles/StringIterator.cpp
  TestFiles/StringOperations.cpp
  TestFiles/ExtraAlgorithms.cpp
  TestFiles/IDFactory.cpp
  TestFiles/Script.cpp
  TestFiles/HashTesting.cpp
  TestFiles/ObjectFiles.cpp
  TestFiles/Floats.cpp
  TestFiles/Regex.cpp
  TestFiles/DataBlock.cpp
  TestFiles/DeltaStates.cpp
  TestFiles/Sendable.cpp
  TestFiles/NamedVars.cpp
  TestFiles/Components.cpp
  TestFiles/StdBehaviour.cpp
  TestFiles/PacketsAndConnection.cpp
  TestFiles/PacketFormat.cpp
  TestFiles/NetworkTestHelpers.h
  )
else()
  # Just a few tests required
set(TestSources
  resource.h
  TestMain.cpp
  TestFiles/StringIterator.cpp
  TestFiles/StringOperations.cpp
  TestFiles/ObjectFiles.cpp
  TestFiles/Floats.cpp
  TestFiles/Regex.cpp
  TestFiles/NamedVars.cpp
  TestFiles/StdBehaviour.cpp
  )
endif()

set(ExtraFiles "DummyLog.cpp" "DummyLog.h")

include_directories("catch")

set(CurrentProjectName LeviathanTest)

set(AllProjectFiles ${TestSources} ${ExtraFiles})

# Include the common file
set(CREATE_CONSOLE_APP ON)
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


  
