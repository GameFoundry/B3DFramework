# Generate the CS version information class for framework and engine
if(B3D_IS_ENGINE)
	configure_file("${BSF_SOURCE_DIR}/Scripting/bsfSharp/EngineVersion.cs.in" "${BSF_SOURCE_DIR}/Scripting/bsfSharp/EngineVersion.cs")
endif()
configure_file("${BSF_SOURCE_DIR}/Scripting/bsfSharp/FrameworkVersion.cs.in" "${BSF_SOURCE_DIR}/Scripting/bsfSharp/FrameworkVersion.cs")

# Generate the CS project file
if(B3D_IS_ENGINE)
	set(assemblyName "MBansheeEngine")
else()
	set(assemblyName "bsfSharpCore")
endif()

if(B3D_IS_ENGINE)
	set(defines "IS_B3D;")
endif()

# Include the CS project in the build
B3DGlobSourceFilesWithExplicitPattern(${CMAKE_CURRENT_SOURCE_DIR} "cs" "" "" csFiles)
add_library(${assemblyName} SHARED ${csFiles})

if(B3D_USE_DOTNETCORE)
	set(defines "${defines}B3D_USE_DOTNETCORE;")

	set_target_properties(${assemblyName} PROPERTIES
			DOTNET_SDK "Microsoft.NET.Sdk"
			DOTNET_TARGET_FRAMEWORK "net9.0"
			VS_GLOBAL_ROOTNAMESPACE bs)
else()
	set_target_properties(${assemblyName} PROPERTIES
			DOTNET_TARGET_FRAMEWORK_VERSION "v4.5"
			VS_GLOBAL_ROOTNAMESPACE bs
			VS_DOTNET_REFERENCES "System;System.Core")
endif()


target_compile_definitions(${assemblyName} PRIVATE ${defines})
set_property(TARGET ${assemblyName} PROPERTY FOLDER Script)

# Copy assembly to correct output location
set(assemblyOutputFolder "${PROJECT_BINARY_DIR}/bin/Assemblies")
add_custom_command(
		TARGET ${assemblyName} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>
		COMMAND ${CMAKE_COMMAND}
		ARGS    -E copy_if_different "$<TARGET_FILE:${assemblyName}>" $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>
		COMMAND ${CMAKE_COMMAND}
		ARGS    -E copy_if_different "$<TARGET_PDB_FILE:${assemblyName}>" $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>
		COMMENT "Copying $<TARGET_FILE:${assemblyName}> to $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>\n"
		COMMAND_EXPAND_LISTS
)

# Convert PDB to MDB
if(WIN32 AND NOT USE_DOTNETCORE)
	add_custom_command(
			TARGET ${assemblyName} POST_BUILD
			COMMAND cmd /c $ENV{MONO_INSTALL_DIR}/bin/pdb2mdb.bat
			ARGS    $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>$<TARGET_FILE_NAME:${assemblyName}>
			COMMENT "Converting $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>$<TARGET_FILE_NAME:${assemblyName}>\n"
			COMMAND_EXPAND_LISTS
	)
endif()

set(csProjectName "${assemblyName}.csproj")

if(NOT WIN32)
	find_package(mcs)

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(csProjectConfiguration Debug)
	else()
		set(csProjectConfiguration Release)
	endif()

	add_custom_target(${assemblyName}
			COMMAND ${msbuild_EXECUTABLE} /p:Configuration=${csProjectConfiguration} ${PROJECT_BINARY_DIR}/${csProjectName}
			COMMENT "Building managed assembly \"${assemblyName}\"")
endif()

# Install managed assemblies
install(
	DIRECTORY ${PROJECT_BINARY_DIR}/bin/Assemblies
	DESTINATION bin/
)
