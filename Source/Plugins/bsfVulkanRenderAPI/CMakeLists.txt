# Ensure pre-built binary dependencies are up to date
if(APPLE) # MacOS
	if(USE_BUNDLED_LIBRARIES)
		set(BS_SPVC_PREBUILT_DEPENDENCY_VERSION 1) # Tick this when you change the binaries
		check_and_update_binary_deps(bsfDep_SPIRVCross SPIRVCross ${BSF_SOURCE_DIR}/../Dependencies/SPIRVCross ${BS_SPVC_PREBUILT_DEPENDENCY_VERSION})

		set(BS_MVK_PREBUILT_DEPENDENCY_VERSION 2) # Tick this when you change the binaries
		check_and_update_binary_deps(bsfDep_MoltenVK MoltenVK ${BSF_SOURCE_DIR}/../Dependencies/MoltenVK ${BS_MVK_PREBUILT_DEPENDENCY_VERSION})
	endif()
endif()

# Source files and their filters
include(CMakeSources.cmake)

# Packages
if(RENDER_API_MODULE MATCHES "Vulkan" OR INCLUDE_ALL_IN_WORKFLOW OR BUILD_ALL_RENDER_API)
	find_package(glslang)

	if(APPLE)
		find_package(MoltenVK)
		find_package(SPIRVCross)
	else()
		find_package(Vulkan)
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
## External libs: Vulkan, MoltenVK, glslang, SPIR-V Cross
if(APPLE) # MacOS
	target_link_libraries(bsfVulkanRenderAPI PRIVATE ${MoltenVK_LIBRARIES})
	target_link_libraries(bsfVulkanRenderAPI PRIVATE ${SPIRVCross_LIBRARIES})
else()
	target_link_libraries(bsfVulkanRenderAPI PRIVATE ${Vulkan_LIBRARIES})
endif()

target_link_libraries(bsfVulkanRenderAPI PRIVATE ${glslang_LIBRARIES})

## OS libs
if(APPLE) # MacOS
	target_link_framework(bsfVulkanRenderAPI Foundation)
	target_link_framework(bsfVulkanRenderAPI CoreGraphics)
	target_link_framework(bsfVulkanRenderAPI CoreVideo)
	target_link_framework(bsfVulkanRenderAPI IOKit)
	target_link_framework(bsfVulkanRenderAPI AppKit)
	target_link_framework(bsfVulkanRenderAPI QuartzCore)
	target_link_framework(bsfVulkanRenderAPI Metal)
	target_link_framework(bsfVulkanRenderAPI IOSurface)
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
