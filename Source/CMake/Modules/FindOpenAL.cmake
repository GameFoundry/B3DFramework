# Find OpenAL dependency
#
# This module defines
#  OpenAL_INCLUDE_DIRS
#  OpenAL_LIBRARIES
#  OpenAL_FOUND

start_find_package(OpenAL)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(OpenAL_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/OpenAL CACHE PATH "")
endif()
gen_default_lib_search_dirs(OpenAL)

if(WIN32)
	set(OpenAL_LIBNAME OpenAL32)
else()
	set(OpenAL_LIBNAME openal)
endif()

find_imported_includes(OpenAL AL/al.h)

if(APPLE)
	find_imported_library(OpenAL ${OpenAL_LIBNAME})
else()
	find_imported_library_shared(OpenAL ${OpenAL_LIBNAME})
endif()

if(B3D_USE_BUNDLED_LIBRARIES)
	install_dependency_binaries(OpenAL)
endif()

end_find_package(OpenAL ${OpenAL_LIBNAME})
