# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_SCRIPT_SRC)

# Target
add_library(bsfScript OBJECT ${BS_SCRIPT_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfScript)
	
# Includes
set(bsfScript_INC
	"./"
	"./Generated/")

target_include_directories(bsfScript PUBLIC ${bsfScript_INC})
	
# Defines
target_compile_definitions(bsfScript PUBLIC -DBS_SCR_BE_STATIC_LIB)

# Libraries
## Local libs
target_link_libraries(bsfScript bsfMono bsf)

# Runtime executable
if(NOT B3D_IS_ENGINE)
	# Target for running framework scripting assemblies
	add_executable(bsfSharp WIN32 BsScriptExec.cpp BsScriptWin32Resource.rc)
	target_link_libraries(bsfSharp bsfScript bsfMono bsf)
	
	set_property(TARGET bsfSharp PROPERTY FOLDER Script)

	# Copy .dlls on build
	if(B3D_USE_BUNDLED_LIBRARIES)
		copy_folder_on_build(bsfSharp ${BSF_SOURCE_DIR}/../Dependencies/mono ${PROJECT_BINARY_DIR} bin/Mono "*.*")
	endif()
endif()

# IDE specific
set_property(TARGET bsfScript PROPERTY FOLDER Script)
