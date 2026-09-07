# B3D Framework - Copyright 2026 Marko Pintera. Licensed under the MIT license (LICENSE.md).

# Builds a managed library with the bundled Roslyn compiler for generators without C# support.
#
# @param	assemblyName	Name of the target and output assembly.
# @param	sourceFiles		C# source files, relative to the current source directory.
# @param	defines			Preprocessor symbols for the assembly.
# @param	references		Additional managed target names to reference.
function(B3DAddManagedAssembly assemblyName sourceFiles defines references)
	set(compilerFolder "${DotNETCoreMono_INSTALL_DIR}/bin/Compiler")
	set(compilerExecutable "${compilerFolder}/dotnet${CMAKE_EXECUTABLE_SUFFIX}")
	if(NOT EXISTS "${compilerExecutable}" OR NOT EXISTS "${compilerFolder}/csc.dll")
		message(FATAL_ERROR "Bundled Roslyn compiler missing. Run bash ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Scripts/B3DBuildMono.sh to build and package the runtime and compiler.")
	endif()

	set(assemblyFolder "${PROJECT_BINARY_DIR}/bin/Assemblies/$<IF:$<CONFIG:Debug>,Debug,Release>")
	set(assemblyPath "${assemblyFolder}/${assemblyName}.dll")
	set(arguments "-nologo\n-nostdlib+\n-target:library\n-unsafe+\n-langversion:13\n-debug:portable\n-optimize$<IF:$<CONFIG:Debug>,-,+>\n-out:\"${assemblyPath}\"\n")
	if(defines)
		string(APPEND arguments "-define:${defines}\n")
	endif()
	string(APPEND arguments "$<$<CONFIG:Debug>:-define:DEBUG;TRACE>\n")

	file(GLOB frameworkAssemblies CONFIGURE_DEPENDS "${DotNETCoreMono_INSTALL_DIR}/bin/Assemblies/*.dll")
	set(referenceFiles ${frameworkAssemblies})

	foreach(reference IN LISTS references)
		list(APPEND referenceFiles "${assemblyFolder}/${reference}.dll")
	endforeach()

	foreach(reference IN LISTS referenceFiles)
		string(APPEND arguments "-reference:\"${reference}\"\n")
	endforeach()

	foreach(sourceFile IN LISTS sourceFiles)
		get_filename_component(sourcePath "${sourceFile}" ABSOLUTE)
		string(APPEND arguments "\"${sourcePath}\"\n")
	endforeach()

	set(responsePath "${CMAKE_CURRENT_BINARY_DIR}/${assemblyName}-$<CONFIG>.rsp")
	file(GENERATE OUTPUT "${responsePath}" CONTENT "${arguments}")
	add_custom_command(OUTPUT "${assemblyPath}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${assemblyFolder}"
		COMMAND "${compilerExecutable}" "${compilerFolder}/csc.dll" -noconfig "@${responsePath}"
		DEPENDS ${sourceFiles} ${referenceFiles} "${responsePath}" "${compilerFolder}/csc.dll"
		COMMENT "Building managed assembly ${assemblyName}"
		VERBATIM)
	add_custom_target(${assemblyName} DEPENDS "${assemblyPath}")

	if(references)
		add_dependencies(${assemblyName} ${references})
	endif()
endfunction()
