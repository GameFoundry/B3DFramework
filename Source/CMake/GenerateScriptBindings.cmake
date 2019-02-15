set(BS_SCRIPT_PARSER_INCLUDE_DIRS "")
set(BS_SCRIPT_PARSER_H_FILES "")

function(addForGeneration path)
	set(BS_SCRIPT_PARSER_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS} "-I${path}" PARENT_SCOPE)
	
	file(GLOB_RECURSE ALL_H_FILES ${path}/*.h)
	
	set(PUBLIC_H_FILES "")
	foreach(f ${ALL_H_FILES})
		if("${f}" MATCHES ".*\\.h" AND NOT "${f}" MATCHES "Private|ThirdParty|Generated")
			list(APPEND PUBLIC_H_FILES ${f})
		endif()
	endforeach(f)
	
	set(BS_SCRIPT_PARSER_H_FILES ${BS_SCRIPT_PARSER_H_FILES} ${PUBLIC_H_FILES} PARENT_SCOPE)	
endfunction()

function(add_generate_script_bindings_target)
	find_package(BansheeSBGen REQUIRED)

	if(BansheeSBGen_FOUND)
		addForGeneration(${BSF_SOURCE_DIR}/Foundation/bsfUtility)
		addForGeneration(${BSF_SOURCE_DIR}/Foundation/bsfCore)
		addForGeneration(${BSF_SOURCE_DIR}/Foundation/bsfEngine)
		addForGeneration(${BSF_SOURCE_DIR}/Plugins/bsfScript)
		
		if(BS_IS_BANSHEE3D)
			addForGeneration(${PROJECT_SOURCE_DIR}/Source/EditorCore)
			addForGeneration(${PROJECT_SOURCE_DIR}/Source/EditorScript)
		endif()

		set(BS_SCRIPT_PARSER_INCLUDE_DIRS 
			${BS_SCRIPT_PARSER_INCLUDE_DIRS} 
			"-I${BSF_SOURCE_DIR}/Plugins/bsfMono"
			"-I${BSF_SOURCE_DIR}/Foundation/bsfUtility/ThirdParty"
			"-I${PROJECT_BINARY_DIR}/Generated/bsfUtility/")

		set(BS_SCRIPT_PARSER_H_FILES 
			"${BSF_SOURCE_DIR}/Foundation/bsfUtility/Prerequisites/BsPrerequisitesUtil.h"
			${BS_SCRIPT_PARSER_H_FILES})
			
		list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_INCLUDE_DIRS)
		list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_H_FILES)

		# Generate a single .cpp file including all headers
		set(BS_GLOBAL_FILE_CONTENTS "")
		foreach(f ${BS_SCRIPT_PARSER_H_FILES})
			list(APPEND BS_GLOBAL_FILE_CONTENTS "#include \"${f}\"\n")
		endforeach(f)

		file(WRITE ${PROJECT_BINARY_DIR}/toParse.cpp ${BS_GLOBAL_FILE_CONTENTS})

		set(GenScriptBinding_SBGEN_EXECUTABLE ${BansheeSBGen_EXECUTABLE_PATH})
		set(GenScriptBinding_SOURCE_FILE ${PROJECT_BINARY_DIR}/toParse.cpp)
		set(GenScriptBinding_OUTPUT_CPP_DIR ${BSF_SOURCE_DIR}/Plugins/bsfScript/Generated)
		set(GenScriptBinding_OUTPUT_CS_DIR ${BSF_SOURCE_DIR}/Plugins/bsfSharp/Generated)
		set(GenScriptBinding_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS})
		set(GenScriptBinding_WORKING_DIR ${PROJECT_SOURCE_DIR})
		
		if(BS_IS_BANSHEE3D)
			set(GenScriptBinding_OUTPUT_CPP_EDITOR_DIR ${PROJECT_SOURCE_DIR}/Source/EditorScript/Generated)
			set(GenScriptBinding_OUTPUT_CS_EDITOR_DIR ${PROJECT_SOURCE_DIR}/Source/EditorManaged/Generated)
		else()
			set(GenScriptBinding_OUTPUT_CPP_EDITOR_DIR "\"\"")
			set(GenScriptBinding_OUTPUT_CS_EDITOR_DIR "\"\"")
		endif()

		configure_file(
			${BSF_SOURCE_DIR}/CMake/Scripts/GenerateScriptBindings.cmake.in
			${CMAKE_CURRENT_BINARY_DIR}/GenerateScriptBindings.cmake
			@ONLY)

		add_custom_target(GenerateScriptBindings COMMAND ${CMAKE_COMMAND} -P
			${CMAKE_CURRENT_BINARY_DIR}/GenerateScriptBindings.cmake)
			
		set_property(TARGET GenerateScriptBindings PROPERTY FOLDER Scripts)
	endif()
endfunction()

add_generate_script_bindings_target()