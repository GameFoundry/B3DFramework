#!/bin/sh

. ./B3DBuildCommon.sh

echo "Builds METIS (Graph Partitioning and Fill-reducing Matrix Ordering) from source"
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

# First, build GKlib (METIS dependency)
echo ""
echo "======================================================================"
echo "Building GKlib (METIS dependency)"
echo "======================================================================"
echo ""

GKLIB_VERSION="b8b8bdc7e0186f1f36412c74e89153feecb5d436"

if [ -d "GKlib/.git" ]; then
    echo "GKlib repository exists, selecting pinned revision..."
    cd GKlib
    git fetch --tags
    git reset --hard
    git checkout --detach "$GKLIB_VERSION" || exit 1
else
    echo "Cloning GKlib repository..."
    git clone https://github.com/KarypisLab/GKlib.git GKlib
    cd GKlib
    git checkout --detach "$GKLIB_VERSION" || exit 1
fi

# Apply patch to fix Windows include paths
echo "Applying GKlib patch..."
git apply "$CurrentDirectory/Patches/GKlib.patch" || exit 1

# Setup GKlib output folders
GKlibOutputFolder="$PlatformDependencyFolder/GKlib"

echo "GKlib output folder: $GKlibOutputFolder"

B3DCleanDependencyFolder "$GKlibOutputFolder"
mkdir -p "$GKlibOutputFolder/include/"
mkdir -p "$GKlibOutputFolder/lib/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    mkdir -p "$GKlibOutputFolder/lib/Release/"
    mkdir -p "$GKlibOutputFolder/lib/Debug/"
fi

# Build GKlib
mkdir -p build
cd build

echo "Configuring GKlib CMake..."
# GKlib options:
# - GKLIB_BUILD_APPS=OFF: Don't build GKlib applications (they have Windows compatibility issues)
cmake .. -G "$CMakeGenerator" \
    -DCMAKE_INSTALL_PREFIX="install" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DGKLIB_BUILD_APPS=OFF || exit 1

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "Building GKlib Release configuration..."
    cmake --build . --config Release || exit 1

    echo "Building GKlib Debug configuration..."
    cmake --build . --config Debug || exit 1

    # Copy Release binaries
    echo "Copying GKlib Release binaries..."
    cp -p Release/GKlib${StaticLibraryExtension} "$GKlibOutputFolder/lib/Release/" 2>/dev/null || true

    # Copy Debug binaries
    echo "Copying GKlib Debug binaries..."
    cp -p Debug/GKlib${StaticLibraryExtension} "$GKlibOutputFolder/lib/Debug/" 2>/dev/null || true
    cp -p Debug/GKlib.pdb "$GKlibOutputFolder/lib/Debug/" 2>/dev/null || true
else
    echo "Building GKlib Release configuration..."
    cmake --build . --config Release || exit 1

    # Copy Release binaries
    echo "Copying GKlib Release binaries..."
    cp -p Release/${StaticLibraryPrefix}GKlib${StaticLibraryExtension} "$GKlibOutputFolder/lib/" 2>/dev/null || true
fi

# Copy GKlib includes
echo "Copying GKlib headers..."
cp -a ../include/* "$GKlibOutputFolder/include/" 2>/dev/null || true

echo "GKlib build complete!"

# Now build METIS
cd ../../

echo ""
echo "======================================================================"
echo "Building METIS"
echo "======================================================================"
echo ""

METIS_VERSION="v5.2.1"

if [ -d "METIS/.git" ]; then
    echo "METIS repository exists, selecting pinned revision..."
    cd METIS
    git fetch --tags
    git reset --hard
    git checkout --detach "$METIS_VERSION" || exit 1
else
    echo "Cloning METIS repository..."
    git clone https://github.com/KarypisLab/METIS.git METIS
    cd METIS
    git checkout --detach "$METIS_VERSION" || exit 1
fi

# Apply patch to fix Windows compilation (disable programs)
echo "Applying METIS patch..."
git apply "$CurrentDirectory/Patches/METIS.patch" || exit 1

# Setup METIS output folders
MetisOutputFolder="$PlatformDependencyFolder/Metis"

echo "METIS output folder: $MetisOutputFolder"

B3DCleanDependencyFolder "$MetisOutputFolder"
mkdir -p "$MetisOutputFolder/include/"
mkdir -p "$MetisOutputFolder/lib/"
mkdir -p "$MetisOutputFolder/bin/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    mkdir -p "$MetisOutputFolder/lib/Release/"
    mkdir -p "$MetisOutputFolder/lib/Debug/"
    mkdir -p "$MetisOutputFolder/bin/Release/"
    mkdir -p "$MetisOutputFolder/bin/Debug/"
fi

# METIS requires a configuration step to generate build/xinclude/metis.h
# This is normally done via 'make config', but we'll do it manually
echo "Configuring METIS..."

# Create build and xinclude directories
mkdir -p build
mkdir -p build/xinclude

# Generate metis.h with width configuration
echo "#define IDXTYPEWIDTH 32" > build/xinclude/metis.h
echo "#define REALTYPEWIDTH 32" >> build/xinclude/metis.h
cat include/metis.h >> build/xinclude/metis.h

# Copy CMakeLists.txt for xinclude
cp include/CMakeLists.txt build/xinclude/

# Now configure with CMake
cd build

# METIS CMake options:
# - GKLIB_PATH: Path to GKlib installation
# - SHARED: Build shared library (we build static by default)
echo "Configuring METIS CMake..."

cmake .. -G "$CMakeGenerator" \
    -DCMAKE_INSTALL_PREFIX="install" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DGKLIB_PATH="$GKlibOutputFolder" \
    -DSHARED=OFF || exit 1

# Build based on platform
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    echo "Building METIS Release configuration..."
    cmake --build . --config Release || exit 1

    echo "Building METIS Debug configuration..."
    cmake --build . --config Debug || exit 1

    # Copy Release binaries
    echo "Copying METIS Release binaries..."
    cp -p libmetis/Release/metis${StaticLibraryExtension} "$MetisOutputFolder/lib/Release/" 2>/dev/null || true
    cp -p programs/Release/*.exe "$MetisOutputFolder/bin/Release/" 2>/dev/null || true

    # Copy Debug binaries
    echo "Copying METIS Debug binaries..."
    cp -p libmetis/Debug/metis${StaticLibraryExtension} "$MetisOutputFolder/lib/Debug/" 2>/dev/null || true
    cp -p libmetis/Debug/metis.pdb "$MetisOutputFolder/lib/Debug/" 2>/dev/null || true
    cp -p programs/Debug/*.exe "$MetisOutputFolder/bin/Debug/" 2>/dev/null || true
    cp -p programs/Debug/*.pdb "$MetisOutputFolder/bin/Debug/" 2>/dev/null || true

else
    echo "Building METIS Release configuration..."
    cmake --build . --config Release || exit 1

    # Copy Release binaries
    echo "Copying METIS Release binaries..."
    cp -p libmetis/Release/${StaticLibraryPrefix}metis${StaticLibraryExtension} "$MetisOutputFolder/lib/" 2>/dev/null || true
    cp -p programs/Release/gpmetis "$MetisOutputFolder/bin/" 2>/dev/null || true
    cp -p programs/Release/graphchk "$MetisOutputFolder/bin/" 2>/dev/null || true
    cp -p programs/Release/m2gmetis "$MetisOutputFolder/bin/" 2>/dev/null || true
    cp -p programs/Release/mpmetis "$MetisOutputFolder/bin/" 2>/dev/null || true
    cp -p programs/Release/ndmetis "$MetisOutputFolder/bin/" 2>/dev/null || true
fi

# Copy includes
echo "Copying METIS headers..."
cp -a ../include/metis.h "$MetisOutputFolder/include/" 2>/dev/null || true

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "METIS has been built and installed to:"
echo "  $MetisOutputFolder"
echo ""
echo "Dependencies used:"
echo "  GKlib: $GKlibOutputFolder"
echo ""
echo "METIS provides the following utilities:"
echo "  - gpmetis: Partitions a graph into a specified number of parts"
echo "  - ndmetis: Computes a fill-reducing ordering of a sparse matrix"
echo "  - mpmetis: Partitions a mesh into a specified number of parts"
echo "  - m2gmetis: Converts a mesh into a graph"
echo "  - graphchk: Checks if a graph file is valid"
echo ""
