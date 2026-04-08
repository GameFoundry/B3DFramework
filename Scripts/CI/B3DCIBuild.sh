#!/bin/bash
set -e

echo "=== Building Banshee 3D ==="
echo "Configuration: $CONFIGURATION_NAME"
echo "Build Number: $BUILD_NUMBER"
echo "Clean Build: ${CLEAN_BUILD:-0}"

cd "$WORKSPACE"

# Build type from config (default to RelWithDebInfo)
BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"

# Create build directory
BUILD_DIR="$WORKSPACE/Build"

# Check if this is an incremental build
if [ -d "$BUILD_DIR" ] && [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "Using existing build directory (incremental build)"
    INCREMENTAL=1
else
    echo "Creating fresh build directory"
    INCREMENTAL=0
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "::phase::configure"
if [ "$INCREMENTAL" = "1" ]; then
    echo "Running incremental CMake configuration..."
else
    echo "Running fresh CMake configuration..."
fi

cmake .. \
    -G "Visual Studio 17 2022" \
    -A x64 \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$ARTIFACTS_DIR/" \
    -DB3D_BUILD_TESTS=ON

# Build
echo "::phase::compile"
if [ "$INCREMENTAL" = "1" ]; then
    echo "Running incremental build..."
else
    echo "Running full build..."
fi

# Build main editor
cmake --build . --target Banshee3D --config "$BUILD_TYPE" --parallel

# Build unit test runner
cmake --build . --target UnitTestRunner --config "$BUILD_TYPE" --parallel

# Build all examples for snapshot testing
# Note: CustomMaterials is disabled in CMakeLists.txt (outdated shader language)
EXAMPLES=(
	"Audio"
	"Decals"
	"GUI"
	"GUICulling"
	"Lighting"
	"LowLevelRendering"
	"Particles"
	"Physics"
	"PhysicallyBasedShading"
	"SkeletalAnimation"
	"VectorGraphics"
)

for EXAMPLE in "${EXAMPLES[@]}"; do
	echo "Building example: $EXAMPLE"
	cmake --build . --target "$EXAMPLE" --config "$BUILD_TYPE" --parallel
done

# Copy artifacts
echo "::phase::artifacts"
echo "Copying build artifacts..."

cmake --build . --target INSTALL --config "$BUILD_TYPE" --parallel

echo "=== Build complete ==="
