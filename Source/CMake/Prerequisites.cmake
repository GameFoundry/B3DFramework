include(CheckCXXCompilerFlag)

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
	set(B3D_IS_ROOT_FOLDER True)
endif()

set (B3D_DEPENDENCY_DIRECTORY ${B3D_FRAMEWORK_ROOT_FOLDER}/Dependencies)
set (B3D_TOOLS_DIRECTORY ${B3D_FRAMEWORK_ROOT_FOLDER}/Tools)

# Configuration types
if(NOT CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
	if(NOT CMAKE_BUILD_TYPE)
		message(STATUS "Defaulting to release build.")
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
	endif()
endif()

# Includes required for various find_package calls
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Modules/")

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(B3D_IS_64BIT true)
endif()

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

# Global compile & linker flags
### Target at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

## Remove /EHsc from CMAKE_CXX_FLAGS for MSVC to disable exceptions
if (MSVC AND NOT B3D_ENABLE_EXCEPTIONS)
	if(CMAKE_CXX_FLAGS MATCHES "/EHsc")
		string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()
endif()

# Enable colored output
if (CMAKE_GENERATOR STREQUAL "Ninja")
	check_cxx_compiler_flag("-fdiagnostics-color=always" F_DIAGNOSTIC_COLOR_ALWAYS)
	if (F_DIAGNOSTIC_COLOR_ALWAYS)
		add_compile_options("-fdiagnostics-color=always")
	endif()
endif()

set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
set(CMAKE_FIND_FRAMEWORK "LAST")

# Output
if(B3D_IS_64BIT)
	set(outputFolderPrefix x64)
else()
	set(outputFolderPrefix x86)
endif()

set(binaryOutputFolder ${PROJECT_BINARY_DIR}/bin/${outputFolderPrefix})
set(libraryOutputFolder ${PROJECT_BINARY_DIR}/lib/${outputFolderPrefix})

if (B3D_IS_ROOT_FOLDER)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${binaryOutputFolder}/Debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${binaryOutputFolder}/RelWithDebInfo)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${binaryOutputFolder}/MinSizeRel)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${binaryOutputFolder}/Release)

	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${binaryOutputFolder}/Debug)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${binaryOutputFolder}/RelWithDebInfo)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${binaryOutputFolder}/MinSizeRel)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${binaryOutputFolder}/Release)

	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${libraryOutputFolder}/Debug)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${libraryOutputFolder}/RelWithDebInfo)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${libraryOutputFolder}/MinSizeRel)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${libraryOutputFolder}/Release)

	set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)
endif()

# Look for global/system dependencies
if (UNIX)
	# macOS
	if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
		# Find tools used for stripping binaries
		find_program(dsymutilToolPath dsymutil)

		if (NOT dsymutilToolPath)
			message(FATAL_ERROR "Could not find 'dsymutil' tool.")
		endif()

		find_program(stripToolPath strip)
		if (NOT stripToolPath)
			message(FATAL_ERROR "Could not find 'strip' tool.")
		endif()

	# Linux
	else()
		# Find tools used for stripping binaries
	    find_program(objcopyToolPath objcopy)

	    if (NOT objcopyToolPath)
	        message(FATAL_ERROR "Could not find 'objcopy' tool.")
	    endif()

	endif()
endif()

# Common includes
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Utility.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/SourceDependencyUtility.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/DownloadPackageUtility.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/FindPackageUtility.cmake)
