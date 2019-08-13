# Find tool packages
if(BUILD_BSL)
	find_package(bison REQUIRED)
	find_package(flex REQUIRED)

	message(STATUS "Building BSL lexer/parser...")

	set(BS_FLEX_OPTIONS -L)
	set(BS_BISON_OPTIONS -l)

	execute_process(
			COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${BSF_SOURCE_DIR}/Plugins/bsfSL/BsLexerFX.l
			COMMAND ${bison_EXECUTABLE} ${BS_BISON_OPTIONS} ${BSF_SOURCE_DIR}/Plugins/bsfSL/BsParserFX.y
			WORKING_DIRECTORY ${BSF_SOURCE_DIR}/Plugins/bsfSL
			RESULT_VARIABLE rv
	)
	
	file(RENAME "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsLexerFX.h" "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsLexerFX.h")
	file(RENAME "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsLexerFX.c" "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsLexerFX.c")
	file(RENAME "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsParserFX.h" "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsParserFX.h")
	file(RENAME "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsParserFX.c" "${BSF_SOURCE_DIR}/Plugins/bsfSL/BsParserFX.c")
endif()

# Ensure pre-built binary dependencies are up to date
if(USE_BUNDLED_LIBRARIES)
	set(BS_XSC_PREBUILT_DEPENDENCY_VERSION 6) # Tick this when you change the binaries
	check_and_update_binary_deps(bsfDep_XShaderCompiler XShaderCompiler ${BSF_SOURCE_DIR}/../Dependencies/XShaderCompiler ${BS_XSC_PREBUILT_DEPENDENCY_VERSION})
endif()

# Source files and their filters
include(CMakeSources.cmake)

# Find package
find_package(XShaderCompiler)

# Target
add_library(bsfSL SHARED ${BS_SL_SRC})

# Common flags
add_common_flags(bsfSL)

# Includes
target_include_directories(bsfSL PRIVATE "./")

# Defines
target_compile_definitions(bsfSL PRIVATE -DBS_SL_EXPORTS)

# Pre-build step
if(BUILD_BSL AND WIN32)
	add_custom_command(TARGET bsfSL PRE_BUILD
		COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${BSF_SOURCE_DIR}/Plugins/bsfSL/BsLexerFX.l
		COMMAND ${bison_EXECUTABLE} ${BS_BISON_OPTIONS} ${BSF_SOURCE_DIR}/Plugins/bsfSL/BsParserFX.y
		COMMAND xcopy /Y /I BsLexerFX.h \"${BSF_SOURCE_DIR}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I BsLexerFX.c \"${BSF_SOURCE_DIR}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I BsParserFX.h \"${BSF_SOURCE_DIR}/Plugins/bsfSL\"
		COMMAND xcopy /Y /I BsParserFX.c \"${BSF_SOURCE_DIR}/Plugins/bsfSL\"
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
install_bsf_target(bsfSL)
set_directory_properties(PROPERTIES
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../;${inttypes_loc}")

conditional_cotire(bsfSL)
