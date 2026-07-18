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
if(APPLE)
	set(outputFolderPrefix arm64)
elseif(B3D_IS_64BIT)
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
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Globbing.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/PostBuild.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/MonoAot.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/CompileFlags.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/SourceDependencyUtility.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/DownloadPackageUtility.cmake)
include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/FindPackageUtility.cmake)

#######################################################################################
######################## Platform discovery ###########################################
#######################################################################################
# Platform-specific code lives under Framework/Platform/<X>/, each a self-contained
# overlay of Framework/Source (see Framework/Platform/README.md). This step discovers
# the available platforms and reads each platform's metadata (Platform.cmake)

set(B3D_PLATFORM_ROOT ${B3D_FRAMEWORK_ROOT_FOLDER}/Platform)
set(B3D_BUILTIN_PLATFORMS Win32 MacOS Linux Unix)

# The host is always a valid build target.
if(WIN32)
	set(B3D_HOST_PLATFORM Win32)
elseif(APPLE)
	set(B3D_HOST_PLATFORM MacOS)
elseif(LINUX)
	set(B3D_HOST_PLATFORM Linux)
endif()

# Discover available platforms
set(B3D_PLATFORM_CHOICES ${B3D_HOST_PLATFORM})

if(EXISTS ${B3D_PLATFORM_ROOT})
	file(GLOB B3D_PLATFORM_FOLDERS RELATIVE ${B3D_PLATFORM_ROOT} ${B3D_PLATFORM_ROOT}/*)
	foreach(platformName ${B3D_PLATFORM_FOLDERS})
		if(NOT IS_DIRECTORY ${B3D_PLATFORM_ROOT}/${platformName})
			continue()
		endif()

		# Per-platform convention paths (always set for present platforms)
		set(B3D_PLATFORM_${platformName}_SOURCE_FOLDER ${B3D_PLATFORM_ROOT}/${platformName}/Source)
		if(EXISTS ${B3D_PLATFORM_ROOT}/${platformName}/Dependencies)
			set(B3D_PLATFORM_${platformName}_DEPENDENCIES_FOLDER ${B3D_PLATFORM_ROOT}/${platformName}/Dependencies)
		else()
			set(B3D_PLATFORM_${platformName}_DEPENDENCIES_FOLDER "")
		endif()

		if(NOT ${platformName} IN_LIST B3D_BUILTIN_PLATFORMS)
			list(APPEND B3D_PLATFORM_CHOICES ${platformName})
		endif()
	endforeach()
endif()

# The single platform this configuration builds for. Defaults to the host.
set(B3D_PLATFORM ${B3D_HOST_PLATFORM} CACHE STRING "Target platform to build for.")
set_property(CACHE B3D_PLATFORM PROPERTY STRINGS ${B3D_PLATFORM_CHOICES})

# Experimental GPU backends are hidden by default: they are excluded from the
# platform's GPU backend choices and are not built as part of B3D_BUILD_ALL_PLUGINS. Declared before the
# platform meta-data include below, since each platform's Platform.cmake decides its backend choices.
set(B3D_ENABLE_EXPERIMENTAL_GPU_BACKEND OFF CACHE BOOL "If true, experimental in-development GPU backends (currently DirectX12) are offered as GPU backend choices and are included when building all plugins.")

# Include platform meta-data (Platform.cmake)
set(B3D_PLATFORM_METADATA ${B3D_PLATFORM_ROOT}/${B3D_PLATFORM}/Platform.cmake)
if(NOT EXISTS ${B3D_PLATFORM_METADATA})
	message(FATAL_ERROR "Selected platform '${B3D_PLATFORM}' is not available. ")
endif()

include(${B3D_PLATFORM_METADATA})
