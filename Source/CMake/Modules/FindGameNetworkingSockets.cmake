# Find GameNetworkingSockets
#
# This module defines
#  GameNetworkingSockets_INCLUDE_DIRS
#  GameNetworkingSockets_LIBRARIES
#  GameNetworkingSockets_FOUND

B3DStartFindPackage(GameNetworkingSockets)

set(GameNetworkingSockets_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/GameNetworkingSockets)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT GameNetworkingSockets_INSTALL_DIR)
	set(GameNetworkingSockets_INSTALL_DIR ${GameNetworkingSockets_BUNDLED_INSTALL_DIR} CACHE PATH "Path to GameNetworkingSockets dependency" FORCE)
endif()

# Ensure the bundled copy is up to date, building it from source if no prebuilt package is available
B3DEnsureBundledDependency(GameNetworkingSockets BUILD_SCRIPT B3DBuildGameNetworkingSockets.sh)

B3DPopulateDefaultPackageSearchPaths(GameNetworkingSockets)

B3DFindImportedIncludes(GameNetworkingSockets steam/steamnetworkingsockets.h)
B3DFindImportedLibrary(GameNetworkingSockets GameNetworkingSockets SHARED)

B3DEndFindPackage(GameNetworkingSockets GameNetworkingSockets)
