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

# IDE specific
set_property(TARGET bsfScript PROPERTY FOLDER Script)
