function(build_dependency DEPENDENCY_NAME BUILD_CONFIG BUILD_OPTIONS)
	set(DEPENDENCY_BUILD_DIR "${CMAKE_SOURCE_DIR}/../Dependencies/Build/${DEPENDENCY_NAME}/${BUILD_CONFIG}")
	
	# Make the build folder
	execute_process(COMMAND "${CMAKE_COMMAND}" 
						-E make_directory ${DEPENDENCY_BUILD_DIR}
					WORKING_DIRECTORY "${DEPENDENCY_BUILD_DIR}")	
	
	# Make build files
	execute_process(COMMAND "${CMAKE_COMMAND}" 
						-G "${CMAKE_GENERATOR}"
						${BUILD_OPTIONS}
						"${CMAKE_SOURCE_DIR}/External/${DEPENDENCY_NAME}"
						WORKING_DIRECTORY "${DEPENDENCY_BUILD_DIR}")
					
	# Execute the build and install
	execute_process(COMMAND "${CMAKE_COMMAND}" 
		--build "${DEPENDENCY_BUILD_DIR}"
		--config ${BUILD_CONFIG})		

	execute_process(COMMAND "${CMAKE_COMMAND}" 
		--build "${DEPENDENCY_BUILD_DIR}"
		--config ${BUILD_CONFIG}
		--target Install)
endfunction()

function(find_package_or_build DEPENDENCY_NAME DEPENDENCY_INCLUDE_PATH BUILD_OPTIONS)
	set(DEPENDENCIES_BUILD_DIR "${CMAKE_SOURCE_DIR}/../Dependencies/Build")
	set(DEPENDENCY_BUILD_DIR "${DEPENDENCIES_BUILD_DIR}/${DEPENDENCY_NAME}")
	set(DEPENDENCY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/External/${DEPENDENCY_NAME}")

	# Look for dependency binaries
	find_package(${DEPENDENCY_NAME} QUIET)

	# Cannot find binaries, see if we can compile them
	if(NOT ${DEPENDENCY_NAME}_FOUND)
		message(STATUS "...${DEPENDENCY_NAME} binaries cannot be found, building from source and retrying.")

		# See if we have the source code for the dependency, and if not fetch them from git
		find_path(SUBMODULE_SOURCES ${DEPENDENCY_INCLUDE_PATH} ${DEPENDENCY_SOURCE_DIR})
		if(NOT SUBMODULE_SOURCES)
			execute_process(COMMAND git submodule update 
								--init 
								-- External/${DEPENDENCY_NAME}
							WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
		endif()

		# Build
		build_dependency(${DEPENDENCY_NAME} Release "${BUILD_OPTIONS}")
		build_dependency(${DEPENDENCY_NAME} Debug "${BUILD_OPTIONS}")
		
		# Update the dependencies version
		file(WRITE ${DEPENDENCIES_BUILD_DIR}/.version ${BS_SRC_DEPENDENCIES_VERSION})
		
		# Now try finding the package again, this time it's required
		find_package(${DEPENDENCY_NAME} REQUIRED)
		
		mark_as_advanced(SUBMODULE_SOURCES)
	endif()
endfunction()