set(FONT_AWESOME_INSTALL_DIR ${B3D_FRAMEWORK_ROOT_FOLDER}/Dependencies/FontAwesome CACHE PATH "")
mark_as_advanced(FONT_AWESOME_INSTALL_DIR)

# Registers a target that can be used for building stock icons.
function(B3DRegisterBuildStockIconsTarget)
	set(BuildStockIcons_FONTAWESOME_DIRECTORY ${FONT_AWESOME_INSTALL_DIR})
	set(BuildStockIcons_ICON_LIST_OUTPUT_DIRECTORY ${B3D_FRAMEWORK_ROOT_FOLDER}/Source/Engine/Core/Text)
	set(BuildStockIcons_FONT_OUTPUT_DIRECTORY ${B3D_FRAMEWORK_ROOT_FOLDER}/Data/Raw/Fonts)

	configure_file(
			${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Scripts/BuildStockIcons.cmake.in
			${CMAKE_CURRENT_BINARY_DIR}/BuildStockIcons.cmake
			@ONLY)

	add_custom_target(BuildStockIcons COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/BuildStockIcons.cmake)
	set_property(TARGET BuildStockIcons PROPERTY FOLDER Scripts)
endfunction()

B3DRegisterBuildStockIconsTarget()
