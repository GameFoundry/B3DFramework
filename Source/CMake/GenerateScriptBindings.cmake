set(B3D_CODEGEN_HEADER_FOLDERS "")
set(B3D_CODEGEN_HEADER_FILES "")

# Registers a folder path in which the code generator tool will look for include files to process.
#
# @param	path		Path to a folder that contains the include files.
function(B3DRegisterIncludeFolderForCodeGen path)
	set(B3D_CODEGEN_HEADER_FOLDERS ${B3D_CODEGEN_HEADER_FOLDERS} "-I${path}" PARENT_SCOPE)

	file(GLOB_RECURSE allHeaderFiles ${path}/*.h)

	set(publicHeaderFiles "")
	foreach(headerFile ${allHeaderFiles})
		if("${headerFile}" MATCHES ".*\\.h" AND NOT "${headerFile}" MATCHES "Private|ThirdParty|Generated|RTTI")
			list(APPEND publicHeaderFiles ${headerFile})
		endif()
	endforeach(headerFile)

	set(B3D_CODEGEN_HEADER_FILES ${B3D_CODEGEN_HEADER_FILES} ${publicHeaderFiles} PARENT_SCOPE)
endfunction()

# Registers a target that runs the code generator tool.
function(B3DRegisterCodeGenTarget)
	if (NOT TARGET BansheeCodeGenerator)
		return()
	endif()

	B3DRegisterIncludeFolderForCodeGen(${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility)
	B3DRegisterIncludeFolderForCodeGen(${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Core)
	B3DRegisterIncludeFolderForCodeGen(${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfScript)

	if(B3D_IS_ENGINE)
		B3DRegisterIncludeFolderForCodeGen(${PROJECT_SOURCE_DIR}/Source/EditorCore)
		B3DRegisterIncludeFolderForCodeGen(${PROJECT_SOURCE_DIR}/Source/EditorScript)
	endif()

	set(B3D_CODEGEN_HEADER_FOLDERS
		${B3D_CODEGEN_HEADER_FOLDERS}
		"-I${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfMono"
		"-I${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility/ThirdParty"
		"-I${PROJECT_BINARY_DIR}/Generated/Utility/")

	# Platform overlay include folders, needed to resolve Prerequisites/{Platform}/... includes. Added
	# as include paths only, since platform-internal headers contain no script exports to parse.
	B3DGetActivePlatformFolders(activePlatformFolders)
	foreach(platformFolder ${activePlatformFolders})
		set(platformEngineFolder ${B3D_PLATFORM_${platformFolder}_SOURCE_FOLDER}/Engine)

		if(EXISTS ${platformEngineFolder}/Utility)
			list(APPEND B3D_CODEGEN_HEADER_FOLDERS "-I${platformEngineFolder}/Utility")
		endif()

		if(EXISTS ${platformEngineFolder}/Core)
			list(APPEND B3D_CODEGEN_HEADER_FOLDERS "-I${platformEngineFolder}/Core")
		endif()
	endforeach()

	set(B3D_CODEGEN_HEADER_FILES
		"${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine/Utility/B3DUtilityPrerequisites.h"
		${B3D_CODEGEN_HEADER_FILES})

	list(REMOVE_DUPLICATES B3D_CODEGEN_HEADER_FOLDERS)
	list(REMOVE_DUPLICATES B3D_CODEGEN_HEADER_FILES)

	string(REPLACE ";" " " headerFoldersArgument "${B3D_CODEGEN_HEADER_FOLDERS}")

	# Generate a single .cpp file including all headers
	set(parseTargetFileContents "")
	foreach(path ${B3D_CODEGEN_HEADER_FILES})
		list(APPEND parseTargetFileContents "#include \"${path}\"\n")
	endforeach(path)

	file(WRITE ${PROJECT_BINARY_DIR}/B3DCodeGenParseTarget.cpp ${parseTargetFileContents})

	set(GenScriptBinding_SOURCE_FILE ${PROJECT_BINARY_DIR}/B3DCodeGenParseTarget.cpp)
	set(GenScriptBinding_OUTPUT_CPP_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfScript/Generated)
	set(GenScriptBinding_OUTPUT_CS_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfSharp/Generated)
	set(GenScriptBinding_INCLUDE_DIRS ${headerFoldersArgument})
	set(GenScriptBinding_WORKING_DIR ${PROJECT_SOURCE_DIR})

	if(B3D_IS_ENGINE)
		set(GenScriptBinding_OUTPUT_CPP_EDITOR_DIR ${PROJECT_SOURCE_DIR}/Source/EditorScript/Generated)
		set(GenScriptBinding_OUTPUT_CS_EDITOR_DIR ${PROJECT_SOURCE_DIR}/Source/EditorManaged/Generated)
		set(GenScriptBinding_DEFINES -gen-editor)
	else()
		set(GenScriptBinding_OUTPUT_CPP_EDITOR_DIR "\"\"")
		set(GenScriptBinding_OUTPUT_CS_EDITOR_DIR "\"\"")
		set(GenScriptBinding_DEFINES "")
	endif()

	set(runCodegenCommandArguments
			"${GenScriptBinding_SOURCE_FILE} \
			-output-cpp \
			${GenScriptBinding_OUTPUT_CPP_DIR} \
			-output-cs \
			${GenScriptBinding_OUTPUT_CS_DIR} \
			-output-cpp-editor \
			${GenScriptBinding_OUTPUT_CPP_EDITOR_DIR} \
			-output-cs-editor \
			${GenScriptBinding_OUTPUT_CS_EDITOR_DIR} \
			${GenScriptBinding_DEFINES} \
			-- ${GenScriptBinding_INCLUDE_DIRS} \
			-std=c++17 \
			-DB3D_STATIC_LIB \
			-DB3D_CODEGEN \
			-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH \
			-w")

	set(runCodegenCommand "${GenScriptBinding_SBGEN_EXECUTABLE} ${runCodegenCommandArguments}")

	configure_file(
		${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Scripts/GenerateScriptBindings.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/GenerateScriptBindings.cmake
		@ONLY)

	add_custom_target(GenerateScriptBindings COMMAND ${CMAKE_COMMAND} -P
		${CMAKE_CURRENT_BINARY_DIR}/GenerateScriptBindings.cmake -- "$<TARGET_FILE:BansheeCodeGenerator>")

	set_property(TARGET GenerateScriptBindings PROPERTY FOLDER Scripts)
	set_target_properties(BansheeCodeGenerator PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${runCodegenCommandArguments}")
	set_target_properties(BansheeCodeGenerator PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${GenScriptBinding_WORKING_DIR}")
endfunction()

B3DRegisterCodeGenTarget()
