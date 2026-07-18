#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds libogg from source"
echo ""

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned libogg version
OGG_VERSION="v1.3.5"

# Clone from official Xiph repo
if [ -d "ogg" ]; then
	cd ogg
	git fetch --tags
	git checkout $OGG_VERSION
else
	git clone https://github.com/xiph/ogg.git ogg
	cd ogg
	git checkout $OGG_VERSION
fi

# Setup output folders
OutputFolder="$PlatformDependencyFolder/libogg"
B3DCleanDependencyFolder "$OutputFolder"

# POLICY_VERSION_MINIMUM needed because libogg's cmake_minimum_required is older than CMake 4.x allows
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DCMAKE_DEBUG_POSTFIX=d \
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
	-DBUILD_SHARED_LIBS=OFF \
	-DBUILD_TESTING=OFF || exit 1

# Build and install both configurations into the same flat lib/ folder
cmake --build build --config Release --target install || exit 1
cmake --build build --config Debug --target install || exit 1

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "libogg has been built and installed to:"
echo "  $OutputFolder"
echo ""
