#######################################################################################
############################## Find package helpers ###################################
#######################################################################################

# Marks the beginning of a CMake Find* script, to be used with the find_package() command.
#
# @param	packageName		Name of the package that's being located.
MACRO(B3DStartFindPackage packageName)
	message(STATUS "Looking for ${packageName} installation...")
ENDMACRO()

# Resolves the on-disk location of a bundled dependency. The host platform uses the shared
# Framework/Dependencies tree. A cross-compiled (non-host) platform that ships its own
# dependency tree under Framework/Platform/<platform>/Dependencies uses that instead, so its
# archives never collide with the host's in the shared folder (both are, e.g., x64).
#
# @param	packageName		Name of the package (also its sub-folder name).
# @param	resultVar		Output variable to receive the resolved install directory.
MACRO(B3DGetBundledDependencyFolder packageName resultVar)
	if(B3D_PLATFORM_${B3D_PLATFORM}_DEPENDENCIES_FOLDER)
		set(${resultVar} ${B3D_PLATFORM_${B3D_PLATFORM}_DEPENDENCIES_FOLDER}/${packageName})
	else()
		set(${resultVar} ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/${packageName})
	endif()
ENDMACRO()

# Marks the end of CMake Find* script.
#
# @param	packageName			Name of the package that's being located.
# @param	mainLibraryName		Name of the primary library in the package, without an extension.
MACRO(B3DEndFindPackage packageName mainLibraryName)
	if(NOT ${packageName}_FOUND)
		if(${packageName}_FIND_REQUIRED)
			message(FATAL_ERROR "Cannot find ${packageName} installation. Try modifying the ${packageName}_INSTALL_DIR path.")
		elseif(NOT ${packageName}_FIND_QUIETLY)
			message(WARNING "Cannot find ${packageName} installation. Try modifying the ${packageName}_INSTALL_DIR path.")
		endif()
	else()
		set_target_properties(${packageName}::${mainLibraryName} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${${packageName}_INCLUDE_DIR}")
		message(STATUS "...${packageName} OK.")
	endif()

	mark_as_advanced(${packageName}_INSTALL_DIR ${packageName}_INCLUDE_DIR)
	set(${packageName}_INCLUDE_DIRS ${${packageName}_INCLUDE_DIR})
ENDMACRO()

# Sets up default paths in which to look for library includes and binaries. These paths will be used internally
# by B3DFindImported* family of methods, so you should call this before calling these methods. User can also manually
# add paths to the variables defined by this command.
#
# @param	packageName		Name of the package being located.
#
# Must be defined before calling:
#  - ${packageName}_INSTALL_DIR - Path to the location of package, relative to which the search paths will be set up.
#
# Will define:
#  - ${packageName}_INCLUDE_SEARCH_DIRS
#  - ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS
#  - ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS
#  - ${packageName}_BINARY_RELEASE_SEARCH_DIRS
#  - ${packageName}_BINARY_DEBUG_SEARCH_DIRS
MACRO(B3DPopulateDefaultPackageSearchPaths packageName)
	set(${packageName}_INCLUDE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/include")

	if (B3D_IS_64BIT)
		set(platform "x64")
	else ()
		set(platform "x86")
	endif ()

	# Both platform and configuration specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}/Release")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}/Debug")

	# Only platform specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}")

	# Only configuration specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/Release")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/Debug")

	# Neither platform and configuration specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib")

	# Both platform and configuration specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}/Release")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}/Debug")

	# Only platform specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}")

	# Only configuration specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/Release")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/Debug")

	# Neither platform and configuration specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin")
ENDMACRO()

# Creates a new library of IMPORTED type and sets up the external paths to library binaries.
#
# @param 		libraryName				Name of the library target that will be defined.
# @param		libraryType				Type of the library: STATIC, SHARED or MODULE.
# @param		releaseLibraryPath		Path to the library (.lib, .so, .dylib) to be used in Release configurations.
# @param		debugLibraryPath		Path to the library (.lib, .so, .dylib) to be used in Debug configurations.
# @param		releaseBinaryPath		Path to the binary (.dll) to be used in Release configurations. Only relevant on Windows.
# @param		debugBinaryPath			Path to the binary (.dll) to be used in Debug configurations. Only relevant on Windows.
#
# Will define:
# - A library named ${libraryName}
MACRO(B3DAddImportedLibrary libraryName libraryType releaseLibraryPath debugLibraryPath releaseBinaryPath debugBinaryPath)

	if(${libraryType} STREQUAL "STATIC")
		add_library(${libraryName} STATIC IMPORTED)

		set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseLibraryPath}")
		set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugLibraryPath}")
	elseif(${libraryType} STREQUAL "SHARED")
		add_library(${libraryName} SHARED IMPORTED)

		if(WIN32)
			set_target_properties(${libraryName} PROPERTIES IMPORTED_IMPLIB "${releaseLibraryPath}") # .lib
			set_target_properties(${libraryName} PROPERTIES IMPORTED_IMPLIB_DEBUG "${debugLibraryPath}") # .lib
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseBinaryPath}") # .dll
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugBinaryPath}") # .dll
		else()
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseLibraryPath}")
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugLibraryPath}")
		endif()
	elseif(${libraryType} STREQUAL "MODULE")

		if(WIN32)
			add_library(${libraryName} INTERFACE IMPORTED)

			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseBinaryPath}") # .dll
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugBinaryPath}") # .dll
		else()
			add_library(${libraryName} MODULE IMPORTED)

			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseLibraryPath}")
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugLibraryPath}")
		endif()
	else()
		message(ERROR "Invalid library type: ${libraryType}")
	endif()
ENDMACRO()

# Attempts to locate a library using the paths set up by B3DPopulateDefaultPackageSearchPaths().
#
# @param	packageName				Name of the package that the library is a part of.
# @param 	libraryName				Name of the library target that will be defined.
# @param	libraryType				Type of the library: STATIC, SHARED or MODULE.
# @param	releaseLibraryName		Name of the library (.lib, .so, .dylib) to be used for Release configurations (without an extension).
# @param	debugLibraryName		Name of the library (.lib, .so, .dylib) to be used for Debug configurations (without an extension).
# @param	releaseBinaryName		Name of the binary (.dll) to be used for Release configurations (without an extension). Only relevant on Windows.
# @param	debugBinaryName			Name of the binary (.dll) to be used for Debug configurations (without an extension). Only relevant on Windows.
#
# Will define:
# - A library named ${packageName}::${libraryName}
# - Library will be appended to list ${packageName}_LIBRARIES
MACRO(B3DFindImportedLibraryWithExplicitNames packageName libraryName libraryType releaseLibraryName debugLibraryName releaseBinaryName debugBinaryName)

	if(NOT ${libraryType} STREQUAL "MODULE" OR NOT WIN32)
		find_library(${libraryName}_LIBRARY_RELEASE NAMES ${releaseLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
		find_library(${libraryName}_LIBRARY_RELEASE NAMES ${releaseLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS})

		if(${packageName}_LIBRARY_DEBUG_SEARCH_DIRS)
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_DEBUG_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_DEBUG_SEARCH_DIRS})
		else()
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS})
		endif()

		if(NOT ${libraryName}_LIBRARY_RELEASE OR NOT ${libraryName}_LIBRARY_DEBUG)
			set(${packageName}_FOUND FALSE)
			message(STATUS "...Cannot find imported library: ${libraryName}")
		endif()
	endif()

	if(NOT ${libraryType} STREQUAL "STATIC" AND WIN32)
		find_file(${libraryName}_BINARY_RELEASE NAMES ${releaseBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
		find_file(${libraryName}_BINARY_RELEASE NAMES ${releaseBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS})

		if(${packageName}_BINARY_DEBUG_SEARCH_DIRS)
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_DEBUG_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_DEBUG_SEARCH_DIRS})
		else()
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS})
		endif()

		if(NOT ${libraryName}_BINARY_RELEASE OR NOT ${libraryName}_BINARY_DEBUG)
			set(${packageName}_FOUND FALSE)
			message(STATUS "...Cannot find imported library .dll: ${libraryName}")
		endif()
	endif()

	if(NOT ${packageName}_FOUND OR ${${packageName}_FOUND})
		B3DAddImportedLibrary(${packageName}::${libraryName} ${libraryType} "${${libraryName}_LIBRARY_RELEASE}" "${${libraryName}_LIBRARY_DEBUG}" "${${libraryName}_BINARY_RELEASE}" "${${libraryName}_BINARY_DEBUG}")
	endif()

	list(APPEND ${packageName}_LIBRARIES ${packageName}::${libraryName})
	mark_as_advanced(${libraryName}_LIBRARY_RELEASE ${libraryName}_LIBRARY_DEBUG)
	mark_as_advanced(${libraryName}_BINARY_RELEASE ${libraryName}_BINARY_DEBUG)
ENDMACRO()

# Equivalent to B3DFindImportedLibraryWithExplicitNames(), but uses @p libraryName for all library file names as a convenience.
MACRO(B3DFindImportedLibrary packageName libraryName libraryType)
	B3DFindImportedLibraryWithExplicitNames(${packageName} ${libraryName} ${libraryType} ${libraryName} ${libraryName} ${libraryName} ${libraryName})
ENDMACRO()

# Equivalent to B3DFindImportedLibrary(), but allows you to provide separate file names for Release and Debug configurations.
MACRO(B3DFindImportedLibraryWithConfigurationNames packageName libraryName libraryType releaseLibraryFileName debugLibraryFileName)
	B3DFindImportedLibraryWithExplicitNames(${packageName} ${libraryName} ${libraryType} ${releaseLibraryFileName} ${debugLibraryFileName} ${releaseLibraryFileName} ${debugLibraryFileName})
ENDMACRO()

# Equivalent to B3DFindImportedLibrary(), but allows you to provide an alternate filename for .dll files. Relevant on Windows only.
MACRO(B3DFindImportedLibraryWithAlternateBinaryName packageName libraryName libraryType binaryFileName)
	B3DFindImportedLibraryWithExplicitNames(${packageName} ${libraryName} ${libraryType} ${libraryName} ${libraryName} ${binaryFileName} ${binaryFileName})
ENDMACRO()

# Looks for include files for an imported library. Uses paths set up by a previous call to B3DPopulateDefaultPackageSearchPaths()
#
# @param	packageName		Name of the package that the includes are a part of.
# @param	includePath		Relative path to an include file to use as a reference for finding the includes.
MACRO(B3DFindImportedIncludes packageName includePath)
	find_path(${packageName}_INCLUDE_DIR NAMES ${includePath} PATHS ${${packageName}_INCLUDE_SEARCH_DIRS} NO_DEFAULT_PATH)
	find_path(${packageName}_INCLUDE_DIR NAMES ${includePath} PATHS ${${packageName}_INCLUDE_SEARCH_DIRS})

	if(${packageName}_INCLUDE_DIR)
		set(${packageName}_FOUND TRUE)
	else()
		message(STATUS "...Cannot find include file \"${includePath}\" at path ${${packageName}_INCLUDE_SEARCH_DIRS}")
		set(${packageName}_FOUND FALSE)
	endif()
ENDMACRO()