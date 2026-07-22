set(B3D_DOCGEN_HEADER_FOLDERS "")
set(B3D_DOCGEN_HEADER_FILES "")

# Registers a folder path in which the doc-gen pass of the code generator tool will look for include files
# to process.
#
# @param	path		Path to a folder that contains the include files.
function(B3DRegisterIncludeFolderForDocGen path)
	set(B3D_DOCGEN_HEADER_FOLDERS ${B3D_DOCGEN_HEADER_FOLDERS} "-I${path}" PARENT_SCOPE)

	file(GLOB_RECURSE allHeaderFiles ${path}/*.h)

	set(docGenHeaderFiles "")
	foreach(headerFile ${allHeaderFiles})
		if("${headerFile}" MATCHES ".*\\.h" AND NOT "${headerFile}" MATCHES "Private|ThirdParty|Generated|RTTI")
			list(APPEND docGenHeaderFiles ${headerFile})
		endif()
	endforeach(headerFile)

	set(B3D_DOCGEN_HEADER_FILES ${B3D_DOCGEN_HEADER_FILES} ${docGenHeaderFiles} PARENT_SCOPE)
endfunction()

# Registers a custom target that builds the full documentation site. The target first ensures
# BansheeCodeGenerator is built, then runs it in documentation JSON mode to produce a single
# docgen.json file at ${PROJECT_BINARY_DIR}/Generated/docgen.json describing every class, struct,
# enum, method, field and free function in the framework source tree, and finally invokes the
# BansheeDocGenerator Python tool to render the HTML site.
function(B3DRegisterDocGenTarget)
	if (NOT TARGET BansheeCodeGenerator)
		return()
	endif()

	B3DRegisterIncludeFolderForDocGen(${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility)
	B3DRegisterIncludeFolderForDocGen(${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Core)
	B3DRegisterIncludeFolderForDocGen(${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfScript)

	if(B3D_IS_ENGINE)
		B3DRegisterIncludeFolderForDocGen(${PROJECT_SOURCE_DIR}/Source/EditorCore)
		B3DRegisterIncludeFolderForDocGen(${PROJECT_SOURCE_DIR}/Source/EditorScript)
	endif()

	set(B3D_DOCGEN_HEADER_FOLDERS
		${B3D_DOCGEN_HEADER_FOLDERS}
		"-I${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfMono"
		"-I${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility/ThirdParty"
		"-I${PROJECT_BINARY_DIR}/Generated/Utility/")

	# Platform overlay include folders, needed to resolve Prerequisites/{Platform}/... includes. Added
	# as include paths only, since platform-internal headers are not documented.
	B3DGetActivePlatformFolders(activePlatformFolders)
	foreach(platformFolder ${activePlatformFolders})
		set(platformEngineFolder ${B3D_PLATFORM_${platformFolder}_SOURCE_FOLDER}/Engine)

		if(EXISTS ${platformEngineFolder}/Utility)
			list(APPEND B3D_DOCGEN_HEADER_FOLDERS "-I${platformEngineFolder}/Utility")
		endif()

		if(EXISTS ${platformEngineFolder}/Core)
			list(APPEND B3D_DOCGEN_HEADER_FOLDERS "-I${platformEngineFolder}/Core")
		endif()
	endforeach()

	set(B3D_DOCGEN_HEADER_FILES
		"${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility/B3DUtilityPrerequisites.h"
		${B3D_DOCGEN_HEADER_FILES})

	list(REMOVE_DUPLICATES B3D_DOCGEN_HEADER_FOLDERS)
	list(REMOVE_DUPLICATES B3D_DOCGEN_HEADER_FILES)

	string(REPLACE ";" " " docGenHeaderFoldersArgument "${B3D_DOCGEN_HEADER_FOLDERS}")

	# Generate a single .cpp file including all headers, used as the single translation unit for the doc-gen pass.
	set(docGenParseTargetFileContents "")
	foreach(path ${B3D_DOCGEN_HEADER_FILES})
		list(APPEND docGenParseTargetFileContents "#include \"${path}\"\n")
	endforeach(path)

	file(WRITE ${PROJECT_BINARY_DIR}/B3DDocGenParseTarget.cpp ${docGenParseTargetFileContents})

	set(GenDocGen_SOURCE_FILE ${PROJECT_BINARY_DIR}/B3DDocGenParseTarget.cpp)
	set(GenDocGen_OUTPUT_FILE ${PROJECT_BINARY_DIR}/Generated/docgen.json)
	set(GenDocGen_INCLUDE_DIRS ${docGenHeaderFoldersArgument})
	set(GenDocGen_WORKING_DIR ${PROJECT_SOURCE_DIR})

	set(runDocGenCommandArguments
			"${GenDocGen_SOURCE_FILE} \
			-docgen-json \
			-docgen-json-output=${GenDocGen_OUTPUT_FILE} \
			-- ${GenDocGen_INCLUDE_DIRS} \
			-std=c++17 \
			-DB3D_STATIC_LIB \
			-DB3D_CODEGEN \
			-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH \
			-w")

	# Locate the Python interpreter used to render the HTML site. We only need the interpreter,
	# not the dev libraries, and we do the lookup here (at configure time) so the path is baked
	# into the generated script.
	find_package(Python3 COMPONENTS Interpreter QUIET)
	if(Python3_FOUND)
		set(B3D_DOCGEN_PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
	else()
		set(B3D_DOCGEN_PYTHON_EXECUTABLE "")
	endif()

	set(B3D_DOCGEN_PY_WORKING_DIR ${B3D_FRAMEWORK_ROOT_FOLDER}/Tools/BansheeDocGenerator)

	configure_file(
		${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Scripts/GenerateDocumentation.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/GenerateDocumentation.cmake
		@ONLY)

	add_custom_target(GenerateDocumentation COMMAND ${CMAKE_COMMAND} -P
		${CMAKE_CURRENT_BINARY_DIR}/GenerateDocumentation.cmake -- "$<TARGET_FILE:BansheeCodeGenerator>")

	# Ensure the C++ generator is compiled before the custom target invokes it.
	add_dependencies(GenerateDocumentation BansheeCodeGenerator)

	set_property(TARGET GenerateDocumentation PROPERTY FOLDER Scripts)
endfunction()

B3DRegisterDocGenTarget()
