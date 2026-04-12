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
## External lib: XShaderCompiler
target_link_libraries(bsfSL PRIVATE ${XShaderCompiler_LIBRARIES})

## Local libs
target_link_libraries(bsfSL PRIVATE bsf)

# IDE specific
set_property(TARGET bsfSL PROPERTY FOLDER Plugins)

# Install
B3DSetUpPostBuildAndInstallSteps(bsfSL)
