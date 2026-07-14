#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds Snappy from source"
echo ""

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned snappy version
SNAPPY_VERSION="1.2.2"

# Clone from official Google repo
if [ -d "snappy" ]; then
	cd snappy
	git fetch --tags
	git checkout $SNAPPY_VERSION
else
	git clone https://github.com/google/snappy.git snappy
	cd snappy
	git checkout $SNAPPY_VERSION
fi

# Setup output folders
OutputFolder="$PlatformDependencyFolder/snappy"
B3DCleanDependencyFolder "$OutputFolder"

# Configure
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DBUILD_SHARED_LIBS=OFF \
	-DSNAPPY_BUILD_TESTS=OFF \
	-DSNAPPY_BUILD_BENCHMARKS=OFF \
	-DSNAPPY_INSTALL=ON \
	$B3D_EXTRA_CMAKE_ARGS || exit 1

# Build and install
cmake --build build --config Release --target install || exit 1
mkdir -p "$OutputFolder/lib/Release"
mv "$OutputFolder/lib/${StaticLibraryPrefix}snappy${StaticLibraryExtension}" "$OutputFolder/lib/Release/"

cmake --build build --config Debug --target install || exit 1
mkdir -p "$OutputFolder/lib/Debug"
mv "$OutputFolder/lib/${StaticLibraryPrefix}snappy${StaticLibraryExtension}" "$OutputFolder/lib/Debug/"

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "Snappy has been built and installed to:"
echo "  $OutputFolder"
echo ""
