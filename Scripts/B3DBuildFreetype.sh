#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds Freetype from source"
echo ""

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned freetype version
FREETYPE_VERSION="VER-2-14-3"

# Clone from official GitLab
if [ -d "freetype" ]; then
	cd freetype
	git fetch --tags
	git checkout $FREETYPE_VERSION
else
	git clone https://gitlab.freedesktop.org/freetype/freetype.git freetype
	cd freetype
	git checkout $FREETYPE_VERSION
fi

# Apply the shared source patch (disables FreeType's env-driven property lookup, which depends on
# getenv() and is unused by Banshee). Applied on every platform. The --check guard keeps re-runs
# idempotent.
RootPatch="$CurrentDirectory/Patches/Freetype.patch"
if [ -f "$RootPatch" ]; then
	echo "Applying patch: $RootPatch"
	git apply --check "$RootPatch" 2>/dev/null && git apply "$RootPatch"
fi

# Setup output folders
OutputFolder="$PlatformDependencyFolder/freetype"
B3DCleanDependencyFolder "$OutputFolder"

# Configure (identical options across all platforms)
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DBUILD_SHARED_LIBS=OFF \
	-DFT_DISABLE_ZLIB=ON \
	-DFT_DISABLE_BZIP2=ON \
	-DFT_DISABLE_PNG=ON \
	-DFT_DISABLE_HARFBUZZ=ON \
	-DFT_DISABLE_BROTLI=ON \
	$B3D_EXTRA_CMAKE_ARGS || exit 1

# Build and install
cmake --build build --config Release --target install || exit 1
mkdir -p "$OutputFolder/lib/Release"
mv "$OutputFolder/lib/${StaticLibraryPrefix}freetype${StaticLibraryExtension}" "$OutputFolder/lib/Release/"

cmake --build build --config Debug --target install || exit 1
mkdir -p "$OutputFolder/lib/Debug"
mv "$OutputFolder/lib/${StaticLibraryPrefix}freetyped${StaticLibraryExtension}" "$OutputFolder/lib/Debug/${StaticLibraryPrefix}freetype${StaticLibraryExtension}"

# Flatten header layout: freetype installs under include/freetype2/, but Banshee expects headers
# (ft2build.h, freetype/*) directly under include/
if [ -d "$OutputFolder/include/freetype2" ]; then
	mv "$OutputFolder/include/freetype2/"* "$OutputFolder/include/"
	rmdir "$OutputFolder/include/freetype2"
fi

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "Freetype has been built and installed to:"
echo "  $OutputFolder"
echo ""
