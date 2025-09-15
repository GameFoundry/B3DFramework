# Builds a dependency that supports CMake as its build system.
#
# @param		dependencyName		Name of the dependency to build, relative to the ./External folder.
# @param		buildConfig			Configuration to build (e.g. 'Debug', 'Release')
# @param		buildOptions		Additional options to pass to CMake when generating the build.
function(B3DBuildDependency dependencyName buildConfig buildOptions)
	set(dependencyBuildFolder "${CMAKE_SOURCE_DIR}/../Dependencies/Build/${dependencyName}/${BUILD_CONFIG}")

	# Make the build folder
	execute_process(COMMAND "${CMAKE_COMMAND}"
						-E make_directory ${dependencyBuildFolder}
					WORKING_DIRECTORY "${dependencyBuildFolder}")
	
	# Make build files
	execute_process(COMMAND "${CMAKE_COMMAND}"
						-G "${CMAKE_GENERATOR}"
						${buildOptions}
						"${CMAKE_SOURCE_DIR}/External/${dependencyName}"
						WORKING_DIRECTORY "${dependencyBuildFolder}")
					
	# Execute the build and install
	execute_process(COMMAND "${CMAKE_COMMAND}"
		--build "${dependencyBuildFolder}"
		--config ${BUILD_CONFIG})		

	execute_process(COMMAND "${CMAKE_COMMAND}"
		--build "${dependencyBuildFolder}"
		--config ${BUILD_CONFIG}
		--target Install)
endfunction()

# Attempts to find a dependency, and if it cannot find it fetches the dependency source as a submodule,
# and runs CMake build to compile the dependency in Release and Debug configurations.
#
# @param		dependencyName			Name of the dependency to find/build, relative to the ./External folder.
# @param		dependencyIncludePath	Location of the include folder for the dependencies, used for locating an existing dependency.
# @param		buildOptions			Additional options to pass to CMake when generating the build.
function(B3DFindOrBuildDependency dependencyName dependencyIncludePath buildOptions)
	set(dependenciesBuildFolder "${CMAKE_SOURCE_DIR}/../Dependencies/Build")
	set(dependencySourceFolder "${CMAKE_SOURCE_DIR}/External/${dependencyName}")

	# Look for dependency binaries
	find_package(${dependencyName} QUIET)

	# Cannot find binaries, see if we can compile them
	if(NOT ${dependencyName}_FOUND)
		message(STATUS "...${dependencyName} binaries cannot be found, building from source and retrying.")

		# See if we have the source code for the dependency, and if not fetch them from git
		find_path(SUBMODULE_SOURCES ${dependencyIncludePath} ${dependencySourceFolder})
		if(NOT SUBMODULE_SOURCES)
			execute_process(COMMAND git submodule update
								--init
								-- External/${dependencyName}
							WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
		else()
			execute_process(COMMAND git submodule update
								-- External/${dependencyName}
							WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
		endif()

		# Build
		B3DBuildDependency(${dependencyName} Release "${buildOptions}")
		B3DBuildDependency(${dependencyName} Debug "${buildOptions}")
		
		# Update the dependencies version
		file(WRITE ${dependenciesBuildFolder}/.version ${B3D_SOURCE_DEPENDENCIES_VERSION})
		
		# Now try finding the package again, this time it's required
		find_package(${dependencyName} REQUIRED)
		
		mark_as_advanced(SUBMODULE_SOURCES)
	endif()
endfunction()
