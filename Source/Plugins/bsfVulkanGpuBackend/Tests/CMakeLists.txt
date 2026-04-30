if(B3D_GPU_BACKEND MATCHES "Vulkan" OR B3D_BUILD_ALL_PLUGINS)
	B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_VULKAN_TESTS_SRC)

	add_library(bsfVulkanGpuBackendTests SHARED ${BS_VULKAN_TESTS_SRC})

	# Common flags
	B3DSetDefaultLinkAndCompileFlags(bsfVulkanGpuBackendTests)

	# Pick up framework public include paths + compile flags via the bsfHeaders interface
	target_link_libraries(bsfVulkanGpuBackendTests PRIVATE bsfHeaders)

	# Plugin-private headers and the Shared/ tree the plugin uses for cross-plugin types.
	target_include_directories(bsfVulkanGpuBackendTests PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/.."
		"${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/")

	# Vulkan headers
	if(APPLE)
		target_link_libraries(bsfVulkanGpuBackendTests PRIVATE ${MoltenVK_LIBRARIES})
	else()
		target_link_libraries(bsfVulkanGpuBackendTests PRIVATE ${Vulkan_LIBRARIES})
	endif()

	# Libraries
	# - In monolithic mode, Vulkan backend is part of the framework library, otherwise we link its library
	if(B3D_MONOLITHIC_BUILD)
		target_link_libraries(bsfVulkanGpuBackendTests PRIVATE bsf)
	else()
		target_link_libraries(bsfVulkanGpuBackendTests PRIVATE bsf bsfVulkanGpuBackend)
	endif()

	# IDE specific
	set_property(TARGET bsfVulkanGpuBackendTests PROPERTY FOLDER Tests)

	# Install / post-build copy alongside UnitTestRunner.exe so the discovery scanner finds it.
	B3DSetUpPostBuildAndInstallSteps(bsfVulkanGpuBackendTests)

	# Self-register so Framework/Source/Tests/CMakeLists.txt can set up a dependency from UnitTestRunner to this test.
	set_property(GLOBAL APPEND PROPERTY B3D_PLUGIN_TEST_TARGETS bsfVulkanGpuBackendTests)
endif()
