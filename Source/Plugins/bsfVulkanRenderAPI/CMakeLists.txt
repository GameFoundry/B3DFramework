# Source files and their filters
include(CMakeSources.cmake)

# Packages
if(RENDER_API_MODULE MATCHES "Vulkan" OR INCLUDE_ALL_IN_WORKFLOW)
	find_package(Vulkan)
	find_package(glslang)
endif()
	
# Target
add_library(bsfVulkanRenderAPI SHARED ${BS_VULKANRENDERAPI_SRC})

# Common flags
add_common_flags(bsfD3D11RenderAPI)

# Includes
target_include_directories(bsfVulkanRenderAPI PRIVATE "./")

# Defines
target_compile_definitions(bsfVulkanRenderAPI PRIVATE -DBS_VULKAN_EXPORTS)

# Libraries	
## External libs: Vulkan, glslang
target_link_libraries(bsfVulkanRenderAPI PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(bsfVulkanRenderAPI PRIVATE ${glslang_LIBRARIES})

## Local libs
target_link_libraries(bsfVulkanRenderAPI PRIVATE bsf)

# IDE specific
set_property(TARGET bsfVulkanRenderAPI PROPERTY FOLDER Plugins)

# Install
if(RENDER_API_MODULE MATCHES "Vulkan")
	install_bsf_target(bsfVulkanRenderAPI)
endif()

conditional_cotire(bsfVulkanRenderAPI)
