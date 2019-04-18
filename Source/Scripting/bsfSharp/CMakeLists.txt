# Generate the CS version information class for framework and engine
if(BS_IS_BANSHEE3D)
	configure_file("${BSF_SOURCE_DIR}/Scripting/bsfSharp/EngineVersion.cs.in" "${BSF_SOURCE_DIR}/Scripting/bsfSharp/EngineVersion.cs")
endif()
configure_file("${BSF_SOURCE_DIR}/Scripting/bsfSharp/FrameworkVersion.cs.in" "${BSF_SOURCE_DIR}/Scripting/bsfSharp/FrameworkVersion.cs")

# Generate the CS project file
if(BS_IS_BANSHEE3D)
	set(BS_CS_ASSEMBLY_NAME "MBansheeEngine")
else()
	set(BS_CS_ASSEMBLY_NAME "bsfSharpCore")
endif()

generate_csharp_project(${BSF_SOURCE_DIR}/Scripting/bsfSharp bsfSharp bs ${BS_CS_ASSEMBLY_NAME})

# Include the CS project in the build
set(BS_CS_PROJ "${BS_CS_ASSEMBLY_NAME}.csproj")

if(MSVC)
	include_external_msproject(${BS_CS_ASSEMBLY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${BS_CS_PROJ})
	set_property(TARGET ${BS_CS_ASSEMBLY_NAME} PROPERTY FOLDER Script)
else()
	find_package(mcs)

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CS_PROJ_CONFIG Debug)
	else()
		set(CS_PROJ_CONFIG Release)
	endif()

	add_custom_target(${BS_CS_ASSEMBLY_NAME}
			COMMAND xbuild /p:Configuration=${CS_PROJ_CONFIG} ${BS_CS_PROJ}
			COMMENT "Building managed assembly \"${BS_CS_ASSEMBLY_NAME}\"")
endif()

# Install managed assemblies
install(
	DIRECTORY ${PROJECT_BINARY_DIR}/bin/Assemblies
	DESTINATION bin/
)