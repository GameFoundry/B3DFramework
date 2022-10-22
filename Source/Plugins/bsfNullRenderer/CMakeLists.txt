# Source files and their filters
include(CMakeSources.cmake)
	
# Target
add_library(bsfNullRenderer SHARED ${BS_NULLRENDERER_SRC})

# Common flags
add_common_flags(bsfNullRenderer)

# Includes
target_include_directories(bsfNullRenderer PRIVATE "./")

# Libraries
## Local libs
target_link_libraries(bsfNullRenderer bsf)

# IDE specific
set_property(TARGET bsfNullRenderer PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDERER MATCHES "Null")
	install_bsf_target(bsfNullRenderer)
endif()

conditional_cotire(bsfNullRenderer)
