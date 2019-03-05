# LeviathanTest application CMake

# TODO: split tests by enabled features
set(TestSources
  resource.h
  TestMain.cpp
  PartialEngine.h PartialEngine.cpp
  NetworkTestHelpers.h NetworkTestHelpers.cpp

  TestFiles/ReferenceCounted.cpp
  TestFiles/StringIterator.cpp
  TestFiles/StringOperations.cpp
  TestFiles/ExtraAlgorithms.cpp
  TestFiles/IDFactory.cpp
  TestFiles/MathTypes.cpp
  TestFiles/Script.cpp
  TestFiles/ScriptConversions.cpp
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
  TestFiles/Archive.cpp
  TestFiles/PacketFormat.cpp
  TestFiles/PacketsAndConnection.cpp
  TestFiles/GuiTests.cpp
  TestFiles/Delegate.cpp
  TestFiles/Task.cpp
  TestFiles/Key.cpp
  TestFiles/Events.cpp
  TestFiles/GameModule.cpp
  TestFiles/Physics.cpp
  TestFiles/Entities.cpp
  TestFiles/ScriptInterfaces.cpp
  TestFiles/CustomScriptComponents.cpp
  TestFiles/MimeTypes.cpp
  
  TestFiles/CoreEngineTests.cpp

  TestFiles/EntitySynchronizationAndInterpolation.cpp
  )

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
  # TODO: debug target (can't use the linux approach as visual studio
  # can switch between the build types without invoking cmake)
  add_custom_target(check COMMAND "${PROJECT_BINARY_DIR}/bin/LeviathanTest.exe"
    DEPENDS LeviathanTest
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
else()
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_target(check COMMAND "${PROJECT_BINARY_DIR}/bin/LeviathanTestD"
      DEPENDS LeviathanTest
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
  else()
    add_custom_target(check COMMAND "${PROJECT_BINARY_DIR}/bin/LeviathanTest"
      DEPENDS LeviathanTest
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
  endif()
endif()



