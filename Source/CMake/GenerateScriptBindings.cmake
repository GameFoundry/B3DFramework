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

function(addForGeneration path prefix name)
	set(BS_SCRIPT_PARSER_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS} "${path}" PARENT_SCOPE)
	
	include(${path}/CMakeSources.cmake)
	string(TOUPPER ${name} LIBNAME)

	set(H_FILES "")
	FOREACH(f ${BS_${LIBNAME}_SRC})
		IF("${f}" MATCHES ".*\\.h" AND NOT "${f}" MATCHES "Win32|Linux|MacOS")
			LIST(APPEND H_FILES ${f})
		ENDIF()
	ENDFOREACH(f)
	
	set(ABS_H_FILES "")
	prepend(ABS_H_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${prefix} ${H_FILES})
	set(BS_SCRIPT_PARSER_H_FILES ${BS_SCRIPT_PARSER_H_FILES} ${ABS_H_FILES} PARENT_SCOPE)	
endfunction()

set(BS_GENERATED_CPP_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/Source/Scripting)
set(BS_GENERATED_CS_ENGINE_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/Source/Scripting/MBansheeEngine/Generated)
set(BS_GENERATED_CS_EDITOR_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/Source/Scripting/MBansheeEditor/Generated)

if(GENERATE_SCRIPT_BINDINGS)
	find_package(BansheeSBGen)

	addForGeneration(Source/bsf/Source/Foundation/bsfUtility Source/bsf/Source/Foundation/ UTILITY)
	addForGeneration(Source/bsf/Source/Foundation/bsfCore Source/bsf/Source/Foundation/ CORE)
	addForGeneration(Source/bsf/Source/Foundation/bsfEngine Source/bsf/Source/Foundation/ ENGINE)
	addForGeneration(Source/EditorCore Source/EditorCore BANSHEEEDITOR)
	addForGeneration(Source/Scripting/SBansheeEngine Source/Scripting/SBansheeEngine SBANSHEEENGINE)
	addForGeneration(Source/Scripting/SBansheeEditor Source/Scripting/SBansheeEditor SBANSHEEEDITOR)

	set(BS_SCRIPT_PARSER_INCLUDE_DIRS 
		${BS_SCRIPT_PARSER_INCLUDE_DIRS} 
		"Source/bsf/Source/Plugins/bsfMono"
		"Source/bsf/Source/Foundation/bsfUtility/ThirdParty")

	list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_INCLUDE_DIRS)
	list(REMOVE_DUPLICATES BS_SCRIPT_PARSER_H_FILES)

	prepend(BS_INCLUDE_DIRS "-I${PROJECT_SOURCE_DIR}" ${BS_SCRIPT_PARSER_INCLUDE_DIRS})

	# Generate a single .cpp file including all headers
	set(BS_GLOBAL_FILE_CONTENTS "")
	FOREACH(f ${BS_SCRIPT_PARSER_H_FILES})
		LIST(APPEND BS_GLOBAL_FILE_CONTENTS "#include \"${f}\"\n")
	ENDFOREACH(f)

	file(WRITE ${PROJECT_BINARY_DIR}/toParse.cpp ${BS_GLOBAL_FILE_CONTENTS})
endif()

if(BansheeSBGen_FOUND)
	if(GENERATE_SCRIPT_BINDINGS)
		set(BS_GSB_COMMAND ${BansheeSBGen_EXECUTABLE_PATH}
			${PROJECT_BINARY_DIR}/toParse.cpp
			-output-cpp ${BS_GENERATED_CPP_OUTPUT_DIR}
			-output-cs-engine ${BS_GENERATED_CS_ENGINE_OUTPUT_DIR}
			-output-cs-editor ${BS_GENERATED_CS_EDITOR_OUTPUT_DIR}
			-- ${BS_INCLUDE_DIRS}
			-std=c++14
			-DBS_STATIC_LIB
			-DBS_SBGEN
			-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH
			-w)

		if(APPLE)
			list(APPEND BS_GSB_COMMAND
				-isystem /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1)
			list(APPEND BS_GSB_COMMAND
				-isystem /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include)
			list(APPEND BS_GSB_COMMAND -stdlib=libc++)
		endif()

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
endif()

file(GLOB BS_GENERATED_ENGINE_H_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/SBansheeEngine/Generated/*.h)
file(GLOB BS_GENERATED_ENGINE_CPP_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/SBansheeEngine/Generated/*.cpp)

file(GLOB BS_GENERATED_EDITOR_H_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/SBansheeEditor/Generated/*.h)
file(GLOB BS_GENERATED_EDITOR_CPP_FILES ${BS_GENERATED_CPP_OUTPUT_DIR}/SBansheeEditor/Generated/*.h)
