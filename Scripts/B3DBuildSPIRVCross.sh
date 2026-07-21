#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds SPIRV-Cross from source"
echo ""

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned SPIRV-Cross version. Matches Vulkan SDK 1.4.321, same as glslang 15.4.0 in B3DBuildGlslang.sh.
SPIRV_CROSS_VERSION="vulkan-sdk-1.4.321.0"

# Clone from official KhronosGroup repo
if [ -d "SPIRV-Cross" ]; then
	cd SPIRV-Cross
	git fetch --tags
	git checkout $SPIRV_CROSS_VERSION
else
	git clone https://github.com/KhronosGroup/SPIRV-Cross.git SPIRV-Cross
	cd SPIRV-Cross
	git checkout $SPIRV_CROSS_VERSION
fi

# Setup output folders
OutputFolder="$PlatformDependencyFolder/SPIRVCross"
B3DCleanDependencyFolder "$OutputFolder"

# Configure. Static libraries only; skip the CLI tool and tests. CMAKE_DEBUG_POSTFIX=d
# lets both configs coexist in a flat lib/ folder. Exceptions become assertions since bsf builds with -fno-exceptions.
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DCMAKE_DEBUG_POSTFIX=d \
	-DSPIRV_CROSS_SHARED=OFF \
	-DSPIRV_CROSS_STATIC=ON \
	-DSPIRV_CROSS_CLI=OFF \
	-DSPIRV_CROSS_ENABLE_TESTS=OFF \
	-DSPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=ON || exit 1

# Build and install both configurations into the same flat lib/ folder
cmake --build build --config Release --target install || exit 1
cmake --build build --config Debug --target install || exit 1

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "SPIRV-Cross has been built and installed to:"
echo "  $OutputFolder"
echo ""
