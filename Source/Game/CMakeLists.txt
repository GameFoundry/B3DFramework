# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(Game_INC 
	"./"
	"../BansheeUtility" 
	"../BansheeCore"
	"../BansheeEngine")

include_directories(${Game_INC})	
	
# Target
if(WIN32)
	add_executable(Game WIN32 ${BS_GAME_SRC})
else()
	add_executable(Game ${BS_GAME_SRC})
endif()

# Post-build step
if(WIN32)
	if(BS_64BIT)
		set(BS_COPY_FOLDER Win64)
	else()
		set(BS_COPY_FOLDER Win32)
	endif()

	if(MSVC)
		add_custom_command(
			TARGET Game POST_BUILD
			COMMAND xcopy /Y /I \"$(TargetDir)$(TargetName).exe\" \"$(SolutionDir)..\\..\\Data\\Binaries\\${BS_COPY_FOLDER}\\\")
	else()
		set(SRC_FILE ${BS_BINARY_OUTPUT_DIR}/${CMAKE_BUILD_TYPE}/Game)
		set(DST_FOLDER ${PROJECT_SOURCE_DIR}/../Data/Binaries/${BS_COPY_FOLDER})
		add_custom_command(TARGET Game POST_BUILD COMMAND xcopy /Y /I ${SRC_FILE} ${DST_FOLDER})
	endif()

elseif(LINUX)
	if(BS_64BIT)
		set(BS_COPY_FOLDER Linux64)
	else()
		set(BS_COPY_FOLDER Linux32)
	endif()

	set(SRC_FILE ${BS_BINARY_OUTPUT_DIR}/${CMAKE_BUILD_TYPE}/Game)
	set(DST_FOLDER ${PROJECT_SOURCE_DIR}/../Data/Binaries/${BS_COPY_FOLDER})
	add_custom_command(
		TARGET Game POST_BUILD
		COMMAND rm -rf ${DST_FOLDER}
		COMMAND mkdir -p ${DST_FOLDER}
		COMMAND cp ${SRC_FILE} ${DST_FOLDER})
else()
# TODO_OTHER_PLATFORMS_GO_HERE
endif()

# Libraries
## Local libs
target_link_libraries(Game BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET Game PROPERTY FOLDER Executable)

# Plugin dependencies
add_engine_dependencies(Game)

# Compiler flags
if(MSVC)
	target_compile_options(Game PUBLIC /wd4509)
endif()