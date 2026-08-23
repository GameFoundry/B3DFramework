if(B3D_GPU_BACKEND MATCHES "DirectX12" OR B3D_BUILD_ALL_PLUGINS)
	B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_D3D12_TESTS_SRC)
	list(APPEND BS_D3D12_TESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../B3DD3D12BarrierUtility.cpp")
	list(APPEND BS_D3D12_TESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../Utility/B3DD3D12BarrierBatch.cpp")

	add_library(bsfD3D12GpuBackendTests SHARED ${BS_D3D12_TESTS_SRC})

	B3DSetDefaultLinkAndCompileFlags(bsfD3D12GpuBackendTests)

	target_link_libraries(bsfD3D12GpuBackendTests PRIVATE bsfHeaders)
	target_include_directories(bsfD3D12GpuBackendTests PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/..")
	B3DUseD3D12AgilitySDKHeaders(bsfD3D12GpuBackendTests)

	target_link_libraries(bsfD3D12GpuBackendTests PRIVATE bsf)

	set_property(TARGET bsfD3D12GpuBackendTests PROPERTY FOLDER Tests)

	B3DSetUpPostBuildAndInstallSteps(bsfD3D12GpuBackendTests)
	set_property(GLOBAL APPEND PROPERTY B3D_PLUGIN_TEST_TARGETS bsfD3D12GpuBackendTests)
endif()
