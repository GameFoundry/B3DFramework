#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds glslang from source"
echo ""

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned glslang version (matches dependencies.md). Tag 15.4.0 corresponds to the glslang shipped with Vulkan SDK 1.4.321
GLSLANG_VERSION="15.4.0"

# Clone (or update) the official KhronosGroup repo and check out the pinned version. The hard reset
# before checkout discards any leftover modifications from a previous run so the checkout can't be
# blocked by a dirty working tree.
if [ -d "glslang/.git" ]; then
	cd glslang
	git fetch --tags --quiet
	git reset --hard --quiet
	git checkout --quiet "$GLSLANG_VERSION"
else
	git clone https://github.com/KhronosGroup/glslang.git glslang
	cd glslang
	git checkout --quiet "$GLSLANG_VERSION"
fi

# Fetch the matching SPIRV-Tools + SPIRV-Headers into External/ so glslang's CMake enables ENABLE_OPT
PythonExe=""
if command -v python >/dev/null 2>&1; then
	PythonExe="python"
elif command -v python3 >/dev/null 2>&1; then
	PythonExe="python3"
fi

if [ -z "$PythonExe" ]; then
	echo "[Error] Python is required to fetch and build SPIRV-Tools (spirv-opt) but was not found on PATH." 1>&2
	exit 1
fi

# Discard any local changes in previously-cloned External repos so update_glslang_sources.py can
# re-checkout the known-good commits cleanly across re-runs.
if [ -d External/spirv-tools/.git ]; then
	git -C External/spirv-tools reset --hard --quiet 2>/dev/null
	git -C External/spirv-tools clean -ffdq 2>/dev/null
fi

"$PythonExe" update_glslang_sources.py || exit 1

# Setup output folders
OutputFolder="$PlatformDependencyFolder/glslang"
B3DCleanDependencyFolder "$OutputFolder"

# Configure (identical options across all platforms). ENABLE_OPT=ON pulls in the bundled
# SPIRV-Tools (fetched above) so the spirv-opt optimizer is built and installed alongside
# glslang. SPIRV-Tools tests/executables are skipped to keep the build lean.
cmake -S . -B build -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$OutputFolder" \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DCMAKE_DEBUG_POSTFIX=d \
	-DBUILD_SHARED_LIBS=OFF \
	-DENABLE_GLSLANG_BINARIES=OFF \
	-DENABLE_OPT=ON \
	-DSPIRV_SKIP_TESTS=ON \
	-DSPIRV_SKIP_EXECUTABLES=ON \
	-DENABLE_HLSL=ON || exit 1

# Build and install. Modern glslang installs all headers under include/glslang/..., including the
# SPIR-V backend at include/glslang/SPIRV/...
cmake --build build --config Release --target install || exit 1
cmake --build build --config Debug --target install || exit 1

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "glslang has been built and installed to:"
echo "  $OutputFolder"
echo ""
