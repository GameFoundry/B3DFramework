# Find MoltenVK installation
#
# MoltenVK ships as part of the LunarG Vulkan SDK. Located through the VULKAN_SDK environment variable (pointing at
# the SDK's macOS folder), or through the SDK's system-global install in /usr/local.
#
# This module defines
#  MoltenVK_INCLUDE_DIRS
#  MoltenVK_LIBRARIES
#  MoltenVK_FOUND

B3DStartFindPackage(MoltenVK)

# Prefer the SDK pointed to by VULKAN_SDK (accepting either the macOS folder or the version root), then the system-global install
if(DEFINED ENV{VULKAN_SDK} AND IS_DIRECTORY "$ENV{VULKAN_SDK}/lib/MoltenVK.xcframework")
	set(mvkDefaultInstallFolder "$ENV{VULKAN_SDK}")
elseif(DEFINED ENV{VULKAN_SDK} AND IS_DIRECTORY "$ENV{VULKAN_SDK}/macOS/lib/MoltenVK.xcframework")
	set(mvkDefaultInstallFolder "$ENV{VULKAN_SDK}/macOS")
else()
	set(mvkDefaultInstallFolder "/usr/local")
endif()

set(MoltenVK_INSTALL_DIR "${mvkDefaultInstallFolder}" CACHE PATH "Path to the MoltenVK installation (part of the Vulkan SDK)")
B3DPopulateDefaultPackageSearchPaths(MoltenVK)

# The static library lives inside the SDK's xcframework. Searched first, so the shared library in lib/ doesn't win.
list(INSERT MoltenVK_LIBRARY_RELEASE_SEARCH_DIRS 0 "${MoltenVK_INSTALL_DIR}/lib/MoltenVK.xcframework/macos-arm64_x86_64")
list(INSERT MoltenVK_LIBRARY_DEBUG_SEARCH_DIRS 0 "${MoltenVK_INSTALL_DIR}/lib/MoltenVK.xcframework/macos-arm64_x86_64")

B3DFindImportedIncludes(MoltenVK MoltenVK/mvk_vulkan.h)
B3DFindImportedLibrary(MoltenVK MoltenVK STATIC)

if(NOT MoltenVK_FOUND AND NOT MoltenVK_FIND_QUIETLY)
	message(WARNING "Cannot find MoltenVK. It is included with the Vulkan SDK (https://vulkan.lunarg.com) - install the SDK and either run its system-global install or point the VULKAN_SDK environment variable at the SDK's macOS folder.")
	set(MoltenVK_FIND_QUIETLY TRUE) # Already warned above, skip the generic message
endif()

B3DEndFindPackage(MoltenVK MoltenVK)
