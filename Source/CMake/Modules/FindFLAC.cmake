# Find FLAC dependency
#
# This module defines
#  FLAC_INCLUDE_DIRS
#  FLAC_LIBRARIES
#  FLAC_FOUND

start_find_package(FLAC)

if(USE_BUNDLED_LIBRARIES)
	set(FLAC_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/libFLAC CACHE PATH "")
endif()
gen_default_lib_search_dirs(FLAC)

if(WIN32)
	set(FLAC_LIBNAME libFLAC)
else()
	set(FLAC_LIBNAME FLAC)
endif()

find_imported_includes(FLAC FLAC/all.h)

if(UNIX)
	find_imported_library(FLAC ${FLAC_LIBNAME})
else()
	find_imported_library_shared(FLAC ${FLAC_LIBNAME})
endif()

if(USE_BUNDLED_LIBRARIES)
	if(WIN32)
		# .dll has a different name than .lib, so we must register it separately
		install_dependency_dll(FLAC ${BSF_SOURCE_DIR}/.. libFLAC_dynamic)
	endif()
	
	install_dependency_binaries(FLAC)
endif()

end_find_package(FLAC ${FLAC_LIBNAME})
