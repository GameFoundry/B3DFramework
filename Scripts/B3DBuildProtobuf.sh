#!/bin/sh

. ./B3DBuildCommon.sh

echo "Builds Protocol Buffers (protobuf) from source"
echo ""

# Check prerequisites
if ! command -v cmake &> /dev/null; then
    echo "[Error] CMake is not installed. Please install CMake 4.2 or later."
    exit 1
fi

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

PROTOBUF_VERSION="v21.12"

if [ -d "protobuf/.git" ]; then
    echo "Protobuf repository exists, selecting pinned revision..."
    cd protobuf
    git fetch --tags
    git reset --hard
    git checkout --detach "$PROTOBUF_VERSION" || exit 1
    git submodule update --init --recursive
else
    echo "Cloning Protobuf repository..."
    git clone https://github.com/protocolbuffers/protobuf.git --recursive protobuf
    cd protobuf
    git checkout --detach "$PROTOBUF_VERSION" || exit 1
fi

# Setup Protobuf output folders
ProtobufOutputFolder="$PlatformDependencyFolder/Protobuf"

echo "Output folder: $ProtobufOutputFolder"

B3DCleanDependencyFolder "$ProtobufOutputFolder"
mkdir -p "$ProtobufOutputFolder/include/"
mkdir -p "$ProtobufOutputFolder/lib/"
mkdir -p "$ProtobufOutputFolder/bin/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    mkdir -p "$ProtobufOutputFolder/lib/Release/"
    mkdir -p "$ProtobufOutputFolder/lib/Debug/"
    mkdir -p "$ProtobufOutputFolder/bin/Release/"
    mkdir -p "$ProtobufOutputFolder/bin/Debug/"
fi

# Create build directory
mkdir -p cmake_build
cd cmake_build

# Configure CMake
# Protobuf options:
# - protobuf_BUILD_TESTS=OFF: Don't build tests
# - protobuf_BUILD_SHARED_LIBS=OFF: Build static libraries
# - protobuf_MSVC_STATIC_RUNTIME=OFF: Use dynamic MSVC runtime (default)
echo "Configuring CMake..."

cmake ../cmake -G "$CMakeGenerator" \
    -DCMAKE_INSTALL_PREFIX="install" \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_SHARED_LIBS=OFF \
    -Dprotobuf_MSVC_STATIC_RUNTIME=OFF || exit 1

# Build based on platform
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "Building Release configuration..."
    cmake --build . --config Release || exit 1

    echo "Building Debug configuration..."
    cmake --build . --config Debug || exit 1

    # Install to temporary location
    echo "Installing Release build..."
    cmake --install . --config Release || exit 1

    echo "Installing Debug build..."
    cmake --install . --config Debug || exit 1

    # Copy Release binaries
    echo "Copying Release binaries..."
    cp -p install/bin/protoc.exe "$ProtobufOutputFolder/bin/Release/" 2>/dev/null || true
    cp -p install/lib/*.lib "$ProtobufOutputFolder/lib/Release/" 2>/dev/null || true

    # Copy Debug binaries
    # Note: Debug protoc.exe is typically in Debug/ subdirectory
    echo "Copying Debug binaries..."
    cp -p Debug/protoc.exe "$ProtobufOutputFolder/bin/Debug/" 2>/dev/null || true
    cp -p Debug/*.lib "$ProtobufOutputFolder/lib/Debug/" 2>/dev/null || true
    cp -p Debug/*.pdb "$ProtobufOutputFolder/bin/Debug/" 2>/dev/null || true

    # Copy includes
    echo "Copying headers..."
    cp -a install/include/google "$ProtobufOutputFolder/include/" 2>/dev/null || true

else
    echo "Building Release configuration..."
    cmake --build . --config Release || exit 1

    # Install to temporary location
    echo "Installing Release build..."
    cmake --install . --config Release || exit 1

    # Copy Release binaries
    echo "Copying Release binaries..."
    cp -p install/bin/protoc "$ProtobufOutputFolder/bin/" 2>/dev/null || true
    cp -p install/lib/${StaticLibraryPrefix}protobuf${StaticLibraryExtension} "$ProtobufOutputFolder/lib/" 2>/dev/null || true
    cp -p install/lib/${StaticLibraryPrefix}protoc${StaticLibraryExtension} "$ProtobufOutputFolder/lib/" 2>/dev/null || true

    # Copy includes
    echo "Copying headers..."
    cp -a install/include/google "$ProtobufOutputFolder/include/" 2>/dev/null || true
fi

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "Protocol Buffers has been built and installed to:"
echo "  $ProtobufOutputFolder"
echo ""
echo "Important: Make sure to add the protoc compiler to your PATH:"
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "  set PATH=%PATH%;$ProtobufOutputFolder\\bin\\Release"
else
    echo "  export PATH=\$PATH:$ProtobufOutputFolder/bin"
fi
echo ""
echo "The protoc compiler is required by GameNetworkingSockets during the"
echo "CMake configuration step."
echo ""
