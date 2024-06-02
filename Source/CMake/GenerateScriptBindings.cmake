set(B3D_CODEGEN_HEADER_FOLDERS "")
set(B3D_CODEGEN_HEADER_FILES "")

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

function(B3DRegisterCodeGenTarget)
	if (NOT TARGET BansheeCodeGenerator)
		return()
	endif()

	B3DRegisterIncludeFolderForCodeGen(${BSF_SOURCE_DIR}/Foundation/bsfUtility)
	B3DRegisterIncludeFolderForCodeGen(${BSF_SOURCE_DIR}/Foundation/bsfCore)
	B3DRegisterIncludeFolderForCodeGen(${BSF_SOURCE_DIR}/Foundation/bsfEngine)
	B3DRegisterIncludeFolderForCodeGen(${BSF_SOURCE_DIR}/Scripting/bsfScript)

	if(B3D_IS_ENGINE)
		B3DRegisterIncludeFolderForCodeGen(${PROJECT_SOURCE_DIR}/Source/EditorCore)
		B3DRegisterIncludeFolderForCodeGen(${PROJECT_SOURCE_DIR}/Source/EditorScript)
	endif()

	set(B3D_CODEGEN_HEADER_FOLDERS
		${B3D_CODEGEN_HEADER_FOLDERS}
		"-I${BSF_SOURCE_DIR}/Scripting/bsfMono"
		"-I${BSF_SOURCE_DIR}/Foundation/bsfUtility/ThirdParty"
		"-I${PROJECT_BINARY_DIR}/Generated/bsfUtility/")

	set(B3D_CODEGEN_HEADER_FILES
		"${BSF_SOURCE_DIR}/Foundation/bsfUtility/Prerequisites/BsPrerequisitesUtil.h"
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
	set(GenScriptBinding_OUTPUT_CPP_DIR ${BSF_SOURCE_DIR}/Scripting/bsfScript/Generated)
	set(GenScriptBinding_OUTPUT_CS_DIR ${BSF_SOURCE_DIR}/Scripting/bsfSharp/Generated)
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
			-DBS_STATIC_LIB \
			-DBS_SBGEN \
			-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH \
			-w")

	set(runCodegenCommand "${GenScriptBinding_SBGEN_EXECUTABLE} ${runCodegenCommandArguments}")

	configure_file(
		${BSF_SOURCE_DIR}/CMake/Scripts/GenerateScriptBindings.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/GenerateScriptBindings.cmake
		@ONLY)

	add_custom_target(GenerateScriptBindings COMMAND ${CMAKE_COMMAND} -P
		${CMAKE_CURRENT_BINARY_DIR}/GenerateScriptBindings.cmake -- "$<TARGET_FILE:BansheeCodeGenerator>")

	set_property(TARGET GenerateScriptBindings PROPERTY FOLDER Scripts)
	set_target_properties(BansheeCodeGenerator PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${runCodegenCommandArguments}")
	set_target_properties(BansheeCodeGenerator PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${GenScriptBinding_WORKING_DIR}")
endfunction()

B3DRegisterCodeGenTarget()
