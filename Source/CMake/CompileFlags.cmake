# Default compiler/linker flags, factored into reusable per-toolchain helpers.
#
# The public entry point is B3DSetDefaultLinkAndCompileFlags(target); it is called on every
# non-imported target. Platforms may REDEFINE it (in their Framework/Platform/<X> CMake) to
# compose a different set of flags — the helpers below are the reusable building blocks:
#
#   B3DSetDefaultCompileAndLinkerFlagsMSVC(target)   - MSVC compiler + linker flags
#   B3DSetDefaultCompileAndLinkerFlagsGCC(target)    - portable GCC/Clang flags (the common base)
#   B3DSetDefaultCompileAndLinkerFlagsClang(target)  - GCC base + Clang-specific flags
#   B3DSetVersionProperties(target)                  - VERSION/SOVERSION for shared libraries
#   B3DSetRPATHInstallProperty(target)               - INSTALL_RPATH for the host loader
#

# MSVC compiler + linker flags.
function(B3DSetDefaultCompileAndLinkerFlagsMSVC target)
	get_target_property(target_type ${target} TYPE)

	# Linker
	# The VS generator seems picky about how the linker flags are passed: we have to make sure
	# the options are quoted correctly and with append_string or random semicolons will be
	# inserted in the command line; and unrecognised options are only treated as warnings
	# and not errors so they won't be caught by CI. Make sure the options are separated by
	# spaces too.
	# For some reason this does not apply to the compiler options...

	set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS "/DYNAMICBASE /NOLOGO")

	set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_DEBUG "/DEBUG")
	set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO "/DEBUG /LTCG:incremental /INCREMENTAL:NO /OPT:REF")
	set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_MINSIZEREL "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF")
	set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF")

	if(B3D_IS_64BIT)
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO " /OPT:ICF")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_MINSIZEREL " /OPT:ICF")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE " /OPT:ICF")
	endif()

	if (${target_type} STREQUAL "SHARED_LIBRARY" OR ${target_type} STREQUAL "MODULE_LIBRARY")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /DLL")
	endif()

	# Compiler
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS /GS- /W3 /WX- /MP /fp:fast /fp:except- /nologo /bigobj /wd4577 /wd4530)
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -DWIN32 -D_WINDOWS)

	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/Od /RTC1 /MDd -DDEBUG>)

	if(B3D_IS_64BIT) # Debug edit and continue for 64-bit
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/ZI>)
	else() # Normal debug for 32-bit
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/Zi>)
	endif()

	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:RelWithDebInfo>:/GL /Gy /Zi /O2 /Oi /MD -DDEBUG>)
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:MinSizeRel>:/GL /Gy /Zi /O2 /Oi /MD -DNDEBUG>)
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:/GL /Gy /Zi /O2 /Oi /MD -DNDEBUG>)

	# Global defines
	#add_definitions(-D_HAS_EXCEPTIONS=0)
endfunction()

# Portable GCC/Clang flags shared by every compiler in that family (the common base).
function(B3DSetDefaultCompileAndLinkerFlagsGCC target)
	# Note: Optionally add -ffunction-sections, -fdata-sections, but with linker option --gc-sections
	# TODO: Use link-time optimization -flto. Might require non-default linker.
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wno-unused-parameter -Wno-reorder-ctor -fPIC -fno-strict-aliasing -ffast-math)

	# SIMD baseline. ARM64 requires no flag (NEON is part of the base ISA).
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i.86)$")
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -msse4.1)
	endif()

	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:-ggdb -O0 -DDEBUG>)
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:RelWithDebInfo>:-ggdb -O2 -DDEBUG -Wno-unused-variable>)
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:MinSizeRel>:-ggdb -O2 -DNDEBUG -Wno-unused-variable>)
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:-ggdb -O2 -DNDEBUG -Wno-unused-variable>)
endfunction()

# Clang flags: the GCC base plus Clang-specific options.
function(B3DSetDefaultCompileAndLinkerFlagsClang target)
	B3DSetDefaultCompileAndLinkerFlagsGCC(${target})

	set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fno-ms-compatibility)

	if(APPLE)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fobjc-arc $<$<COMPILE_LANGUAGE:CXX>:-std=c++1z>)
	endif()
endfunction()

# VERSION/SOVERSION for shared libraries.
function(B3DSetVersionProperties target)
	get_target_property(target_type ${target} TYPE)

	if (${target_type} STREQUAL "SHARED_LIBRARY")
		set_property(TARGET ${target} PROPERTY VERSION ${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH})
		set_property(TARGET ${target} PROPERTY SOVERSION ${B3D_FRAMEWORK_VERSION_MAJOR})
	endif()
endfunction()

# Loader search path for installed binaries.
function(B3DSetRPATHInstallProperty target)
	get_target_property(target_type ${target} TYPE)

	if(APPLE)
		set_property(TARGET ${target} PROPERTY INSTALL_RPATH "@loader_path;@loader_path/../lib;@loader_path/bsf-${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH}")
	else()
		if (${target_type} STREQUAL "EXECUTABLE")
			set_property(TARGET ${target} PROPERTY INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/../lib/bsf-${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH}")
		else()
			set_property(TARGET ${target} PROPERTY INSTALL_RPATH "\$ORIGIN:\$ORIGIN/bsf-${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH}")
		endif()
	endif()
endfunction()

# Default implementation: dispatches to the per-toolchain helper, then layers on the
# desktop-only extras (-fveclib=libmvec, -no-pie). Platforms can redefine this function.
function(B3DSetDefaultLinkAndCompileFlags target)
	get_target_property(target_type ${target} TYPE)

	if(MSVC)
		B3DSetDefaultCompileAndLinkerFlagsMSVC(${target})
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
		B3DSetDefaultCompileAndLinkerFlagsClang(${target})

		# glibc vector-math library
		if(LINUX)
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fveclib=libmvec)
		endif()

		# Desktop executables are linked non-PIE
		if (${target_type} STREQUAL "EXECUTABLE" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_DEBUG -no-pie)
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELWITHDEBINFO -no-pie)
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_MINSIZEREL -no-pie)
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELEASE -no-pie)
		endif()
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		B3DSetDefaultCompileAndLinkerFlagsGCC(${target})

		# glibc vector-math library
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fveclib=libmvec)

		# Desktop executables are linked non-PIE
		if (${target_type} STREQUAL "EXECUTABLE")
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_DEBUG -no-pie)
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELWITHDEBINFO -no-pie)
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_MINSIZEREL -no-pie)
			set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELEASE -no-pie)
		endif()
	else()
		# TODO_OTHER_COMPILERS_GO_HERE
	endif()

	B3DSetVersionProperties(${target})
	B3DSetRPATHInstallProperty(${target})
endfunction()
