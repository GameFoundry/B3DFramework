#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds OpenAL Soft from source"
echo ""

# On Linux, OpenAL Soft links against the system audio backends (PulseAudio, ALSA,
# JACK, OSS). These need to be installed before configuring so CMake can detect them.
if [[ "$Platform" == "linux-gnu"* ]]; then
	echo "Note: on Linux, install the backend -dev packages before building,"
	echo "e.g. Debian/Ubuntu: apt-get install libpulse-dev libasound2-dev libjack-dev"
	echo ""
fi

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned OpenAL Soft version
OPENAL_VERSION="openal-soft-1.17.2"

# Clone from kcat/openal-soft
if [ -d "openal-soft" ]; then
	cd openal-soft
	git fetch --tags
	git reset --hard $OPENAL_VERSION
else
	git clone https://github.com/kcat/openal-soft.git openal-soft
	cd openal-soft
	git checkout $OPENAL_VERSION
fi

# Apply patch. OpenAL Soft 1.17.2's CMakeLists.txt passes an inline -D flag as a
# positional argument to CHECK_INCLUDE_FILES, which CMake 3.12+ rejects.
echo "Applying OpenAL patch..."
git apply "$CurrentDirectory/Patches/OpenAL.patch" || exit 1

# Setup output folders
OutputFolder="$PlatformDependencyFolder/OpenAL"
B3DCleanDependencyFolder "$OutputFolder"

# Pick library type per platform. dependencies.md: dynamic on Windows/Linux, static
# on macOS.
if [[ "$Platform" == "darwin"* ]]; then
	LibType="STATIC"
else
	LibType="SHARED"
fi

# Configure. Disable tests, utils (openal-info, altonegen, ...) and examples; we
# only need the library itself. ALSOFT_CONFIG/ALSOFT_HRTF_DEFS/ALSOFT_AMBDEC_PRESETS
# install runtime data files we don't ship. POLICY_VERSION_MINIMUM needed because
# OpenAL Soft 1.17.2's cmake_minimum_required is older than CMake 4.x allows.
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
	-DLIBTYPE="$LibType" \
	-DALSOFT_TESTS=OFF \
	-DALSOFT_UTILS=OFF \
	-DALSOFT_EXAMPLES=OFF \
	-DALSOFT_CONFIG=OFF \
	-DALSOFT_HRTF_DEFS=OFF \
	-DALSOFT_AMBDEC_PRESETS=OFF || exit 1

# FindOpenAL.cmake doesn't differentiate Debug/Release (it looks up a single
# OpenAL32/openal target), so we only install Release to match the expected
# flat bin/ + lib/ layout.
cmake --build build --config Release --target install || exit 1

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "OpenAL Soft has been built and installed to:"
echo "  $OutputFolder"
echo ""
