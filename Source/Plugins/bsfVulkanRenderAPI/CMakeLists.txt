# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_VULKANRENDERAPI_SRC)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "../Shared/" "" BS_VULKANRENDERAPI_SRC)

# Packages
if(B3D_RENDER_BACKEND MATCHES "Vulkan" OR B3D_BUILD_ALL_PLUGINS)
	# Ensure pre-built binary dependencies are up to date
	if(B3D_USE_BUNDLED_LIBRARIES)
		B3DCheckAndUpdatePrebuiltDependency(glslang)
		B3DCheckAndUpdatePrebuiltDependency(SPIRVCross)

		if(WIN32)
			B3DCheckAndUpdatePrebuiltDependency(RenderDoc)
		endif()
	endif()

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
	B3DTargetLinkFramework(bsfVulkanRenderAPI Foundation)
	B3DTargetLinkFramework(bsfVulkanRenderAPI CoreGraphics)
	B3DTargetLinkFramework(bsfVulkanRenderAPI CoreVideo)
	B3DTargetLinkFramework(bsfVulkanRenderAPI IOKit)
	B3DTargetLinkFramework(bsfVulkanRenderAPI AppKit)
	B3DTargetLinkFramework(bsfVulkanRenderAPI QuartzCore)
	B3DTargetLinkFramework(bsfVulkanRenderAPI Metal)
	B3DTargetLinkFramework(bsfVulkanRenderAPI IOSurface)
endif()

## Local libs
target_link_libraries(bsfVulkanRenderAPI PRIVATE bsf)

# IDE specific
set_property(TARGET bsfVulkanRenderAPI PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDER_BACKEND MATCHES "Vulkan" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfVulkanRenderAPI)
endif()
