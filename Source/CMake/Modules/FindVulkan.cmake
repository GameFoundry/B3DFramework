# Find Vulkan installation
#
# This module defines
#  Vulkan_INCLUDE_DIRS
#  Vulkan_LIBRARIES
#  Vulkan_FOUND

start_find_package(Vulkan)

set(Vulkan_INSTALL_DIR "$ENV{VULKAN_SDK}" CACHE PATH "")
gen_default_lib_search_dirs(Vulkan)

if(WIN32)
	set(Vulkan_LIBNAME vulkan-1)
	list(APPEND Vulkan_INCLUDE_SEARCH_DIRS "${Vulkan_INSTALL_DIR}/Include")

	if(BS_64BIT)
		list(APPEND Vulkan_LIBRARY_RELEASE_SEARCH_DIRS "${Vulkan_INSTALL_DIR}/Bin")
		list(APPEND Vulkan_LIBRARY_DEBUG_SEARCH_DIRS "${Vulkan_INSTALL_DIR}/Bin")
	else()
		list(APPEND Vulkan_LIBRARY_RELEASE_SEARCH_DIRS "${Vulkan_INSTALL_DIR}/Bin32")
		list(APPEND Vulkan_LIBRARY_DEBUG_SEARCH_DIRS "${Vulkan_INSTALL_DIR}/Bin32")
	endif()
else()
	set(Vulkan_LIBNAME vulkan)
endif()

find_imported_includes(Vulkan vulkan/vulkan.h)
find_imported_library_shared(Vulkan ${Vulkan_LIBNAME})

if(APPLE)
	find_imported_library_shared(Vulkan MoltenVK)
	find_imported_library(Vulkan SPIRV-Tools-opt)
endif()

end_find_package(Vulkan ${Vulkan_LIBNAME})

