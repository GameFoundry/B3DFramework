#######################################################################################
######################## Pre-built dependency download ################################
#######################################################################################

set(B3D_PREBUILT_DEPENDENCIES_URL "https://dependencies.banshee3d.io" CACHE STRING "The location that binary packages (prebuilt dependencies, built-in assets) will be pulled from.")
mark_as_advanced(B3D_PREBUILT_DEPENDENCIES_URL)

# Reads the version stamps of a package folder and reports whether the package needs updating.
# Compares .reqversion (the version the source tree requires) and .version (the version present on disk).
#
# @param	targetFolder		Folder holding the package (e.g. Dependencies/XShaderCompiler)
# @param	packageName			Name used in status messages
# @param	outRequiredVersion	Receives the required version, or an empty string if the folder has no .reqversion
# @param	outNeedsUpdate		Receives TRUE if the package is missing or older than required, FALSE otherwise
function(B3DCheckPackageVersion targetFolder packageName outRequiredVersion outNeedsUpdate)
	set(versionFile ${targetFolder}/.version)
	set(reqVersionFile ${targetFolder}/.reqversion)

	set(${outRequiredVersion} "" PARENT_SCOPE)
	set(${outNeedsUpdate} FALSE PARENT_SCOPE)

	if(NOT EXISTS ${reqVersionFile})
		message(WARNING "No .reqversion file found in '${targetFolder}'. Skipping update check.")
		return()
	endif()

	file(STRINGS ${reqVersionFile} requiredVersion)
	set(${outRequiredVersion} ${requiredVersion} PARENT_SCOPE)

	if(NOT EXISTS ${versionFile})
		message(STATUS "Package '${packageName}' is missing (need v${requiredVersion}).")
		set(${outNeedsUpdate} TRUE PARENT_SCOPE)
		return()
	endif()

	file(STRINGS ${versionFile} currentVersion)
	if(${requiredVersion} GREATER ${currentVersion})
		message(STATUS "Package '${packageName}' is out of date (have v${currentVersion}, need v${requiredVersion}).")
		set(${outNeedsUpdate} TRUE PARENT_SCOPE)
	endif()
endfunction()

# Downloads a package archive and extracts it over the target folder. Does not check versions; see
# B3DCheckPackageVersion for that. A failed download leaves the target folder untouched.
# If the package contains a DataPackageRemovals.txt at its root, the paths it lists (relative to @p targetFolder)
# are deleted from the target folder before the new contents are copied over.
#
# @param	targetFolder		Folder to extract contents into (e.g. Dependencies/XShaderCompiler)
# @param	archivePrefix		Prefix for the archive name (version will be appended, e.g. XShaderCompiler_Win32)
# @param	extractedFolderName	Name of the folder inside the archive (e.g. XShaderCompiler)
# @param	version				Version of the package to download
# @param	outSucceeded		Receives TRUE if the package was downloaded and extracted, FALSE if the download failed
function(B3DDownloadPackage targetFolder archivePrefix extractedFolderName version outSucceeded)
	set(${outSucceeded} FALSE PARENT_SCOPE)

	set(tempFolder ${B3D_FRAMEWORK_ROOT_FOLDER}/Temp)
	set(archiveName ${archivePrefix}_${version}.tar.gz)
	set(packageURL ${B3D_PREBUILT_DEPENDENCIES_URL}/${archiveName})

	# Clean and create a temporary folder
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${tempFolder})
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${tempFolder})

	message(STATUS "Downloading ${archiveName}...")
	file(DOWNLOAD ${packageURL} ${tempFolder}/${archiveName}
			SHOW_PROGRESS
			STATUS DOWNLOAD_STATUS)

	list(GET DOWNLOAD_STATUS 0 statusCode)
	if(NOT statusCode EQUAL 0)
		list(GET DOWNLOAD_STATUS 1 statusMessage)
		message(STATUS "Package failed to download from URL: ${packageURL} (${statusMessage})")
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${tempFolder})
		return()
	endif()

	message(STATUS "Extracting ${archiveName}...")
	execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${tempFolder}/${archiveName}
			WORKING_DIRECTORY ${tempFolder}
	)

	# Remove items the package declares obsolete. A package lists paths (relative to @p targetFolder, one per line)
	# in a DataPackageRemovals.txt at its root; this cleans up leftovers of files that used to be part of the
	# package but no longer are (e.g. files that moved into version control), which the remove-and-replace step
	# below cannot reach because they are absent from the new package.
	set(removalsFile ${tempFolder}/${extractedFolderName}/DataPackageRemovals.txt)
	if(EXISTS ${removalsFile})
		file(STRINGS ${removalsFile} removalEntries)
		foreach(removalEntry ${removalEntries})
			set(removalTarget ${targetFolder}/${removalEntry})
			if(IS_DIRECTORY ${removalTarget})
				execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${removalTarget})
			elseif(EXISTS ${removalTarget})
				execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${removalTarget})
			endif()
		endforeach()
	endif()

	# Get list of items in the extracted package
	file(GLOB extractedContents "${tempFolder}/${extractedFolderName}/*")

	# Only remove items that exist in the new package (preserve .reqversion and other unrelated items)
	foreach(item ${extractedContents})
		get_filename_component(itemName ${item} NAME)
		set(targetItem ${targetFolder}/${itemName})
		if(EXISTS ${targetItem} AND NOT itemName STREQUAL ".reqversion")
			if(IS_DIRECTORY ${targetItem})
				execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${targetItem})
			else()
				execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${targetItem})
			endif()
		endif()
	endforeach()

	# Copy new contents
	foreach(item ${extractedContents})
		get_filename_component(itemName ${item} NAME)
		if(IS_DIRECTORY ${item})
			execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${item} ${targetFolder}/${itemName})
		else()
			execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${item} ${targetFolder}/${itemName})
		endif()
	endforeach()

	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${tempFolder})

	set(${outSucceeded} TRUE PARENT_SCOPE)
endfunction()

# Downloads and extracts a package if the version is out of date. Fails the configure if the download fails.
# Compares .reqversion and .version files in the target folder.
#
# @param	targetFolder		Folder to extract contents into (e.g. Dependencies/XShaderCompiler)
# @param	archivePrefix		Prefix for the archive name (version will be appended, e.g. XShaderCompiler_Win32)
# @param	extractedFolderName	Name of the folder inside the archive (e.g. XShaderCompiler)
function(B3DDownloadPackageIfNeeded targetFolder archivePrefix extractedFolderName)
	B3DCheckPackageVersion(${targetFolder} ${archivePrefix} requiredVersion needsUpdate)
	if(NOT needsUpdate)
		return()
	endif()

	B3DDownloadPackage(${targetFolder} ${archivePrefix} ${extractedFolderName} ${requiredVersion} downloaded)
	if(NOT downloaded)
		message(FATAL_ERROR "Failed to download package '${archivePrefix}' version ${requiredVersion}.")
	endif()
endfunction()

#######################################################################################
######################## Dependency build from source #################################
#######################################################################################

# Locates the shell that runs the dependency build scripts in Framework/Scripts. On Windows this must be the
# bash shipped with Git for Windows, because the scripts rely on its MSYS environment; the WSL launcher at
# System32/bash.exe would not work, so the system search path is deliberately skipped there.
# Set B3D_DEPENDENCY_BUILD_SHELL in the cache to override the choice.
#
# @param	outShell		Receives the path to the shell executable. Fails the configure if no shell is found.
function(B3DFindDependencyBuildShell outShell)
	if(WIN32)
		find_package(Git QUIET)

		set(searchFolders "")
		if(GIT_EXECUTABLE)
			get_filename_component(gitFolder ${GIT_EXECUTABLE} DIRECTORY)
			list(APPEND searchFolders ${gitFolder}/../bin ${gitFolder}/../../bin)
		endif()
		list(APPEND searchFolders "$ENV{ProgramFiles}/Git/bin" "$ENV{ProgramW6432}/Git/bin" "$ENV{LOCALAPPDATA}/Programs/Git/bin")

		find_program(B3D_DEPENDENCY_BUILD_SHELL NAMES bash PATHS ${searchFolders} NO_DEFAULT_PATH
			DOC "Shell used to run the dependency build scripts in Framework/Scripts (bash from Git for Windows).")
	else()
		find_program(B3D_DEPENDENCY_BUILD_SHELL NAMES bash
			DOC "Shell used to run the dependency build scripts in Framework/Scripts.")
	endif()
	mark_as_advanced(B3D_DEPENDENCY_BUILD_SHELL)

	if(NOT B3D_DEPENDENCY_BUILD_SHELL)
		message(FATAL_ERROR "Cannot find bash, which is needed to run the dependency build scripts. "
			"On Windows install Git for Windows, or point B3D_DEPENDENCY_BUILD_SHELL at a bash executable.")
	endif()

	set(${outShell} ${B3D_DEPENDENCY_BUILD_SHELL} PARENT_SCOPE)
endfunction()

# Builds a dependency from source by running its build script from Framework/Scripts, and stamps the dependency
# folder with the required version once the script succeeds. The script clones the upstream source, builds it and
# installs the result into the dependency folder, so this can take a long time. Fails the configure if the
# script fails.
#
# @param	dependencyName		Name of the dependency (e.g. 'XShaderCompiler'), used for messages
# @param	buildScript			File name of the build script in Framework/Scripts (e.g. 'B3DBuildShaderCompiler.sh')
# @param	dependencyFolder	Folder the script installs the dependency into
# @param	requiredVersion		Version written to the folder's .version file after a successful build
function(B3DBuildDependencyFromSource dependencyName buildScript dependencyFolder requiredVersion)
	set(scriptsFolder ${B3D_FRAMEWORK_ROOT_FOLDER}/Scripts)
	if(NOT EXISTS ${scriptsFolder}/${buildScript})
		message(FATAL_ERROR "Build script '${buildScript}' for dependency '${dependencyName}' not found in '${scriptsFolder}'.")
	endif()

	B3DFindDependencyBuildShell(shell)

	# The scripts invoke 'cmake' by name. Put the running CMake first on the search path so the script uses it even
	# when CMake is not on the user's PATH (e.g. an IDE-bundled CMake).
	get_filename_component(cmakeFolder ${CMAKE_COMMAND} DIRECTORY)
	if(WIN32)
		set(scriptPath "${cmakeFolder};$ENV{PATH}")
	else()
		set(scriptPath "${cmakeFolder}:$ENV{PATH}")
	endif()

	# The scripts build for the host unless told otherwise.
	set(scriptArgs "")
	if(NOT B3D_PLATFORM STREQUAL B3D_HOST_PLATFORM)
		list(APPEND scriptArgs --target ${B3D_PLATFORM})
	endif()

	message(STATUS "Building '${dependencyName}' from source with ${buildScript}. This can take a while...")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E env "PATH=${scriptPath}" ${shell} ./${buildScript} ${scriptArgs}
		WORKING_DIRECTORY ${scriptsFolder}
		RESULT_VARIABLE scriptResult
	)

	if(NOT scriptResult EQUAL 0)
		message(FATAL_ERROR "Build script '${buildScript}' for dependency '${dependencyName}' failed (exit code ${scriptResult}). See the output above.")
	endif()

	# The script stamps .version relative to whatever was on disk before. The build satisfies the required version,
	# so record that instead, otherwise the next configure would try to update the dependency again.
	file(WRITE ${dependencyFolder}/.version "${requiredVersion}")
	message(STATUS "Built '${dependencyName}' v${requiredVersion} from source.")
endfunction()

#######################################################################################
######################## Dependency functions #########################################
#######################################################################################

# Ensures a dependency folder holds the required version, downloading or building it if it does not. Version is read
# from the .reqversion file in the dependency folder, and compared against the .version file the package carries.
#
# With bundled libraries enabled, an out-of-date dependency is updated by downloading its prebuilt package. If no
# package is available for the required version, and the dependency has a build script, the dependency is built from
# source instead. With bundled libraries disabled (B3D_USE_BUNDLED_LIBRARIES=OFF) no download is attempted, and an
# out-of-date dependency is always built from source.
#
# The prebuilt-archive suffix is the active platform (B3D_PLATFORM, e.g. Win32/Linux/MacOS),
# resolved during platform discovery in Prerequisites.cmake.
#
# @param	dependencyFolder	Folder the dependency is installed in
# @param	dependencyName		Name of the dependency, which is also its folder name and prebuilt-archive prefix
# @param	buildScript			File name of the script in Framework/Scripts that builds the dependency from source
#								(e.g. 'B3DBuildShaderCompiler.sh'), or an empty string if it has no build script. A
#								dependency without a build script can only be downloaded.
function(B3DUpdateDependency dependencyFolder dependencyName buildScript)
	# Without a build script an unbundled build has no way to provide the dependency, so leave it to the user.
	if(NOT B3D_USE_BUNDLED_LIBRARIES AND NOT buildScript)
		return()
	endif()

	B3DCheckPackageVersion(${dependencyFolder} ${dependencyName} requiredVersion needsUpdate)
	if(NOT needsUpdate)
		return()
	endif()

	if(B3D_USE_BUNDLED_LIBRARIES)
		set(archivePrefix ${dependencyName}_${B3D_PLATFORM})
		B3DDownloadPackage(${dependencyFolder} ${archivePrefix} ${dependencyName} ${requiredVersion} downloaded)
		if(downloaded)
			return()
		endif()

		if(NOT buildScript)
			message(FATAL_ERROR "Failed to download prebuilt package '${archivePrefix}' version ${requiredVersion}, and dependency '${dependencyName}' cannot be built from source.")
		endif()

		message(STATUS "No prebuilt package available for '${dependencyName}' v${requiredVersion}, building from source instead.")
	else()
		message(STATUS "Bundled libraries are disabled, building '${dependencyName}' from source.")
	endif()

	B3DBuildDependencyFromSource(${dependencyName} ${buildScript} ${dependencyFolder} ${requiredVersion})
endfunction()

# Ensures the bundled copy of a package is present and up to date, see B3DUpdateDependency for how it is provided.
# Meant to be called from a Find module, after it resolved ${packageName}_INSTALL_DIR and ${packageName}_BUNDLED_INSTALL_DIR,
# so every consumer of the package gets the check without repeating it. Does nothing when the user pointed
# ${packageName}_INSTALL_DIR at a copy of their own, as that copy is theirs to manage.
#
# The bundled folder's name is the dependency name, which need not match the package name (e.g. package 'FLAC' lives in
# a 'libFLAC' folder).
#
# @param	packageName		Name of the package being located (the name passed to B3DStartFindPackage)
# @param	BUILD_SCRIPT	(optional) File name of the script in Framework/Scripts that builds the dependency
#							from source (e.g. 'B3DBuildShaderCompiler.sh'). Without it the dependency can only be
#							downloaded.
function(B3DEnsureBundledDependency packageName)
	cmake_parse_arguments(ARG "" "BUILD_SCRIPT" "" ${ARGN})
	if(ARG_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "B3DEnsureBundledDependency(${packageName}): unknown arguments '${ARG_UNPARSED_ARGUMENTS}'. Use BUILD_SCRIPT <script>.")
	endif()

	if(NOT ${packageName}_BUNDLED_INSTALL_DIR)
		message(FATAL_ERROR "B3DEnsureBundledDependency(${packageName}): ${packageName}_BUNDLED_INSTALL_DIR is not set. Resolve it before the call.")
	endif()

	# Bundled folders are spelled with a '..' segment in some Find modules, so compare normalized paths.
	set(bundledFolder ${${packageName}_BUNDLED_INSTALL_DIR})
	cmake_path(NORMAL_PATH bundledFolder)

	set(installFolder ${${packageName}_INSTALL_DIR})
	if(installFolder)
		cmake_path(NORMAL_PATH installFolder)
		if(NOT installFolder STREQUAL bundledFolder)
			return()
		endif()
	endif()

	get_filename_component(dependencyName ${bundledFolder} NAME)
	B3DUpdateDependency(${bundledFolder} ${dependencyName} "${ARG_BUILD_SCRIPT}")
endfunction()

# Ensures a dependency is present and up to date, see B3DUpdateDependency for how it is provided. Locates the
# dependency folder by name; prefer B3DEnsureBundledDependency for a dependency that has a Find module.
#
# @param	dependencyName		Name of the dependency (e.g. 'FontAwesome', 'LLVM', etc.)
# @param	USE_PLATFORM_FOLDER	(optional) If present, the dependency lives in the active platform's Dependencies folder
#								(Framework/Platform/<B3D_PLATFORM>/Dependencies), otherwise the dependency lives in
#								the framework's global Dependencies folder.
# @param	BUILD_SCRIPT		(optional) File name of the script in Framework/Scripts that builds the dependency
#								from source (e.g. 'B3DBuildShaderCompiler.sh'). Without it the dependency can only be
#								downloaded.
function(B3DCheckAndUpdatePrebuiltDependency dependencyName)
	cmake_parse_arguments(ARG "USE_PLATFORM_FOLDER" "BUILD_SCRIPT" "" ${ARGN})
	if(ARG_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "B3DCheckAndUpdatePrebuiltDependency(${dependencyName}): unknown arguments '${ARG_UNPARSED_ARGUMENTS}'. Use USE_PLATFORM_FOLDER and/or BUILD_SCRIPT <script>.")
	endif()

	if(ARG_USE_PLATFORM_FOLDER)
		if(NOT B3D_PLATFORM_${B3D_PLATFORM}_DEPENDENCIES_FOLDER)
			message(FATAL_ERROR "Platform '${B3D_PLATFORM}' has no Dependencies folder; cannot update '${dependencyName}'.")
		endif()
		set(dependencyFolder ${B3D_PLATFORM_${B3D_PLATFORM}_DEPENDENCIES_FOLDER}/${dependencyName})
	else()
		set(dependencyFolder ${B3D_DEPENDENCY_DIRECTORY}/${dependencyName})
	endif()

	B3DUpdateDependency(${dependencyFolder} ${dependencyName} "${ARG_BUILD_SCRIPT}")
endfunction()

#######################################################################################
######################## Asset functions ##############################################
#######################################################################################

# Checks if an asset package is out of date and if so, downloads it.
# Version is read from .reqversion file in the asset folder.
#
# @param	packageName		Name of the asset package. Supported values:
#							FrameworkData, FrameworkDataRaw, FrameworkDocumentation, ExampleData, EditorData, EditorDataRaw
function(B3DCheckAndUpdateAssetPackage packageName)
	if(packageName STREQUAL "FrameworkData")
		set(assetFolder ${B3D_FRAMEWORK_ROOT_FOLDER}/Data)
	elseif(packageName STREQUAL "FrameworkDataRaw")
		set(assetFolder ${B3D_FRAMEWORK_ROOT_FOLDER}/Data/Raw)
	elseif(packageName STREQUAL "FrameworkDocumentation")
		set(assetFolder ${B3D_FRAMEWORK_FOLDER}/Documentation)
	elseif(packageName STREQUAL "ExampleData")
		set(assetFolder ${B3D_FRAMEWORK_ROOT_FOLDER}/Examples/Data)
	elseif(packageName STREQUAL "EditorData" AND B3D_IS_ENGINE)
		set(assetFolder ${PROJECT_SOURCE_DIR}/Data)
	elseif(packageName STREQUAL "EditorDataRaw" AND B3D_IS_ENGINE)
		set(assetFolder ${PROJECT_SOURCE_DIR}/Data/Raw)
	else()
		message(FATAL_ERROR "Unknown asset package '${packageName}'. Supported: FrameworkData, FrameworkDataRaw, FrameworkDocumentation, ExampleData. EditorData and EditorDataRaw require B3D_IS_ENGINE.")
	endif()

	B3DDownloadPackageIfNeeded(${assetFolder} ${packageName} ${packageName})

	# Touch timestamp file to avoid triggering reimport
	if(EXISTS ${assetFolder}/Timestamp.asset)
		execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${assetFolder}/Timestamp.asset)
	endif()
endfunction()
