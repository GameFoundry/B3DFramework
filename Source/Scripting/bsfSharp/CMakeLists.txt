# Generate the CS version information class for framework and engine
if(B3D_IS_ENGINE)
	configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfSharp/EngineVersion.cs.in" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfSharp/EngineVersion.cs")
endif()
configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfSharp/FrameworkVersion.cs.in" "${B3D_FRAMEWORK_SOURCE_FOLDER}/Scripting/bsfSharp/FrameworkVersion.cs")

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

if(CMAKE_GENERATOR MATCHES "Visual Studio")
	# Visual Studio generates and builds the C# project natively
	add_library(${assemblyName} SHARED ${csFiles})

	set_target_properties(${assemblyName} PROPERTIES
			DOTNET_SDK "Microsoft.NET.Sdk"
			DOTNET_TARGET_FRAMEWORK "net9.0"
			VS_GLOBAL_ROOTnamespace b3d)

	target_compile_definitions(${assemblyName} PRIVATE ${defines})

	# Copy assembly to correct output location
	set(assemblyOutputFolder "${PROJECT_BINARY_DIR}/bin/Assemblies")
	add_custom_command(
			TARGET ${assemblyName} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>
			COMMAND ${CMAKE_COMMAND}
			ARGS    -E copy_if_different "$<TARGET_FILE:${assemblyName}>" $<$<CONFIG:Debug>:${assemblyOutputFolder}/Debug/>$<$<NOT:$<CONFIG:Debug>>:${assemblyOutputFolder}/Release/>
			COMMAND_EXPAND_LISTS
	)

	# If AOT mode is enabled, compiles this assembly and all BCL assemblies using the AOT compiler. The assemblies must already be present in the Assemblies folder.
	B3DAddPostBuildAOTCompileIfNeeded(${assemblyName} "$<TARGET_FILE_NAME:${assemblyName}>")
else()
	B3DAddManagedAssembly(${assemblyName} "${csFiles}" "${defines}" "")
endif()

set_property(TARGET ${assemblyName} PROPERTY FOLDER Script)

# Install managed assemblies
install(
	DIRECTORY ${PROJECT_BINARY_DIR}/bin/Assemblies
	DESTINATION bin/
)
