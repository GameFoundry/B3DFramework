#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds libvorbis from source"
echo ""

# libvorbis depends on libogg. Make sure it was built first.
OggOutputFolder="$PlatformDependencyFolder/libogg"
if [ ! -f "$OggOutputFolder/lib/cmake/Ogg/OggConfig.cmake" ]; then
	echo "[Error] libogg is required. Build it first by running:"
	echo "  ./B3DBuildOgg.sh"
	exit 1
fi

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned libvorbis version
VORBIS_VERSION="v1.3.7"

# Clone from official Xiph repo
if [ -d "vorbis" ]; then
	cd vorbis
	git fetch --tags
	git checkout $VORBIS_VERSION
else
	git clone https://github.com/xiph/vorbis.git vorbis
	cd vorbis
	git checkout $VORBIS_VERSION
fi

# Setup output folders
OutputFolder="$PlatformDependencyFolder/libvorbis"
B3DCleanDependencyFolder "$OutputFolder"

# Configure. CMAKE_PREFIX_PATH lets find_package(Ogg) locate our libogg install
# via its OggConfig.cmake. CMAKE_DEBUG_POSTFIX=d lets both configs coexist in a
# flat lib/ folder. POLICY_VERSION_MINIMUM needed because libvorbis's
# cmake_minimum_required is older than CMake 4.x allows.
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_PREFIX_PATH="$OggOutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DCMAKE_DEBUG_POSTFIX=d \
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
	-DBUILD_SHARED_LIBS=OFF || exit 1

# Build and install both configurations into the same flat lib/ folder
cmake --build build --config Release --target install || exit 1
cmake --build build --config Debug --target install || exit 1

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "libvorbis has been built and installed to:"
echo "  $OutputFolder"
echo ""
