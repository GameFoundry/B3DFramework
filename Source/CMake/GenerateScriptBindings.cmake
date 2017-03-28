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

addForGeneration(BansheeUtility)
addForGeneration(BansheeCore)
addForGeneration(BansheeEngine)
addForGeneration(BansheeEditor)
addForGeneration(SBansheeEngine)
addForGeneration(SBansheeEditor)

set(BS_SCRIPT_PARSER_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS} "BansheeMono/Include")

list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_INCLUDE_DIRS)
list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_H_FILES)

# Overriding previous code and using a fixed set of files in order to speed up parsing
#set(BS_SCRIPT_PARSER_SOURCE_FILES "")
#list(APPEND BS_SCRIPT_PARSER_SOURCE_FILES "BansheeUtility/Include/PixelVolume.h");

set(BS_GENERATED_FILES_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Generated)
prepend(BS_INCLUDE_DIRS "-I${PROJECT_SOURCE_DIR}" ${BS_SCRIPT_PARSER_INCLUDE_DIRS})

# Generate a single .cpp file including all headers
set(BS_GLOBAL_FILE_CONTENTS "")
FOREACH(f ${BS_SCRIPT_PARSER_H_FILES})
	LIST(APPEND BS_GLOBAL_FILE_CONTENTS "#include \"${f}\"\n")
ENDFOREACH(f)

file(WRITE ${BS_GENERATED_FILES_OUTPUT_DIR}/toParse.cpp ${BS_GLOBAL_FILE_CONTENTS})

#execute_process(
#    COMMAND ${PROJECT_SOURCE_DIR}/../Dependencies/tools/SBGen 
#		${BS_GENERATED_FILES_OUTPUT_DIR}/toParse.cpp
#		-output ${BS_GENERATED_FILES_OUTPUT_DIR}
#		-- ${BS_INCLUDE_DIRS}
#		-DBS_STATIC_LIB
#	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#    RESULT_VARIABLE SBGEN_RETURN_VALUE
#)

#if (NOT SBGEN_RETURN_VALUE EQUAL 0)
#    message(FATAL_ERROR "Failed to generate script bindings.")
#endif()

file(GLOB BS_GENERATED_ENGINE_H_FILES ${BS_GENERATED_FILES_OUTPUT_DIR}/Cpp/Engine/Include/*)
file(GLOB BS_GENERATED_ENGINE_CPP_FILES ${BS_GENERATED_FILES_OUTPUT_DIR}/Cpp/Engine/Source/*)

file(GLOB BS_GENERATED_EDITOR_H_FILES ${BS_GENERATED_FILES_OUTPUT_DIR}/Cpp/Editor/Include/*)
file(GLOB BS_GENERATED_EDITOR_CPP_FILES ${BS_GENERATED_FILES_OUTPUT_DIR}/Cpp/Editor/Source/*)

file(GLOB BS_GENERATED_ENGINE_CS_FILES ${BS_GENERATED_FILES_OUTPUT_DIR}/Cs/Engine/*)
file(GLOB BS_GENERATED_EDITOR_CS_FILES ${BS_GENERATED_FILES_OUTPUT_DIR}/Cs/Editor/*)