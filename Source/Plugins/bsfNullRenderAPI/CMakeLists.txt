# Source files and their filters
include(CMakeSources.cmake)
	
# Target
add_library(bsfNullRenderAPI SHARED ${BS_NULLRENDERAPI_SRC})

# Common flags
add_common_flags(bsfNullRenderAPI)

# Includes
target_include_directories(bsfNullRenderAPI PRIVATE "./")
	
## Local libs
target_link_libraries(bsfNullRenderAPI bsf)

# IDE specific
set_property(TARGET bsfNullRenderAPI PROPERTY FOLDER Plugins)

# Install
if(RENDER_API_MODULE MATCHES "Null")
	install_bsf_target(bsfNullRenderAPI)
endif()

conditional_cotire(bsfNullRenderAPI)
