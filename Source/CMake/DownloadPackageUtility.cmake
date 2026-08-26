#######################################################################################
######################## Pre-built dependency download ################################
#######################################################################################

set(B3D_PREBUILT_DEPENDENCIES_URL "https://dependencies.banshee3d.io" CACHE STRING "The location that binary packages (prebuilt dependencies, built-in assets) will be pulled from.")
mark_as_advanced(B3D_PREBUILT_DEPENDENCIES_URL)

# Downloads and extracts a package if the version is out of date.
# Compares .reqversion and .version files in the target folder.
# If the package contains a DataPackageRemovals.txt at its root, the paths it lists (relative to @p targetFolder)
# are deleted from the target folder before the new contents are copied over.
#
# @param	targetFolder		Folder to extract contents into (e.g. Dependencies/XShaderCompiler)
# @param	archivePrefix		Prefix for the archive name (version will be appended, e.g. XShaderCompiler_Win32)
# @param	extractedFolderName	Name of the folder inside the archive (e.g. XShaderCompiler)
function(B3DDownloadPackageIfNeeded targetFolder archivePrefix extractedFolderName)
	set(versionFile ${targetFolder}/.version)
	set(reqVersionFile ${targetFolder}/.reqversion)

	# Check if .reqversion file exists
	if(NOT EXISTS ${reqVersionFile})
		message(WARNING "No .reqversion file found in '${targetFolder}'. Skipping update check.")
		return()
	endif()

	# Read required version
	file(STRINGS ${reqVersionFile} requiredVersion)

	# Check if package needs to be downloaded
	set(needsDownload FALSE)
	if(NOT EXISTS ${versionFile})
		message(STATUS "Package '${archivePrefix}' is missing. Downloading version ${requiredVersion}...")
		set(needsDownload TRUE)
	else()
		file(STRINGS ${versionFile} currentVersion)
		if(${requiredVersion} GREATER ${currentVersion})
			message(STATUS "Package '${archivePrefix}' is out of date (have v${currentVersion}, need v${requiredVersion}). Downloading...")
			set(needsDownload TRUE)
		endif()
	endif()

	if(NOT needsDownload)
		return()
	endif()

	set(tempFolder ${B3D_FRAMEWORK_ROOT_FOLDER}/Temp)
	set(archiveName ${archivePrefix}_${requiredVersion}.tar.gz)
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
		message(FATAL_ERROR "Package failed to download from URL: ${packageURL}")
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
endfunction()

#######################################################################################
######################## Dependency functions #########################################
#######################################################################################

# Checks if a dependency is out of date and if so, downloads it.
# Version is read from .reqversion file in the dependency folder.
#
# The prebuilt-archive suffix is the active platform (B3D_PLATFORM, e.g. Win32/Linux/MacOS),
# resolved during platform discovery in Prerequisites.cmake.
#
# @param	dependencyName		Name of the dependency (e.g. 'XShaderCompiler', 'Mono', etc.)
# @param	usePlatformFolder	(optional) If TRUE, the dependency lives in the active platform's Dependencies folder
#								(Framework/Platform/<B3D_PLATFORM>/Dependencies), if FALSE the dependency lives in
#								the framework's global Dependencies folder.
function(B3DCheckAndUpdatePrebuiltDependency dependencyName)
	set(usePlatformFolder FALSE)
	if(${ARGC} GREATER 1 AND ARGV1)
		set(usePlatformFolder TRUE)
	endif()

	if(usePlatformFolder)
		if(NOT B3D_PLATFORM_${B3D_PLATFORM}_DEPENDENCIES_FOLDER)
			message(FATAL_ERROR "Platform '${B3D_PLATFORM}' has no Dependencies folder; cannot update '${dependencyName}'.")
		endif()
		set(dependencyFolder ${B3D_PLATFORM_${B3D_PLATFORM}_DEPENDENCIES_FOLDER}/${dependencyName})
	else()
		set(dependencyFolder ${B3D_FRAMEWORK_ROOT_FOLDER}/Dependencies/${dependencyName})
	endif()

	set(archivePrefix ${dependencyName}_${B3D_PLATFORM})

	B3DDownloadPackageIfNeeded(${dependencyFolder} ${archivePrefix} ${dependencyName})
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
