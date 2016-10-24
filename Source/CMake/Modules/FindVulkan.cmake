# Find Vulkan installation
#
# This module defines
#  Vulkan_INCLUDE_DIRS
#  Vulkan_LIBRARIES_DEBUG
#  Vulkan_LIBRARIES_OPTIMIZED
#  Vulkan_LIBRARY_DIRS
#  Vulkan_FOUND

# TODO: Set default install paths for mac/unix
# TODO: Don't hardcode the version (instead use regex or GLOB to match latest)
set(Vulkan_INSTALL_DIRS "C:/VulkanSDK/1.0.17.0" CACHE PATH "")
set(Vulkan_INCLUDE_SEARCH_DIRS "${Vulkan_INSTALL_DIRS}/Include")

if(BS_64BIT)
	set(Vulkan_LIBRARY_SEARCH_DIRS "${Vulkan_INSTALL_DIRS}/Bin")
else()
	set(Vulkan_LIBRARY_SEARCH_DIRS "${Vulkan_INSTALL_DIRS}/Bin32")
endif()

message(STATUS "Looking for Vulkan installation...")
find_path(Vulkan_INCLUDE_DIR vulkan/vulkan.h PATHS ${Vulkan_INCLUDE_SEARCH_DIRS})

find_library(Vulkan_LIBRARY_OPTIMIZED NAMES vulkan-1 PATHS ${Vulkan_LIBRARY_SEARCH_DIRS})
find_library(Vulkan_LIBRARY_DEBUG NAMES vulkan-1 PATHS ${Vulkan_LIBRARY_SEARCH_DIRS})

if(Vulkan_INCLUDE_DIR AND Vulkan_LIBRARY_OPTIMIZED AND Vulkan_LIBRARY_DEBUG)
	set(Vulkan_FOUND TRUE)
endif()

if(NOT Vulkan_FOUND)
	if(Vulkan_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find Vulkan installation. Try modifying the Vulkan_INSTALL_DIRS path.")
	else()
		message(WARNING "Cannot find Vulkan installation. Try modifying the Vulkan_INSTALL_DIRS path.")
	endif()
else()
	message(STATUS "...Vulkan OK.")
endif()

mark_as_advanced(Vulkan_INSTALL_DIRS Vulkan_INCLUDE_DIR Vulkan_LIBRARY_OPTIMIZED Vulkan_LIBRARY_DEBUG)
set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})
set(Vulkan_LIBRARIES_OPTIMIZED ${Vulkan_LIBRARY_OPTIMIZED})
set(Vulkan_LIBRARIES_DEBUG ${Vulkan_LIBRARY_DEBUG})