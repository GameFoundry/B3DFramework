# Ensure pre-built binary dependencies are up to date
if(APPLE) # MacOS
	if(B3D_USE_BUNDLED_LIBRARIES)
		set(BS_SPVC_PREBUILT_DEPENDENCY_VERSION 1) # Tick this when you change the binaries
		check_and_update_binary_deps(bsfDep_SPIRVCross SPIRVCross ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/SPIRVCross ${BS_SPVC_PREBUILT_DEPENDENCY_VERSION})

		set(BS_MVK_PREBUILT_DEPENDENCY_VERSION 3) # Tick this when you change the binaries
		check_and_update_binary_deps(bsfDep_MoltenVK MoltenVK ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/MoltenVK ${BS_MVK_PREBUILT_DEPENDENCY_VERSION})
	endif()
endif()

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_VULKANRENDERAPI_SRC)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "../Shared/" "" BS_VULKANRENDERAPI_SRC)

# Packages
if(B3D_RENDER_BACKEND MATCHES "Vulkan" OR B3D_INCLUDE_ALL_IN_WORKFLOW OR B3D_BUILD_ALL_RENDER_BACKENDS)
	find_package(glslang)
	find_package(SPIRVCross)

	if(APPLE)
		find_package(MoltenVK)
	else()
		find_package(Vulkan)
	endif()

	if(WIN32)
		find_package(RenderDoc)
	endif()
endif()
	
# Target
add_library(bsfVulkanRenderAPI SHARED ${BS_VULKANRENDERAPI_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfVulkanRenderAPI)

# Includes
target_include_directories(bsfVulkanRenderAPI PRIVATE
	"./"
	"../Shared/"
)

# Libraries	
## External libs: Vulkan, MoltenVK, glslang, SPIR-V Cross

### Note: It's important to link glslang before Vulkan, as it contains its own glslang libraries. This ensures the search finds out local version first.
target_link_libraries(bsfVulkanRenderAPI PRIVATE ${glslang_LIBRARIES})
target_link_libraries(bsfVulkanRenderAPI PRIVATE ${SPIRVCross_LIBRARIES})

if(APPLE) # MacOS
	target_link_libraries(bsfVulkanRenderAPI PRIVATE ${MoltenVK_LIBRARIES})
else()
	target_link_libraries(bsfVulkanRenderAPI PRIVATE ${Vulkan_LIBRARIES})
endif()

if(WIN32)
	target_link_libraries(bsfVulkanRenderAPI PRIVATE ${RenderDoc_LIBRARIES})

	#install(IMPORTED_RUNTIME_ARTIFACTS bsfVulkanRenderAPI RUNTIME)

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
	target_link_framework(bsfVulkanRenderAPI IOSurface)
endif()

## Local libs
target_link_libraries(bsfVulkanRenderAPI PRIVATE bsf)

# IDE specific
set_property(TARGET bsfVulkanRenderAPI PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDER_BACKEND MATCHES "Vulkan" OR B3D_BUILD_ALL_RENDER_BACKENDS)
	B3DSetUpPostBuildAndInstallSteps(bsfVulkanRenderAPI)
endif()

conditional_cotire(bsfVulkanRenderAPI)
