# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(RenderBeast_INC 
	"./"
	"../BansheeUtility" 
	"../BansheeCore"
	"../BansheeEngine")

include_directories(${RenderBeast_INC})	
	
# Target
add_library(RenderBeast SHARED ${BS_RENDERBEAST_SRC})

# Defines
target_compile_definitions(RenderBeast PRIVATE -DBS_BSRND_EXPORTS)

# Libraries
## Local libs
target_link_libraries(RenderBeast BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET RenderBeast PROPERTY FOLDER Plugins)

# Install
install(
	TARGETS RenderBeast
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)