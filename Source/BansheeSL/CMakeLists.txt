# Find tool packages
find_package(bison REQUIRED)
find_package(flex REQUIRED)

message(STATUS "Building BSL lexer/parser...")
if(WIN32)
	set(BS_FLEX_OPTIONS --wincompat)
endif()

execute_process(
		COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.l
		COMMAND ${bison_EXECUTABLE} ${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.y
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/BansheeSL
		RESULT_VARIABLE rv
)

file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.h" "${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.h")
file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.c" "${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.c")
file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.h" "${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.h")
file(RENAME "${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.c" "${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.c")

# Source files and their filters
include(CMakeSources.cmake)

# Find package
find_package(XShaderCompiler)

# Includes
set(BansheeSL_INC
	"./"
	"../BansheeUtility"
	"../BansheeCore"
	"../BansheeEngine")

include_directories(${BansheeSL_INC})

# Target
add_library(BansheeSL SHARED ${BS_BANSHEESL_SRC})

# Defines
target_compile_definitions(BansheeSL PRIVATE -DBS_SL_EXPORTS)

# Pre-build step
if(WIN32)
	add_custom_command(TARGET BansheeSL PRE_BUILD
		COMMAND ${flex_EXECUTABLE} ${BS_FLEX_OPTIONS} ${PROJECT_SOURCE_DIR}/BansheeSL/BsLexerFX.l
		COMMAND ${bison_EXECUTABLE} ${PROJECT_SOURCE_DIR}/BansheeSL/BsParserFX.y
		COMMAND xcopy /Y /I BsLexerFX.h \"${PROJECT_SOURCE_DIR}/BansheeSL\"
		COMMAND xcopy /Y /I BsLexerFX.c \"${PROJECT_SOURCE_DIR}/BansheeSL\"
		COMMAND xcopy /Y /I BsParserFX.h \"${PROJECT_SOURCE_DIR}/BansheeSL\"
		COMMAND xcopy /Y /I BsParserFX.c \"${PROJECT_SOURCE_DIR}/BansheeSL\"
		COMMAND del BsLexerFX.h BsLexerFX.c BsParserFX.h BsParserFX.c
		COMMENT "Building BSL lexer/parser...")
endif()

# Libraries
## External lib: XShaderCompiler
target_link_libraries(BansheeSL PRIVATE ${XShaderCompiler_LIBRARIES})

## Local libs
target_link_libraries(BansheeSL PRIVATE BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeSL PROPERTY FOLDER Plugins)

# Install
install(
	TARGETS BansheeSL
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)