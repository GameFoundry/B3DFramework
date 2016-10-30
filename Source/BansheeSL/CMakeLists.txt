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

# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeSL_INC 
	"Include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include")

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
else()
# TODO_OTHER_PLATFORMS_GO_HERE
endif()

# Libraries
## Local libs
target_link_libraries(BansheeSL BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeSL PROPERTY FOLDER Plugins)