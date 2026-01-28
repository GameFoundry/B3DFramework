# FrameworkTests.dll - Contains test suites and FrameworkTestSuiteFactory
set(FRAMEWORK_TESTS_FILES_TO_EXCLUDE
	"B3DNetworkTest.cpp"
	"B3DUnitTestRunner.cpp"
)

B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" FRAMEWORK_TESTS_SOURCE_FILES)

# Filter out excluded files
foreach(excludeFile ${FRAMEWORK_TESTS_FILES_TO_EXCLUDE})
	list(FILTER FRAMEWORK_TESTS_SOURCE_FILES EXCLUDE REGEX "${excludeFile}$")
endforeach()

# FrameworkTests target (DLL)
add_library(FrameworkTests SHARED ${FRAMEWORK_TESTS_SOURCE_FILES})

# Common flags
B3DSetDefaultLinkAndCompileFlags(FrameworkTests)

# Defines
target_compile_definitions(FrameworkTests PRIVATE -DB3D_FRAMEWORK_TESTS_EXPORTS)

# Includes
target_include_directories(FrameworkTests PUBLIC
	"${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility"
	"${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility/ThirdParty"
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/TestSuites"
	"${CMAKE_CURRENT_SOURCE_DIR}/Factories"
	"${CMAKE_CURRENT_SOURCE_DIR}/Utility"
	"${CMAKE_CURRENT_SOURCE_DIR}/RTTI")

# Libraries
target_link_libraries(FrameworkTests PUBLIC bsf)

# Plugin dependencies (needed for Core layer tests)
B3DAddRuntimeDependencies(FrameworkTests)
add_dependencies(FrameworkTests bsfFBXImporter bsfFontImporter bsfFreeImgImporter bsfSL)

# IDE specific
set_property(TARGET FrameworkTests PROPERTY FOLDER Tests)

# Install
B3DSetUpPostBuildAndInstallSteps(FrameworkTests)

# UnitTestRunner.exe - Minimal loader that loads test DLLs
add_executable(UnitTestRunner B3DUnitTestRunner.cpp)

# Common flags
B3DSetDefaultLinkAndCompileFlags(UnitTestRunner)

# Working directory
set_target_properties(UnitTestRunner PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

# Includes (needs access to bsf headers for DynamicLibrary, etc.)
target_include_directories(UnitTestRunner PRIVATE
	"${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility"
	"${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility/ThirdParty")

# Libraries - links to bsf for CrashHandler, CommandLine, DynamicLibrary, etc.
target_link_libraries(UnitTestRunner bsf)

# IDE specific
set_property(TARGET UnitTestRunner PROPERTY FOLDER Tests)

# Ensure FrameworkTests is built when UnitTestRunner is built
add_dependencies(UnitTestRunner FrameworkTests)

# Install
B3DSetUpPostBuildAndInstallSteps(UnitTestRunner)

# Register test (runs all layers by default)
add_test(NAME UnitTests COMMAND $<TARGET_FILE:UnitTestRunner>)
