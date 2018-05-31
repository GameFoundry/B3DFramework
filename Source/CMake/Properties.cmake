include(CheckCXXCompilerFlag)

# Configuration types
if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
	set(CMAKE_CONFIGURATION_TYPES "Debug;OptimizedDebug;Release;" CACHE INTERNAL "")
else()
	if(NOT CMAKE_BUILD_TYPE)
		message("Defaulting to release build.")
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
	# Set the valid options for cmake-gui drop-down list
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug OptimizedDebug Release)
endif()

# Includes required for various find_package calls
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${BSF_SOURCE_DIR}/CMake/Modules/")

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(BS_64BIT true)
endif()

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

# Global compile & linker flags
## Compiler-agnostic settings
### Target at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_INSTALL_RPATH "\$ORIGIN")

## Compiler-specific settings
if(MSVC)
	# Linker
	set(BS_LINKER_FLAGS_COMMON "/DYNAMICBASE /NOLOGO")

	set(BS_LINKER_FLAGS_DEBUG "${BS_LINKER_FLAGS_COMMON} /DEBUG")
	set(BS_LINKER_FLAGS_OPTIMIZEDDEBUG "${BS_LINKER_FLAGS_COMMON} /LTCG:incremental /INCREMENTAL:NO /OPT:REF /DEBUG")
	set(BS_LINKER_FLAGS_RELEASE "${BS_LINKER_FLAGS_COMMON} /DEBUG /LTCG /INCREMENTAL:NO /OPT:REF")

	if(BS_64BIT)
		set(BS_LINKER_FLAGS_OPTIMIZEDDEBUG "${BS_LINKER_FLAGS_OPTIMIZEDDEBUG} /OPT:ICF")
		set(BS_LINKER_FLAGS_RELEASE "${BS_LINKER_FLAGS_RELEASE} /OPT:ICF")
	endif()

	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DLL ${BS_LINKER_FLAGS_DEBUG}")
	set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/DLL ${BS_LINKER_FLAGS_DEBUG}")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${BS_LINKER_FLAGS_DEBUG})

	set(CMAKE_SHARED_LINKER_FLAGS_OPTIMIZEDDEBUG "/DLL ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG}")
	set(CMAKE_MODULE_LINKER_FLAGS_OPTIMIZEDDEBUG "/DLL ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG}")
	set(CMAKE_EXE_LINKER_FLAGS_OPTIMIZEDDEBUG ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG})

	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DLL ${BS_LINKER_FLAGS_RELEASE}")
	set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "/DLL ${BS_LINKER_FLAGS_RELEASE}")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${BS_LINKER_FLAGS_RELEASE})

	# Compiler
	set(BS_COMPILER_FLAGS_COMMON "/GS- /W3 /GR- /WX- /MP /nologo /bigobj /wd\"4577\" /wd\"4530\"")
	set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS")

	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} /Od /RTC1 /MDd -DDEBUG")

	if(BS_64BIT) # Debug edit and continue for 64-bit
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
	else() # Normal debug for 32-bit
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
	endif()

	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} /GL /Gy /Zi /O2 /Oi /MD -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} /GL /Gy /Zi /O2 /Oi /MD -DNDEBUG")

	# Global defines
	#add_definitions(-D_HAS_EXCEPTIONS=0)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
	# Note: Optionally add -ffunction-sections, -fdata-sections, but with linker option --gc-sections
	# TODO: Use link-time optimization -flto. Might require non-default linker.
	set(BS_COMPILER_FLAGS_COMMON "-Wall -Wextra -Wno-unused-parameter -fPIC -fno-exceptions -fno-strict-aliasing -fno-rtti -fno-ms-compatibility")

	if(APPLE)
		set(BS_COMPILER_FLAGS_COMMON "${BS_COMPILER_FLAGS_COMMON} -fobjc-arc -std=c++1z")
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} -ggdb -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} -ggdb -O2 -DDEBUG -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} -ggdb -O2 -DNDEBUG -Wno-unused-variable")

	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-no-pie")
		set(CMAKE_EXE_LINKER_FLAGS_OPTIMIZEDDEBUG "-no-pie")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-no-pie")
	endif()

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# TODO: Use link-time optimization -flto. Might require non-default linker.
	set(BS_COMPILER_FLAGS_COMMON "-Wall -Wextra -Wno-unused-parameter -fPIC -fno-exceptions -fno-strict-aliasing -fno-rtti")

	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} -ggdb -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} -ggdb -O2 -DDEBUG -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} -ggdb -O2 -DNDEBUG -Wno-unused-variable")

	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-no-pie")
	set(CMAKE_EXE_LINKER_FLAGS_OPTIMIZEDDEBUG "-no-pie")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-no-pie")
else()
# TODO_OTHER_COMPILERS_GO_HERE
endif()

# Enable colored output
if (CMAKE_GENERATOR STREQUAL "Ninja")
	check_cxx_compiler_flag("-fdiagnostics-color=always" F_DIAGNOSTIC_COLOR_ALWAYS)
	if (F_DIAGNOSTIC_COLOR_ALWAYS)
		add_compile_options("-fdiagnostics-color=always")
	endif()
endif()

set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
set(CMAKE_FIND_FRAMEWORK "LAST")

# Output
if(BS_64BIT)
	set(BS_OUTPUT_DIR_PREFIX x64)
else()
	set(BS_OUTPUT_DIR_PREFIX x86)
endif()

set(BS_BINARY_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin/${BS_OUTPUT_DIR_PREFIX})
set(BS_LIBRARY_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/lib/${BS_OUTPUT_DIR_PREFIX})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BS_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_BINARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BS_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BS_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_BINARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BS_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${BS_LIBRARY_OUTPUT_DIR}/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_LIBRARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${BS_LIBRARY_OUTPUT_DIR}/Release)

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

# Look for global/system dependencies
if (UNIX)
	# macOS
	if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
		# Find tools used for stripping binaries
		find_program(DSYMUTIL_TOOL dsymutil)
		
		if (NOT DSYMUTIL_TOOL)
			message(FATAL_ERROR "Could not find 'dsymutil' tool.")
		endif()
			
		find_program(STRIP_TOOL strip)
		if (NOT STRIP_TOOL)
			message(FATAL_ERROR "Could not find 'strip' tool.")
		endif()
	
	# Linux
	else()
		# Find tools used for stripping binaries
	    find_program(OBJCOPY_TOOL objcopy)
		
	    if (NOT OBJCOPY_TOOL)
	        message(FATAL_ERROR "Could not find 'objcopy' tool.")
	    endif()
		
	endif()
endif()