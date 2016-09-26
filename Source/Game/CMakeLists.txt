# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(Game_INC 
	"Include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include")

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

	add_custom_command(TARGET Game POST_BUILD 
		COMMAND xcopy /Y /I \"$(TargetDir)$(TargetName).exe\" \"$(SolutionDir)..\\..\\Data\\Binaries\\${BS_COPY_FOLDER}\\\"
)
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
target_compile_options(Game PUBLIC /wd4509)