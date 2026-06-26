# Find tool packages
if(B3D_BUILD_BSL_PARSER)
	find_package(bison REQUIRED)
	find_package(flex REQUIRED)

	message(STATUS "Building BSL lexer/parser...")

	set(BS_FLEX_OPTIONS -L)
	set(BS_BISON_OPTIONS -l)

	execute_process(
			COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DLexerFX.l
			COMMAND ${bison_EXECUTABLE} ${BS_BISON_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DParserFX.y
			WORKING_DIRECTORY ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL
			RESULT_VARIABLE rv
	)
	
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DLexerFX.h" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DLexerFX.h")
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DLexerFX.c" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DLexerFX.c")
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DParserFX.h" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DParserFX.h")
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DParserFX.c" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DParserFX.c")
endif()

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_SL_SRC)

# Each entry is "<language>|<targetLanguage>|<define>" (the trailing "|<define>" is optional):
# the engine shading-language id, the B3DShaderCompiler output target id, and an optional preprocessor define emitted into the HLSL input.
# These are populated by Bytecode libraries when they are included in the build
get_property(B3D_SHADER_CROSS_COMPILE_TARGETS GLOBAL PROPERTY B3D_SHADER_CROSS_COMPILE_TARGETS)

# List of extra libraries required by shader backends.
get_property(B3D_SHADER_BACKEND_LIBRARIES GLOBAL PROPERTY B3D_SHADER_BACKEND_LIBRARIES)

# Generate .cpp for cross compile target registration
set(B3D_SHADER_CROSS_COMPILE_TARGET_REGISTRATIONS "")
foreach(targetEntry ${B3D_SHADER_CROSS_COMPILE_TARGETS})
	string(REPLACE "|" ";" targetFields ${targetEntry})
	list(GET targetFields 0 targetLanguageId)
	list(GET targetFields 1 targetOutputLanguage)
	list(LENGTH targetFields targetFieldCount)
	if(targetFieldCount GREATER 2)
		list(GET targetFields 2 targetDefine)
		string(APPEND B3D_SHADER_CROSS_COMPILE_TARGET_REGISTRATIONS
			"\t\tRegisterTarget(\"${targetLanguageId}\", { \"${targetOutputLanguage}\", \"${targetDefine}\" });\n")
	else()
		string(APPEND B3D_SHADER_CROSS_COMPILE_TARGET_REGISTRATIONS
			"\t\tRegisterTarget(\"${targetLanguageId}\", { \"${targetOutputLanguage}\", nullptr });\n")
	endif()
endforeach()

set(BS_SL_CROSSCOMPILE_TARGETS_GENERATED ${PROJECT_BINARY_DIR}/Generated/Plugins/bsfSL/B3DHLSLCrossCompileTargets.cpp)
configure_file(
	${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/B3DHLSLCrossCompileTargets.cpp.in
	${BS_SL_CROSSCOMPILE_TARGETS_GENERATED}
	@ONLY
)
list(APPEND BS_SL_SRC ${BS_SL_CROSSCOMPILE_TARGETS_GENERATED})
source_group("Generated" FILES ${BS_SL_CROSSCOMPILE_TARGETS_GENERATED})

# Ensure pre-built binary dependencies are up to date
if(B3D_USE_BUNDLED_LIBRARIES)
	B3DCheckAndUpdatePrebuiltDependency(XShaderCompiler)
endif()

# Find package
find_package(XShaderCompiler)

# Target
B3DAddPlugin(bsfSL IMPORTER ${BS_SL_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfSL)

# Includes
target_include_directories(bsfSL PRIVATE "./")

# Pre-build step
if(B3D_BUILD_BSL_PARSER AND WIN32)
	add_custom_command(TARGET bsfSL PRE_BUILD
		COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DLexerFX.l
		COMMAND ${bison_EXECUTABLE} ${BS_BISON_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/B3DParserFX.y
		COMMAND xcopy /Y /I B3DLexerFX.h \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I B3DLexerFX.c \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I B3DParserFX.h \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I B3DParserFX.c \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND del B3DLexerFX.h B3DLexerFX.c B3DParserFX.h B3DParserFX.c
		COMMENT "Building BSL lexer/parser...")
endif()

# Libraries
## External lib: XShaderCompiler, plus any optional per-platform shader backends found above.
target_link_libraries(bsfSL PRIVATE ${XShaderCompiler_LIBRARIES} ${B3D_SHADER_BACKEND_LIBRARIES})

# IDE specific
set_property(TARGET bsfSL PROPERTY FOLDER Plugins)

# Install
B3DSetUpPostBuildAndInstallSteps(bsfSL)
