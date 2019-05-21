# Source files and their filters
include(CMakeSources.cmake)

# Packages
if(RENDER_API_MODULE MATCHES "Vulkan" OR INCLUDE_ALL_IN_WORKFLOW OR BUILD_ALL_RENDER_API)
	find_package(Vulkan)
	find_package(glslang)

	if(APPLE)
		find_package(SPIRVCross)
	endif()
endif()
	
# Target
add_library(bsfVulkanRenderAPI SHARED ${BS_VULKANRENDERAPI_SRC})

# Common flags
add_common_flags(bsfVulkanRenderAPI)

# Includes
target_include_directories(bsfVulkanRenderAPI PRIVATE
	"./"
	"../Shared/"
)

# Defines
target_compile_definitions(bsfVulkanRenderAPI PRIVATE -DBS_VULKAN_EXPORTS)

# Libraries	
## External libs: Vulkan, glslang
target_link_libraries(bsfVulkanRenderAPI PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(bsfVulkanRenderAPI PRIVATE ${glslang_LIBRARIES})

if(APPLE) # MacOS
	target_link_libraries(bsfVulkanRenderAPI PRIVATE ${SPIRVCross_LIBRARIES})
endif()

## OS libs
if(APPLE) # MacOS
	target_link_framework(bsfVulkanRenderAPI Foundation)
	target_link_framework(bsfVulkanRenderAPI CoreGraphics)
	target_link_framework(bsfVulkanRenderAPI CoreVideo)
	target_link_framework(bsfVulkanRenderAPI IOKit)
	target_link_framework(bsfVulkanRenderAPI AppKit)
	target_link_framework(bsfVulkanRenderAPI QuartzCore)
	target_link_framework(bsfVulkanRenderAPI Metal)
endif()

## Local libs
target_link_libraries(bsfVulkanRenderAPI PRIVATE bsf)

# IDE specific
set_property(TARGET bsfVulkanRenderAPI PROPERTY FOLDER Plugins)

# Install
if(RENDER_API_MODULE MATCHES "Vulkan" OR BUILD_ALL_RENDER_API)
	install_bsf_target(bsfVulkanRenderAPI)
endif()

conditional_cotire(bsfVulkanRenderAPI)
