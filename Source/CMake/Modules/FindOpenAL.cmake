# Find OpenAL dependency
#
# This module defines
#  OpenAL_INCLUDE_DIRS
#  OpenAL_LIBRARIES
#  OpenAL_FOUND

start_find_package(OpenAL)

set(OpenAL_INSTALL_DIR ${PROJECT_SOURCE_DIR}/../Dependencies/OpenAL CACHE PATH "")
gen_default_lib_search_dirs(OpenAL)

if(WIN32)
	set(OpenAL_LIBNAME OpenAL32)
else()
	set(OpenAL_LIBNAME openal)
endif()

find_imported_includes(OpenAL AL/al.h)
find_imported_library_shared(OpenAL ${OpenAL_LIBNAME})

install_dependency_binaries(OpenAL)

end_find_package(OpenAL ${OpenAL_LIBNAME})
