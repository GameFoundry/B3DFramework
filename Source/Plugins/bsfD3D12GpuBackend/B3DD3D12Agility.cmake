# Copyright (c) 2026 Banshee Engine

include_guard(GLOBAL)

include(FetchContent)

set(B3D_D3D12_AGILITY_PACKAGE_VERSION "1.619.4")
set(B3D_D3D12_AGILITY_SDK_VERSION 619)

FetchContent_Declare(B3DD3D12AgilitySDK
	URL "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/${B3D_D3D12_AGILITY_PACKAGE_VERSION}"
	URL_HASH "SHA256=D30F756CE05BB4B7705FC1B04A5DED32ED62F2C2A2B392AE8D3318181395C8BC"
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(B3DD3D12AgilitySDK)

if(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$")
	set(B3D_D3D12_AGILITY_ARCHITECTURE arm64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(B3D_D3D12_AGILITY_ARCHITECTURE x64)
else()
	set(B3D_D3D12_AGILITY_ARCHITECTURE win32)
endif()

set(B3D_D3D12_AGILITY_INCLUDE_DIRECTORY "${b3dd3d12agilitysdk_SOURCE_DIR}/build/native/include")
set(B3D_D3D12_AGILITY_BINARY_DIRECTORY "${b3dd3d12agilitysdk_SOURCE_DIR}/build/native/bin/${B3D_D3D12_AGILITY_ARCHITECTURE}")
set(B3D_D3D12_AGILITY_CORE_LIBRARY "${B3D_D3D12_AGILITY_BINARY_DIRECTORY}/D3D12Core.dll")
set(B3D_D3D12_AGILITY_LAYERS_LIBRARY "${B3D_D3D12_AGILITY_BINARY_DIRECTORY}/d3d12SDKLayers.dll")
set(B3D_D3D12_AGILITY_LICENSE "${b3dd3d12agilitysdk_SOURCE_DIR}/LICENSE.txt")

foreach(requiredPath
	${B3D_D3D12_AGILITY_INCLUDE_DIRECTORY}
	${B3D_D3D12_AGILITY_CORE_LIBRARY}
	${B3D_D3D12_AGILITY_LAYERS_LIBRARY}
	${B3D_D3D12_AGILITY_LICENSE})

	if(NOT EXISTS "${requiredPath}")
		message(FATAL_ERROR "The Direct3D 12 Agility SDK package is missing '${requiredPath}'.")
	endif()
endforeach()

set(agilityExportSource "${CMAKE_CURRENT_BINARY_DIR}/B3DD3D12AgilityExports.cpp")
configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/B3DD3D12AgilityExports.cpp.in"
	"${agilityExportSource}"
	@ONLY
)

set_property(GLOBAL PROPERTY B3D_D3D12_AGILITY_EXPORT_SOURCE "${agilityExportSource}")

# Adds the Agility SDK selection exports to an executable, ensuring they are present in the process module.
#
# @param	target	Name of the executable target to configure.
function(B3DApplyD3D12ExecutableConfiguration target)
	get_property(agilityExportSource GLOBAL PROPERTY B3D_D3D12_AGILITY_EXPORT_SOURCE)
	target_sources(${target} PRIVATE "${agilityExportSource}")
	source_group("Generated" FILES "${agilityExportSource}")
endfunction()

B3DRegisterExecutableConfigurationProvider(B3DApplyD3D12ExecutableConfiguration)

# Adds the Agility SDK headers ahead of the platform D3D12 headers used by @p target.
#
# @param	target	Name of the target that consumes D3D12 declarations.
function(B3DUseD3D12AgilitySDKHeaders target)
	target_include_directories(${target} BEFORE PRIVATE "${B3D_D3D12_AGILITY_INCLUDE_DIRECTORY}")
endfunction()

# Adds Agility SDK headers to @p target and stages the selected runtime beside the target. The SDK layers are included
# only in Development builds.
#
# @param	target	Name of the D3D12 backend target to configure.
function(B3DConfigureD3D12AgilitySDK target)
	B3DUseD3D12AgilitySDKHeaders(${target})

	set(runtimeDirectory "$<TARGET_FILE_DIR:${target}>/D3D12")
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "${runtimeDirectory}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${B3D_D3D12_AGILITY_CORE_LIBRARY}"
			"${B3D_D3D12_AGILITY_LICENSE}"
			"${runtimeDirectory}"
		COMMENT "Staging the Direct3D 12 Agility SDK runtime"
		VERBATIM
	)

	if(B3D_BUILD_TYPE STREQUAL "Development")
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${B3D_D3D12_AGILITY_LAYERS_LIBRARY}"
				"${runtimeDirectory}"
			COMMENT "Staging the Direct3D 12 Agility SDK debug layers"
			VERBATIM
		)
	else()
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E rm -f "${runtimeDirectory}/d3d12SDKLayers.dll"
			COMMENT "Removing Direct3D 12 SDK debug layers from the Shipping runtime"
			VERBATIM
		)
	endif()

	if(B3D_IS_ENGINE)
		set(installDirectory D3D12)
	else()
		set(installDirectory bin/D3D12)
	endif()

	install(FILES
		"${B3D_D3D12_AGILITY_CORE_LIBRARY}"
		"${B3D_D3D12_AGILITY_LICENSE}"
		DESTINATION ${installDirectory})

	if(B3D_BUILD_TYPE STREQUAL "Development")
		install(FILES "${B3D_D3D12_AGILITY_LAYERS_LIBRARY}"
			DESTINATION ${installDirectory})
	endif()
endfunction()
