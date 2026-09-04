#!/bin/sh

. ./B3DBuildCommon.sh

echo "Builds GameNetworkingSockets from source"
echo ""

# Check prerequisites
if ! command -v cmake &> /dev/null; then
    echo "[Error] CMake is not installed. Please install CMake 4.2 or later."
    exit 1
fi

# Check for Protobuf dependency
ProtobufDependencyFolder="$PlatformDependencyFolder/Protobuf"

if [ ! -d "$ProtobufDependencyFolder" ] || [ ! -d "$ProtobufDependencyFolder/include" ]; then
    echo ""
    echo "======================================================================"
    echo "Protobuf Required"
    echo "======================================================================"
    echo ""
    echo "GameNetworkingSockets requires Protocol Buffers (protobuf) to be built."
    echo ""
    echo "Would you like to build Protobuf now? (y/n)"
    read -r buildProtobuf

    if [[ "$buildProtobuf" == "y" || "$buildProtobuf" == "Y" ]]; then
        echo ""
        echo "Building Protobuf..."
        ./B3DBuildProtobuf.sh "$Platform" || exit 1
        echo ""
        echo "Protobuf build complete. Continuing with GameNetworkingSockets build..."
        echo ""
    else
        echo ""
        echo "[Error] Protobuf is required to build GameNetworkingSockets."
        echo "        Please build Protobuf first using: ./B3DBuildProtobuf.sh"
        exit 1
    fi
else
    echo "Using Protobuf from: $ProtobufDependencyFolder"
fi

# Add protoc to PATH for CMake to find it
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    export PATH="$PATH:$ProtobufDependencyFolder/bin/Release"
else
    export PATH="$PATH:$ProtobufDependencyFolder/bin"
fi

# Check for OpenSSL dependency
# GameNetworkingSockets requires OpenSSL. We check if OPENSSL_ROOT_DIR is set
# or if OpenSSL can be found in standard locations.

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    # On Windows, check if OPENSSL_ROOT_DIR is set or provide installation instructions
    if [ -z "$OPENSSL_ROOT_DIR" ]; then
        echo ""
        echo "======================================================================"
        echo "OpenSSL Required"
        echo "======================================================================"
        echo ""
        echo "GameNetworkingSockets requires OpenSSL to be installed."
        echo ""
        echo "To install OpenSSL on Windows:"
        echo "  1. Download the OpenSSL installer from:"
        echo "     https://slproweb.com/products/Win32OpenSSL.html"
        echo ""
        echo "  2. Choose the installer WITHOUT the 'Light' suffix"
        echo "     (e.g., 'Win64 OpenSSL v3.x.x' NOT 'Win64 OpenSSL v3.x.x Light')"
        echo ""
        echo "  3. Install to a location like: C:\\OpenSSL-Win64"
        echo ""
        echo "  4. Set the OPENSSL_ROOT_DIR environment variable:"
        echo "     set OPENSSL_ROOT_DIR=C:\\OpenSSL-Win64"
        echo ""
        echo "  5. Re-run this script"
        echo ""
        echo "Alternatively, you can install OpenSSL via vcpkg or build from source."
        echo ""
        echo "Do you want to continue anyway? CMake might find OpenSSL automatically. (y/n)"
        read -r continueAnyway

        if [[ "$continueAnyway" != "y" && "$continueAnyway" != "Y" ]]; then
            exit 1
        fi
    else
        echo "Using OpenSSL from: $OPENSSL_ROOT_DIR"
    fi
fi

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

GNS_VERSION="v1.4.1"

if [ -d "GameNetworkingSockets/.git" ]; then
    echo "GameNetworkingSockets repository exists, selecting pinned revision..."
    cd GameNetworkingSockets
    git fetch --tags
    git reset --hard
    git checkout --detach "$GNS_VERSION" || exit 1
    git submodule update --init --recursive
else
    echo "Cloning GameNetworkingSockets repository..."
    git clone https://github.com/ValveSoftware/GameNetworkingSockets.git --recursive GameNetworkingSockets
    cd GameNetworkingSockets
    git checkout --detach "$GNS_VERSION" || exit 1
fi

# Setup GameNetworkingSockets output folders
GNSOutputFolder="$PlatformDependencyFolder/GameNetworkingSockets"

echo "Output folder: $GNSOutputFolder"

B3DCleanDependencyFolder "$GNSOutputFolder"
mkdir -p "$GNSOutputFolder/include/"
mkdir -p "$GNSOutputFolder/lib/"
mkdir -p "$GNSOutputFolder/bin/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    mkdir -p "$GNSOutputFolder/lib/Release/"
    mkdir -p "$GNSOutputFolder/lib/Debug/"
    mkdir -p "$GNSOutputFolder/bin/Release/"
    mkdir -p "$GNSOutputFolder/bin/Debug/"
fi

# Create build directory
mkdir -p build
cd build

# Configure CMake
# GNS options:
# - GAMENETWORKINGSOCKETS_BUILD_TESTS=OFF: Don't build tests
# - GAMENETWORKINGSOCKETS_BUILD_EXAMPLES=OFF: Don't build examples
# - USE_STEAMWEBRTC=OFF: Don't require Steam WebRTC library
# - OPENSSL_ROOT_DIR: Path to OpenSSL installation (if set)
# - Protobuf_DIR: Path to Protobuf CMake config files
# - Protobuf_USE_STATIC_LIBS=ON: Link with static protobuf libraries
echo "Configuring CMake..."

# Accumulate dependency paths as positional arguments so spaces in paths (notably Program Files) remain
# part of a single CMake -D argument. Embedded quote characters in a word-split string are not reparsed by
# the shell and previously produced malformed cache values.
set -- -DProtobuf_USE_STATIC_LIBS=ON
if [ ! -z "$OPENSSL_ROOT_DIR" ]; then
    set -- "$@" "-DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR"
fi

# Set Protobuf paths for CMake
if [ -d "$ProtobufDependencyFolder" ]; then
    set -- "$@" "-DProtobuf_INCLUDE_DIR=$ProtobufDependencyFolder/include"

    if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
        # For multi-config generators on Windows, specify both Debug and Release libraries
        set -- "$@" "-DProtobuf_LIBRARY_RELEASE=$ProtobufDependencyFolder/lib/Release/libprotobuf.lib"
        set -- "$@" "-DProtobuf_LIBRARY_DEBUG=$ProtobufDependencyFolder/lib/Debug/libprotobufd.lib"
        set -- "$@" "-DProtobuf_LIBRARY=$ProtobufDependencyFolder/lib/Release/libprotobuf.lib"
        set -- "$@" "-DProtobuf_PROTOC_EXECUTABLE=$ProtobufDependencyFolder/bin/Release/protoc.exe"
    else
        set -- "$@" "-DProtobuf_LIBRARY=$ProtobufDependencyFolder/lib/${StaticLibraryPrefix}protobuf${StaticLibraryExtension}"
        set -- "$@" "-DProtobuf_PROTOC_EXECUTABLE=$ProtobufDependencyFolder/bin/protoc"
    fi
fi

cmake .. -G "$CMakeGenerator" \
    -DCMAKE_INSTALL_PREFIX="install" \
    -DGAMENETWORKINGSOCKETS_BUILD_TESTS=OFF \
    -DGAMENETWORKINGSOCKETS_BUILD_EXAMPLES=OFF \
    -DUSE_STEAMWEBRTC=OFF \
    "$@" || exit 1

# Build based on platform
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "Building Release configuration..."
    cmake --build . --config Release || exit 1

    echo "Building Debug configuration..."
    # Debug build might fail if protobuf isn't available in Debug, but static lib usually builds
    if ! cmake --build . --config Debug; then
        echo "Warning: Debug dynamic library build failed (likely due to missing Debug protobuf)."
        echo "Attempting to use the static library instead..."
    fi

    # Copy Release binaries
    echo "Copying Release binaries..."
    cp -p bin/Release/GameNetworkingSockets${SharedLibraryExtension} "$GNSOutputFolder/bin/Release/" 2>/dev/null || true
    cp -p src/Release/GameNetworkingSockets_s${StaticLibraryExtension} "$GNSOutputFolder/lib/Release/" 2>/dev/null || true
    cp -p src/Release/GameNetworkingSockets${ImportLibraryExtension} "$GNSOutputFolder/lib/Release/" 2>/dev/null || true

    # Copy Debug binaries
    echo "Copying Debug binaries..."
    cp -p bin/Debug/GameNetworkingSockets${SharedLibraryExtension} "$GNSOutputFolder/bin/Debug/" 2>/dev/null || true
    cp -p src/Debug/GameNetworkingSockets_s${StaticLibraryExtension} "$GNSOutputFolder/lib/Debug/" 2>/dev/null || true
    cp -p src/Debug/GameNetworkingSockets${ImportLibraryExtension} "$GNSOutputFolder/lib/Debug/" 2>/dev/null || true
    cp -p bin/Debug/GameNetworkingSockets.pdb "$GNSOutputFolder/bin/Debug/" 2>/dev/null || true
    cp -p src/Debug/GameNetworkingSockets_s.pdb "$GNSOutputFolder/lib/Debug/" 2>/dev/null || true

    # Copy includes
    echo "Copying headers..."
    cp -a ../include/steam "$GNSOutputFolder/include/" 2>/dev/null || true

else
    echo "Building Release configuration..."
    cmake --build . --config Release || exit 1

    # Copy Release binaries
    echo "Copying Release binaries..."
    cp -p Release/${StaticLibraryPrefix}GameNetworkingSockets${SharedLibraryExtension}* "$GNSOutputFolder/bin/" 2>/dev/null || true
    cp -p Release/${StaticLibraryPrefix}GameNetworkingSockets_s${StaticLibraryExtension} "$GNSOutputFolder/lib/" 2>/dev/null || true
    cp -p Release/${StaticLibraryPrefix}GameNetworkingSockets${StaticLibraryExtension} "$GNSOutputFolder/lib/" 2>/dev/null || true

    # Copy includes
    echo "Copying headers..."
    cp -a ../include/steam "$GNSOutputFolder/include/" 2>/dev/null || true
fi

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "GameNetworkingSockets has been built and installed to:"
echo "  $GNSOutputFolder"
echo ""
echo "Dependencies used:"
echo "  Protobuf: $ProtobufDependencyFolder"
if [ ! -z "$OPENSSL_ROOT_DIR" ]; then
    echo "  OpenSSL:  $OPENSSL_ROOT_DIR"
fi
echo ""
