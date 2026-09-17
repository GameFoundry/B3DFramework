#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds libFLAC from source"
echo ""

# libFLAC depends on libogg. Make sure it was built first.
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

# Pinned libFLAC version
FLAC_VERSION="1.5.0"

# Clone from official Xiph repo
if [ -d "flac" ]; then
	cd flac
	git fetch --tags
	git checkout $FLAC_VERSION
else
	git clone https://github.com/xiph/flac.git flac
	cd flac
	git checkout $FLAC_VERSION
fi

# Setup output folders
OutputFolder="$PlatformDependencyFolder/libFLAC"
B3DCleanDependencyFolder "$OutputFolder"

# Configure. CMAKE_PREFIX_PATH lets find_package(Ogg) locate our libogg install
# via its OggConfig.cmake. CMAKE_DEBUG_POSTFIX=d lets both configs coexist in a
# flat lib/ folder. We only need the C library; skip C++ bindings, tools,
# examples, tests, and docs.
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_PREFIX_PATH="$OggOutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DCMAKE_DEBUG_POSTFIX=d \
	-DBUILD_SHARED_LIBS=OFF \
	-DBUILD_CXXLIBS=OFF \
	-DBUILD_PROGRAMS=OFF \
	-DBUILD_EXAMPLES=OFF \
	-DBUILD_TESTING=OFF \
	-DBUILD_DOCS=OFF \
	-DINSTALL_MANPAGES=OFF \
	-DWITH_FORTIFY_SOURCE=OFF \
	-DWITH_STACK_PROTECTOR=OFF || exit 1

# Build and install both configurations into the same flat lib/ folder
cmake --build build --config Release --target install || exit 1
cmake --build build --config Debug --target install || exit 1

B3DStampDependencyBuild "$OutputFolder"

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "libFLAC has been built and installed to:"
echo "  $OutputFolder"
echo ""
