# Find tool packages
if(B3D_BUILD_BSL_PARSER)
	find_package(bison REQUIRED)
	find_package(flex REQUIRED)

	message(STATUS "Building BSL lexer/parser...")

	set(BS_FLEX_OPTIONS -L)
	set(BS_BISON_OPTIONS -l)

	execute_process(
			COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsLexerFX.l
			COMMAND ${bison_EXECUTABLE} ${BS_BISON_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsParserFX.y
			WORKING_DIRECTORY ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL
			RESULT_VARIABLE rv
	)
	
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsLexerFX.h" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsLexerFX.h")
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsLexerFX.c" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsLexerFX.c")
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsParserFX.h" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsParserFX.h")
	file(RENAME "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsParserFX.c" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsParserFX.c")
endif()

# Ensure pre-built binary dependencies are up to date
if(B3D_USE_BUNDLED_LIBRARIES)
	set(BS_XSC_PREBUILT_DEPENDENCY_VERSION 7) # Tick this when you change the binaries
	check_and_update_binary_deps(bsfDep_XShaderCompiler XShaderCompiler ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/XShaderCompiler ${BS_XSC_PREBUILT_DEPENDENCY_VERSION})
endif()

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_SL_SRC)

# Find package
find_package(XShaderCompiler)

# Target
add_library(bsfSL SHARED ${BS_SL_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfSL)

# Includes
target_include_directories(bsfSL PRIVATE "./")

# Pre-build step
if(B3D_BUILD_BSL_PARSER AND WIN32)
	add_custom_command(TARGET bsfSL PRE_BUILD
		COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsLexerFX.l
		COMMAND ${bison_EXECUTABLE} ${BS_BISON_OPTIONS} ${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL/BsParserFX.y
		COMMAND xcopy /Y /I BsLexerFX.h \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I BsLexerFX.c \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I BsParserFX.h \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I BsParserFX.c \"${B3D_FRAMEWORK_SOURCE_FOLDER}/Plugins/bsfSL\"
		COMMAND del BsLexerFX.h BsLexerFX.c BsParserFX.h BsParserFX.c
		COMMENT "Building BSL lexer/parser...")
endif()

# Libraries
## External lib: XShaderCompiler
target_link_libraries(bsfSL PRIVATE ${XShaderCompiler_LIBRARIES})

## Local libs
target_link_libraries(bsfSL PRIVATE bsf)

# IDE specific
set_property(TARGET bsfSL PROPERTY FOLDER Plugins)

find_clang_invalid_libc_pch_headers(inttypes_loc)

# Install
B3DSetUpPostBuildAndInstallSteps(bsfSL)
set_directory_properties(PROPERTIES
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../;${inttypes_loc}")

conditional_cotire(bsfSL)
