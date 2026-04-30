# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "Tests" B3D_VULKANGPUBACKEND_SOURCE_FILES)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "../Shared/" "" B3D_VULKANGPUBACKEND_SOURCE_FILES)

# Packages
if(B3D_GPU_BACKEND MATCHES "Vulkan" OR B3D_BUILD_ALL_PLUGINS)
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
B3DAddPlugin(bsfVulkanGpuBackend ENGINE ${B3D_VULKANGPUBACKEND_SOURCE_FILES})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfVulkanGpuBackend)

# Includes
target_include_directories(bsfVulkanGpuBackend PRIVATE
	"./"
	"../Shared/"
)

# Libraries	
## External libs: Vulkan, MoltenVK, glslang, SPIR-V Cross

### Note: It's important to link glslang before Vulkan, as it contains its own glslang libraries. This ensures the search finds out local version first.
target_link_libraries(bsfVulkanGpuBackend PRIVATE ${glslang_LIBRARIES})
target_link_libraries(bsfVulkanGpuBackend PRIVATE ${SPIRVCross_LIBRARIES})

if(APPLE) # MacOS
	target_link_libraries(bsfVulkanGpuBackend PRIVATE ${MoltenVK_LIBRARIES})
else()
	target_link_libraries(bsfVulkanGpuBackend PRIVATE ${Vulkan_LIBRARIES})
endif()

if(WIN32)
	target_link_libraries(bsfVulkanGpuBackend PRIVATE ${RenderDoc_LIBRARIES})
endif()

## OS libs
if(APPLE) # MacOS
	B3DTargetLinkFramework(bsfVulkanGpuBackend Foundation)
	B3DTargetLinkFramework(bsfVulkanGpuBackend CoreGraphics)
	B3DTargetLinkFramework(bsfVulkanGpuBackend CoreVideo)
	B3DTargetLinkFramework(bsfVulkanGpuBackend IOKit)
	B3DTargetLinkFramework(bsfVulkanGpuBackend AppKit)
	B3DTargetLinkFramework(bsfVulkanGpuBackend QuartzCore)
	B3DTargetLinkFramework(bsfVulkanGpuBackend Metal)
	B3DTargetLinkFramework(bsfVulkanGpuBackend IOSurface)
endif()

# IDE specific
set_property(TARGET bsfVulkanGpuBackend PROPERTY FOLDER Plugins)

# Install
if(B3D_GPU_BACKEND MATCHES "Vulkan" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfVulkanGpuBackend)
endif()

# Tests
if(B3D_BUILD_TESTS)
	add_subdirectory(Tests)
endif()
