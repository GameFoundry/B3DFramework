# Outputs:
#   BS_GENERATED_ENGINE_H_FILES
#   BS_GENERATED_ENGINE_CPP_FILES
#   BS_GENERATED_EDITOR_H_FILES
#   BS_GENERATED_EDITOR_CPP_FILES
#   BS_GENERATED_ENGINE_CS_FILES
#   BS_GENERATED_EDITOR_CS_FILES

set(BS_SCRIPT_PARSER_INCLUDE_DIRS "")
set(BS_SCRIPT_PARSER_H_FILES "")

function(prepend var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
endfunction()

function(addForGeneration name)
	set(BS_SCRIPT_PARSER_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS} "${name}/Include" PARENT_SCOPE)
	
	include(${name}/CMakeSources.cmake)
	string(TOUPPER ${name} LIBNAME)

	set(H_FILES "")
	FOREACH(f ${BS_${LIBNAME}_SRC})
		IF("${f}" MATCHES ".*\\.h")
			LIST(APPEND H_FILES ${f})
		ENDIF()
	ENDFOREACH(f)
	
	set(ABS_H_FILES "")
	prepend(ABS_H_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${name} ${H_FILES})
	set(BS_SCRIPT_PARSER_H_FILES ${BS_SCRIPT_PARSER_H_FILES} ${ABS_H_FILES} PARENT_SCOPE)	
endfunction()

set(BS_GENERATED_CPP_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Generated)
set(BS_GENERATED_CS_ENGINE_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/MBansheeEngine/Generated)
set(BS_GENERATED_CS_EDITOR_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/MBansheeEditor/Generated)

if(GENERATE_SCRIPT_BINDINGS)
	addForGeneration(BansheeUtility)
	addForGeneration(BansheeCore)
	addForGeneration(BansheeEngine)
	addForGeneration(BansheeEditor)
	addForGeneration(SBansheeEngine)
	addForGeneration(SBansheeEditor)

	set(BS_SCRIPT_PARSER_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS} "BansheeMono/Include")

	list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_INCLUDE_DIRS)
	list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_H_FILES)

	prepend(BS_INCLUDE_DIRS "-I${PROJECT_SOURCE_DIR}" ${BS_SCRIPT_PARSER_INCLUDE_DIRS})

	# Generate a single .cpp file including all headers
	set(BS_GLOBAL_FILE_CONTENTS "")
	FOREACH(f ${BS_SCRIPT_PARSER_H_FILES})
		LIST(APPEND BS_GLOBAL_FILE_CONTENTS "#include \"${f}\"\n")
	ENDFOREACH(f)

	file(WRITE ${BS_GENERATED_CPP_OUTPUT_DIR}/toParse.cpp ${BS_GLOBAL_FILE_CONTENTS})
endif()

find_package(BansheeSBGen)
if(BansheeSBGen_FOUND)
	if(GENERATE_SCRIPT_BINDINGS)
		set(BS_GSB_COMMAND ${BansheeSBGen_EXECUTABLE_PATH}
			${BS_GENERATED_CPP_OUTPUT_DIR}/toParse.cpp
			-output-cpp ${BS_GENERATED_CPP_OUTPUT_DIR}
			-output-cs-engine ${BS_GENERATED_CS_ENGINE_OUTPUT_DIR}
			-output-cs-editor ${BS_GENERATED_CS_EDITOR_OUTPUT_DIR}
			-- ${BS_INCLUDE_DIRS}
			-DBS_STATIC_LIB
			-DBS_SBGEN
			-w)

		message(STATUS "Generating script bindings, please wait...")
		execute_process(
			COMMAND ${BS_GSB_COMMAND}
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			RESULT_VARIABLE SBGEN_RETURN_VALUE
		)

		if (NOT SBGEN_RETURN_VALUE EQUAL 0)
			message(FATAL_ERROR "Failed to generate script bindings.")
		else()
			message(STATUS "...scripting binding generation OK.")
		endif()
	endif()
		
	file(GLOB BS_GENERATED_ENGINE_H_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/Engine/Include/*)
	file(GLOB BS_GENERATED_ENGINE_CPP_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/Engine/Source/*)

	file(GLOB BS_GENERATED_EDITOR_H_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/Editor/Include/*)
	file(GLOB BS_GENERATED_EDITOR_CPP_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/Editor/Source/*)
endif()