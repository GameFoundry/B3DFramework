#!/bin/sh

. ./B3DBuildCommon.sh

echo "Builds B3DShaderCompiler (XShaderCompiler) from source"
echo ""

# Check prerequisites
if ! command -v cmake &> /dev/null; then
	echo "[Error] CMake is not installed. Please install CMake 2.8 or later."
	exit 1
fi

# Platform-specific information
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	echo "Building for Windows."
	CMakeGenerator="-G \"Visual Studio 17 2022\" -A x64"
elif [[ "$Platform" == "darwin"* ]]; then
	echo "Building for macOS."
	CMakeGenerator="-G \"Unix Makefiles\""
elif [[ "$Platform" == "linux-gnu"* ]]; then
	echo "Building for Linux."
	CMakeGenerator="-G \"Unix Makefiles\""
else
	echo "[Error] This build script is not currently supported on the current platform: $Platform."
	exit 1
fi

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Clone or update B3DShaderCompiler repository
if [ -d "B3DShaderCompiler" ]; then
	echo "B3DShaderCompiler repository exists, updating..."
	cd B3DShaderCompiler
	git stash
	git fetch
	git pull origin master
	git stash pop
else
	echo "Cloning B3DShaderCompiler repository..."
	git clone https://github.com/BearishSun/B3DShaderCompiler.git B3DShaderCompiler
	cd B3DShaderCompiler
fi

# Setup B3DShaderCompiler output folders
ShaderCompilerOutputFolder="$PlatformDependencyFolder/XShaderCompiler"

echo "Output folder: $ShaderCompilerOutputFolder"

# Read existing version before deleting the folder
VersionFile="$ShaderCompilerOutputFolder/.version"
if [ -f "$VersionFile" ]; then
	CurrentVersion=$(cat "$VersionFile")
	NewVersion=$((CurrentVersion + 1))
else
	NewVersion=0
fi

rm -rf "$ShaderCompilerOutputFolder"
mkdir -p "$ShaderCompilerOutputFolder/include/"
mkdir -p "$ShaderCompilerOutputFolder/lib/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	mkdir -p "$ShaderCompilerOutputFolder/lib/Release/"
	mkdir -p "$ShaderCompilerOutputFolder/lib/Debug/"
fi

# Create build directory
rm -rf cmake_build
mkdir -p cmake_build
cd cmake_build

# Configure CMake
# B3DShaderCompiler options:
# - XSC_BUILD_SHELL=OFF: Don't build shell application
# - XSC_BUILD_DEBUGGER=OFF: Don't build debugger
# - XSC_BUILD_TESTS=OFF: Don't build tests
# - XSC_SHARED_LIB=OFF: Build static library
echo "Configuring CMake..."

eval cmake .. $CMakeGenerator \
	-DINSTALL_OUTPUT_PATH="$ShaderCompilerOutputFolder" \
	-DXSC_BUILD_SHELL=OFF \
	-DXSC_BUILD_DEBUGGER=OFF \
	-DXSC_BUILD_TESTS=OFF \
	-DXSC_SHARED_LIB=OFF || exit 1

# Build based on platform
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	# Build Debug (RelWithDebInfo) first so PDB exists for install step
	echo "Building Debug (RelWithDebInfo) configuration..."
	cmake --build . --config RelWithDebInfo || exit 1
	cmake --build . --config RelWithDebInfo --target install || exit 1

	# Move Debug binaries
	echo "Moving Debug binaries..."
	mv "$ShaderCompilerOutputFolder/xsc_core${StaticLibraryExtension}" "$ShaderCompilerOutputFolder/lib/Debug/xsc_core${StaticLibraryExtension}" 2>/dev/null || true
	mv "$ShaderCompilerOutputFolder/xsc_core.pdb" "$ShaderCompilerOutputFolder/lib/Debug/xsc_core.pdb" 2>/dev/null || true

	# Build Release
	echo "Building Release configuration..."
	cmake --build . --config Release || exit 1
	cmake --build . --config Release --target install || exit 1

	# Move Release binaries
	echo "Moving Release binaries..."
	mv "$ShaderCompilerOutputFolder/xsc_core${StaticLibraryExtension}" "$ShaderCompilerOutputFolder/lib/Release/xsc_core${StaticLibraryExtension}" 2>/dev/null || true

else
	echo "Building Release configuration..."
	cmake --build . --config Release || exit 1
	cmake --build . --config Release --target install || exit 1

	# Move Release binaries
	echo "Moving Release binaries..."
	mv "$ShaderCompilerOutputFolder/${StaticLibraryPrefix}xsc_core${StaticLibraryExtension}" "$ShaderCompilerOutputFolder/lib/${StaticLibraryPrefix}xsc_core${StaticLibraryExtension}" 2>/dev/null || true
fi

# Write version file to prevent the build system from thinking the dependency is out of date
echo "$NewVersion" > "$ShaderCompilerOutputFolder/.version"

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "B3DShaderCompiler has been built and installed to:"
echo "  $ShaderCompilerOutputFolder"
echo ""
echo "Headers location: $ShaderCompilerOutputFolder/include/Xsc"
echo "Library location: $ShaderCompilerOutputFolder/lib"
echo ""
