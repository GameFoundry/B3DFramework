if(WIN32)
	execute_process(
		COMMAND ${PROJECT_SOURCE_DIR}/../Dependencies/tools/flex/flex.exe --wincompat ${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.l 
		COMMAND ${PROJECT_SOURCE_DIR}/../Dependencies/tools/bison/bison.exe ${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.y
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/BansheeSL
		RESULT_VARIABLE rv
	 )
	 
	file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.h" "${PROJECT_SOURCE_DIR}/BansheeSL/Include/BsLexerFX.h")
	file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.c" "${PROJECT_SOURCE_DIR}/BansheeSL/Source/BsLexerFX.c")
	file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.h" "${PROJECT_SOURCE_DIR}/BansheeSL/Include/BsParserFX.h")
	file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.c" "${PROJECT_SOURCE_DIR}/BansheeSL/Source/BsParserFX.c")
else()
	find_package(BISON REQUIRED)
	find_package(FLEX REQUIRED)
	message("Building lexer/parser...")
	execute_process(
		COMMAND ${FLEX_EXECUTABLE} ${BS_FLEX_OPTIONS} ${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.l
		COMMAND ${BISON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.y)
	execute_process(
		COMMAND mv BsLexerFX.h ${PROJECT_SOURCE_DIR}/BansheeSL/Include
		COMMAND mv BsLexerFX.c ${PROJECT_SOURCE_DIR}/BansheeSL/Source
		COMMAND mv BsParserFX.h ${PROJECT_SOURCE_DIR}/BansheeSL/Include
		COMMAND mv BsParserFX.c ${PROJECT_SOURCE_DIR}/BansheeSL/Source)
endif()

set(DEPENDENCY_BUILD_DIR "${CMAKE_SOURCE_DIR}/../Dependencies/Build/${DEPENDENCY_NAME}")
set(DEPENDENCY_PARAMS -DXSC_BUILD_SHELL=OFF -DXSC_ENABLE_LANGUAGE_EXT=ON)

find_package_or_build(XShaderCompiler inc/Xsc/Xsc.h "${DEPENDENCY_PARAMS}")

# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeSL_INC
	"Include"
	"../BansheeUtility/Include"
	"../BansheeCore/Include"
	"../BansheeEngine/Include"
	${XShaderCompiler_INCLUDE_DIR})

include_directories(${BansheeSL_INC})

# Target
add_library(BansheeSL SHARED ${BS_BANSHEESL_SRC})

# Defines
target_compile_definitions(BansheeSL PRIVATE -DBS_SL_EXPORTS)

# Pre-build step
if(WIN32)
	add_custom_command(TARGET BansheeSL PRE_BUILD
		COMMAND ${PROJECT_SOURCE_DIR}/../Dependencies/tools/flex/flex.exe --wincompat ${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.l
		COMMAND ${PROJECT_SOURCE_DIR}/../Dependencies/tools/bison/bison.exe ${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.y
		COMMAND xcopy /Y /I BsLexerFX.h \"${PROJECT_SOURCE_DIR}/BansheeSL/Include\"
		COMMAND xcopy /Y /I BsLexerFX.c \"${PROJECT_SOURCE_DIR}/BansheeSL/Source\"
		COMMAND xcopy /Y /I BsParserFX.h \"${PROJECT_SOURCE_DIR}/BansheeSL/Include\"
		COMMAND xcopy /Y /I BsParserFX.c \"${PROJECT_SOURCE_DIR}/BansheeSL/Source\"
		COMMAND del BsLexerFX.h BsLexerFX.c BsParserFX.h BsParserFX.c
		COMMENT "Building lexer/parser...")
endif()

# Libraries
## External lib: XShaderCompiler
target_link_libraries(BansheeSL PRIVATE ${XShaderCompiler_LIBRARIES})

## Local libs
target_link_libraries(BansheeSL PRIVATE BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeSL PROPERTY FOLDER Plugins)
