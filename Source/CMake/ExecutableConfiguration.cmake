# Copyright (c) 2026 Banshee Engine

# Registers a callback that applies platform or plugin-owned configuration to every executable created after
# the provider is registered. The callback is passed the executable target name.
#
# @param	provider	Name of the callback command to register.
function(B3DRegisterExecutableConfigurationProvider provider)
	if(NOT COMMAND ${provider})
		message(FATAL_ERROR "Executable configuration provider '${provider}' is not defined.")
	endif()

	get_property(providers GLOBAL PROPERTY B3D_EXECUTABLE_CONFIGURATION_PROVIDERS)
	list(FIND providers ${provider} providerIndex)
	if(providerIndex EQUAL -1)
		set_property(GLOBAL APPEND PROPERTY B3D_EXECUTABLE_CONFIGURATION_PROVIDERS ${provider})
	endif()
endfunction()

# Invokes all registered executable configuration providers for @p target. Non-executable targets are ignored.
#
# @param	target	Name of the target to configure.
function(B3DApplyExecutableConfigurations target)
	get_target_property(targetType ${target} TYPE)
	if(NOT targetType STREQUAL "EXECUTABLE")
		return()
	endif()

	get_property(providers GLOBAL PROPERTY B3D_EXECUTABLE_CONFIGURATION_PROVIDERS)
	foreach(provider IN LISTS providers)
		cmake_language(CALL ${provider} ${target})
	endforeach()
endfunction()
