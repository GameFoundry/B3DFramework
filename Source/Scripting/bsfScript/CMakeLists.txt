# Source files and their filters
include(CMakeSources.cmake)

file(GLOB BS_GENERATED_SCRIPT_H_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Generated/*.h)
file(GLOB BS_GENERATED_SCRIPT_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Generated/*.cpp)

source_group("Generated" FILES ${BS_GENERATED_SCRIPT_H_FILES} ${BS_GENERATED_SCRIPT_CPP_FILES})

# Target
add_library(bsfScript OBJECT 
	${BS_SCRIPT_SRC}
	${BS_GENERATED_SCRIPT_H_FILES} 
	${BS_GENERATED_SCRIPT_CPP_FILES})

# Common flags
add_common_flags(bsfScript)
	
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
if(NOT BS_IS_BANSHEE3D)
	# Target for running framework scripting assemblies
	add_executable(bsfSharp WIN32 BsScriptExec.cpp)
	target_link_libraries(bsfSharp bsfScript bsfMono bsf)
	
	set_property(TARGET bsfSharp PROPERTY FOLDER Script)

	# Copy .dlls on build
	if(USE_BUNDLED_LIBRARIES)
		install_dll_on_build(bsfSharp ${BSF_SOURCE_DIR}/../Dependencies/mono)
		copy_folder_on_build(bsfSharp ${BSF_SOURCE_DIR}/../Dependencies/mono ${PROJECT_BINARY_DIR} bin/Mono "*.*")
	endif()
endif()

# IDE specific
set_property(TARGET bsfScript PROPERTY FOLDER Script)