#!/bin/sh

. ./B3DBuildCommon.sh

echo "Builds Slang shader compiler from source"
echo ""

# Check prerequisites
if ! command -v cmake &> /dev/null; then
    echo "[Error] CMake is not installed. Please install CMake 4.2 or later."
    exit 1
fi

if ! command -v python3 &> /dev/null && ! command -v python &> /dev/null; then
    echo "[Error] Python 3 is not installed. Please install Python 3."
    exit 1
fi

# Platform-specific information
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "Building for Windows."
elif [[ "$Platform" == "darwin"* ]]; then
    echo "Building for macOS."
    CMakePreset="default"
elif [[ "$Platform" == "linux-gnu"* ]]; then
    echo "Building for Linux."
    CMakePreset="default"
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

SLANG_VERSION="v2025.21.2"

if [ -d "slang/.git" ]; then
    echo "Slang repository exists, selecting pinned revision..."
    cd slang
    git fetch --tags
    git reset --hard
    git checkout --detach "$SLANG_VERSION" || exit 1
    git submodule update --init --recursive
else
    echo "Cloning Slang repository..."
    git clone https://github.com/shader-slang/slang.git --recursive slang
    cd slang
    git checkout --detach "$SLANG_VERSION" || exit 1
fi

# Setup Slang output folders
SlangOutputFolder="$PlatformDependencyFolder/Slang"

echo "Output folder: $SlangOutputFolder"

rm -rf "$SlangOutputFolder"
mkdir -p "$SlangOutputFolder/include/"
mkdir -p "$SlangOutputFolder/lib/"
mkdir -p "$SlangOutputFolder/bin/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    mkdir -p "$SlangOutputFolder/lib/Release/"
    mkdir -p "$SlangOutputFolder/lib/Debug/"
    mkdir -p "$SlangOutputFolder/bin/Release/"
    mkdir -p "$SlangOutputFolder/bin/Debug/"
fi

# Slang v2025.21.2 predates a VS2026 preset. Reproduce its upstream MSVC preset options with the selected
# generator so the source checkout remains pristine and generator overrides continue to work.
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "Configuring CMake with generator: $CMakeGenerator"
    cmake -S . -B build -G "$CMakeGenerator" \
        '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' \
        '-DSLANG_ENABLE_IR_BREAK_ALLOC=$<$<CONFIG:Debug>:TRUE>$<$<NOT:$<CONFIG:Debug>>:FALSE>' \
        '-DCMAKE_CONFIGURATION_TYPES=Debug;Release;RelWithDebInfo;MinSizeRel' \
        '-DCMAKE_C_FLAGS_INIT=-D_ITERATOR_DEBUG_LEVEL=0 /MP' \
        '-DCMAKE_CXX_FLAGS_INIT=-D_ITERATOR_DEBUG_LEVEL=0 /MP' \
        -DSLANG_ENABLE_TESTS=OFF \
        -DSLANG_ENABLE_EXAMPLES=OFF \
        -DSLANG_ENABLE_SLANG_RHI=OFF || exit 1
else
    echo "Configuring CMake with preset: $CMakePreset"
    cmake --preset "$CMakePreset" || exit 1
fi

# Build based on platform
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "Building Release configuration..."
    cmake --build build --config Release || exit 1

    echo "Building Debug configuration..."
    cmake --build build --config Debug || exit 1

    # Copy Release binaries
    echo "Copying Release binaries..."
    cp -p build/Release/bin/slang-compiler${SharedLibraryExtension} "$SlangOutputFolder/bin/Release/" 2>/dev/null || \
        cp -p build/Release/bin/slang${SharedLibraryExtension} "$SlangOutputFolder/bin/Release/"
    cp -p build/Release/bin/slang.exe "$SlangOutputFolder/bin/Release/" 2>/dev/null || true
    cp -p build/Release/bin/slangc.exe "$SlangOutputFolder/bin/Release/" 2>/dev/null || true
    cp -p build/Release/lib/slang-compiler${ImportLibraryExtension} "$SlangOutputFolder/lib/Release/" 2>/dev/null || \
        cp -p build/Release/lib/slang${ImportLibraryExtension} "$SlangOutputFolder/lib/Release/" 2>/dev/null || true

    # Copy Debug binaries
    echo "Copying Debug binaries..."
    cp -p build/Debug/bin/slang-compiler${SharedLibraryExtension} "$SlangOutputFolder/bin/Debug/" 2>/dev/null || \
        cp -p build/Debug/bin/slang${SharedLibraryExtension} "$SlangOutputFolder/bin/Debug/"
    cp -p build/Debug/bin/slang.exe "$SlangOutputFolder/bin/Debug/" 2>/dev/null || true
    cp -p build/Debug/bin/slangc.exe "$SlangOutputFolder/bin/Debug/" 2>/dev/null || true
    cp -p build/Debug/lib/slang-compiler${ImportLibraryExtension} "$SlangOutputFolder/lib/Debug/" 2>/dev/null || \
        cp -p build/Debug/lib/slang${ImportLibraryExtension} "$SlangOutputFolder/lib/Debug/" 2>/dev/null || true
    cp -p build/Debug/bin/slang-compiler.pdb "$SlangOutputFolder/bin/Debug/" 2>/dev/null || \
        cp -p build/Debug/bin/slang.pdb "$SlangOutputFolder/bin/Debug/" 2>/dev/null || true

    # Copy includes
    echo "Copying headers..."
    cp -a include/*.h "$SlangOutputFolder/include/" 2>/dev/null || true
    cp -a build/Release/include/slang-tag-version.h "$SlangOutputFolder/include/" 2>/dev/null || true

else
    echo "Building Release configuration..."
    cmake --build build --config Release || exit 1

    # Copy Release binaries
    echo "Copying Release binaries..."
    cp -p build/Release/bin/${StaticLibraryPrefix}slang-compiler${SharedLibraryExtension}* "$SlangOutputFolder/bin/" 2>/dev/null || \
        cp -p build/Release/bin/${StaticLibraryPrefix}slang${SharedLibraryExtension}* "$SlangOutputFolder/bin/" || true
    cp -p build/Release/bin/slang "$SlangOutputFolder/bin/" 2>/dev/null || true
    cp -p build/Release/bin/slangc "$SlangOutputFolder/bin/" 2>/dev/null || true
    cp -p build/Release/lib/${StaticLibraryPrefix}slang-compiler${StaticLibraryExtension} "$SlangOutputFolder/lib/" 2>/dev/null || \
        cp -p build/Release/lib/${StaticLibraryPrefix}slang${StaticLibraryExtension} "$SlangOutputFolder/lib/" 2>/dev/null || true

    # Copy includes
    echo "Copying headers..."
    cp -a include/*.h "$SlangOutputFolder/include/" 2>/dev/null || true
    cp -a build/Release/include/slang-tag-version.h "$SlangOutputFolder/include/" 2>/dev/null || true
fi

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "Slang has been built and installed to:"
echo "  $SlangOutputFolder"
