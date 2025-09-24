# Runtime executable
if(NOT B3D_IS_ENGINE)
	# Target for running framework scripting assemblies
	add_executable(bsfSharp WIN32 B3DScriptExec.cpp BsScriptWin32Resource.rc)
	target_link_libraries(bsfSharp bsfScript bsfMono bsf)
	
	set_property(TARGET bsfSharp PROPERTY FOLDER Script)

	# Copy .dlls on build
	if(B3D_USE_BUNDLED_LIBRARIES)
		B3DCopyFolderOnBuild(bsfSharp ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/mono ${PROJECT_BINARY_DIR} bin/Mono "*.*")
	endif()
endif()
