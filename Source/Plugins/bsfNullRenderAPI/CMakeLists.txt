# Source files and their filters
include(CMakeSources.cmake)
	
# Target
add_library(bsfNullRenderAPI SHARED ${BS_NULLRENDERAPI_SRC})

# Common flags
add_common_flags(bsfNullRenderAPI)

# Includes
target_include_directories(bsfNullRenderAPI PRIVATE "./")
	
# Libraries
## Local libs
target_link_libraries(bsfNullRenderAPI PRIVATE bsf)

# IDE specific
set_property(TARGET bsfNullRenderAPI PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDER_BACKEND MATCHES "Null")
	install_bsf_target(bsfNullRenderAPI)
endif()

conditional_cotire(bsfNullRenderAPI)
