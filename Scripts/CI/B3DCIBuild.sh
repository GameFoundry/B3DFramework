#!/bin/bash
set -e

echo "=== Building Banshee 3D ==="
echo "Configuration: $CONFIGURATION_NAME"
echo "Build Number: $BUILD_NUMBER"
echo "Clean Build: ${CLEAN_BUILD:-0}"

cd "$WORKSPACE"

# Build type from config (default to RelWithDebInfo)
BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"

# The workspace is either the editor repository (framework at Framework/) or a standalone framework
# repository. A framework checkout builds and installs the framework SDK instead of the editor.
if [ -f "$WORKSPACE/Framework/CMakeLists.txt" ]; then
	ROOT_KIND="editor"
	FRAMEWORK_DIR="$WORKSPACE/Framework"
else
	ROOT_KIND="framework"
	FRAMEWORK_DIR="$WORKSPACE"
fi
echo "Checkout: $ROOT_KIND"

# Where everything a later deployment needs is left: the deploy script itself and the tools it
# runs, packaged dependencies, and (on request) the framework archive. BansheeForge transfers
# this directory to the orchestrator and runs deploy.sh there.
DEPLOY_DIR="${DEPLOY_DIR:-$WORKSPACE/Build/Deploy}"

# Target platform, as injected by the BansheeForge agent ($PLATFORM: win32, darwin, linux, ps5).
# When run by hand outside CI, fall back to the host OS.
if [ -z "${PLATFORM:-}" ]; then
	case "$OSTYPE" in
		msys*|cygwin*|win32) PLATFORM="win32" ;;
		darwin*)             PLATFORM="darwin" ;;
		linux*)              PLATFORM="linux" ;;
		*) echo "::error::Cannot infer platform from OSTYPE=$OSTYPE; set PLATFORM" ; exit 1 ;;
	esac
fi
Platform="$PLATFORM"

# Platform-specific CMake generator + architecture defaults. Override by exporting
# B3D_CMAKE_GENERATOR and/or B3D_CMAKE_ARCHITECTURE before invoking this script
# (e.g. to build for a non-host target such as ARM64 Windows).
case "$Platform" in
	win32)
		DefaultGenerator="Visual Studio 18 2026"
		DefaultArchitecture="x64"
		InstallTarget="INSTALL"
		;;
	darwin|linux)
		DefaultGenerator="Ninja Multi-Config"
		DefaultArchitecture=""
		InstallTarget="install"
		;;
	*)
		# Console platforms with proprietary SDKs are built by an overlay kept outside this
		# repository (Framework/Platform/<Name>/Scripts/CI/B3DCIBuildOverlay.sh), which takes over
		# the whole build when present.
		OVERLAY_BUILD=""
		for OVERLAY_SCRIPT in "$FRAMEWORK_DIR"/Platform/*/Scripts/CI/B3DCIBuildOverlay.sh; do
			[ -f "$OVERLAY_SCRIPT" ] && OVERLAY_BUILD="$OVERLAY_SCRIPT"
		done
		if [ -z "$OVERLAY_BUILD" ]; then
			echo "::error::Unsupported platform: $Platform (no build overlay found)"
			exit 1
		fi
		echo "Delegating $Platform build to overlay: $OVERLAY_BUILD"
		source "$OVERLAY_BUILD"
		exit $?
		;;
esac

CMakeGenerator="${B3D_CMAKE_GENERATOR:-$DefaultGenerator}"
CMakeArchitecture="${B3D_CMAKE_ARCHITECTURE-$DefaultArchitecture}"

echo "Platform: $Platform"
echo "Generator: $CMakeGenerator"
if [ -n "$CMakeArchitecture" ]; then
	echo "Architecture: $CMakeArchitecture"
fi

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

CMakeArgs=(
    -G "$CMakeGenerator"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$ARTIFACTS_DIR/"
    -DB3D_BUILD_ALL_PLUGINS=ON
    -DB3D_BUILD_TESTS=ON
)
if [ -n "$CMakeArchitecture" ]; then
    CMakeArgs+=(-A "$CMakeArchitecture")
fi

cmake .. "${CMakeArgs[@]}"

# Build
echo "::phase::compile"
if [ "$INCREMENTAL" = "1" ]; then
    echo "Running incremental build..."
else
    echo "Running full build..."
fi

# Build main editor (framework checkouts build the framework library instead)
if [ "$ROOT_KIND" = "editor" ]; then
	cmake --build . --target Banshee3D --config "$BUILD_TYPE" --parallel
else
	cmake --build . --target bsf --config "$BUILD_TYPE" --parallel
fi

# Build unit test runner
cmake --build . --target UnitTestRunner --config "$BUILD_TYPE" --parallel

# Build all examples for snapshot testing
# Note: CustomMaterials is disabled in CMakeLists.txt (outdated shader language)
EXAMPLES=(
	"AdvancedLowLevelRendering"
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

cmake --build . --target "$InstallTarget" --config "$BUILD_TYPE" --parallel

# Leave everything a deployment needs next to the install tree. Packaging happens now, from the
# exact folders this build consumed, so a later deployment never has to read a workspace that
# another build may have changed since. The orchestrator only knows how to transfer these files
# and run deploy.sh; what they mean is decided here.
echo "::phase::deploy-inputs"
mkdir -p "$DEPLOY_DIR"
rm -rf "$DEPLOY_DIR/dependencies" "$DEPLOY_DIR/framework" "$DEPLOY_DIR/tools" "$DEPLOY_DIR/deploy.sh" "$DEPLOY_DIR/build-info.txt"
mkdir -p "$DEPLOY_DIR/dependencies" "$DEPLOY_DIR/tools"

CMAKE_PLATFORM=$(grep -E '^B3D_PLATFORM:' CMakeCache.txt | cut -d= -f2 | tr -d '\r')
{
	echo "rootKind=$ROOT_KIND"
	echo "platform=$Platform"
	echo "cmakePlatform=$CMAKE_PLATFORM"
	echo "arch=${ARCH:-x64}"
	echo "buildType=$BUILD_TYPE"
	echo "rootCommit=$(git -C "$WORKSPACE" rev-parse HEAD)"
} > "$DEPLOY_DIR/build-info.txt"

# The deploy script and the uploader it runs travel with the build, taken from the tested commit.
cp "$FRAMEWORK_DIR/Scripts/CI/B3DCIDeploy.sh" "$DEPLOY_DIR/deploy.sh"
cp "$FRAMEWORK_DIR/Scripts/B3DUploadBinaries.sh" "$DEPLOY_DIR/tools/B3DUploadBinaries.sh"

# Every dependency the configure step had to build from source (no package server holds those
# yet) is archived. Such a folder carries a .builtfromsource stamp. Dependencies live in the
# framework's Dependencies folder or in the active platform's overlay.
#
# The stamp is removed once the archive exists, so only this build carries it: deploying this
# build publishes it. Otherwise the stamp would survive in the incremental workspace, and every
# later build would archive the dependency again. If this build is never deployed the package
# stays unpublished, and the next build that cannot download it builds and archives it again.
for depStamp in "$FRAMEWORK_DIR"/Dependencies/*/.builtfromsource "$FRAMEWORK_DIR/Platform/$CMAKE_PLATFORM"/Dependencies/*/.builtfromsource; do
	[ -f "$depStamp" ] || continue
	depFolder=$(dirname "$depStamp")
	depName=$(basename "$depFolder")
	echo "Packaging source-built dependency $depName v$(tr -d '\r\n' < "$depStamp")..."
	"$FRAMEWORK_DIR/Scripts/B3DUploadBinaries.sh" "$depName" --no-upload --no-bump \
		--folder "$depFolder" --platform "$CMAKE_PLATFORM" --output "$DEPLOY_DIR/dependencies"
	rm -f "$depStamp"
done

# The framework archive is a build option (PACKAGE_FRAMEWORK) of framework configurations: the
# install tree zipped with a manifest of the commits it was built from and per-file checksums.
# It carries no version; deploy.sh names it when a FRAMEWORK_VERSION is given at deploy time.
if [ "${PACKAGE_FRAMEWORK:-}" = "1" ] || [ "${PACKAGE_FRAMEWORK:-}" = "true" ]; then
	if [ "$ROOT_KIND" != "framework" ]; then
		echo "::error::PACKAGE_FRAMEWORK needs a standalone framework checkout; this is an editor build"
		exit 1
	fi
	if [ ! -d "$ARTIFACTS_DIR" ] || [ -z "$(ls -A "$ARTIFACTS_DIR")" ]; then
		echo "::error::Install tree is empty: $ARTIFACTS_DIR"
		exit 1
	fi
	ARCHIVE_NAME="B3DFramework-${Platform}-${ARCH:-x64}-${BUILD_TYPE}.zip"
	ARCHIVE_PATH="$DEPLOY_DIR/framework/$ARCHIVE_NAME"
	mkdir -p "$DEPLOY_DIR/framework"

	{
		echo "platform=$Platform"
		echo "arch=${ARCH:-x64}"
		echo "buildType=$BUILD_TYPE"
		echo "buildId=${BUILD_ID:-}"
		echo "createdAt=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
		echo "source.root=$(git -C "$WORKSPACE" rev-parse HEAD) $(git -C "$WORKSPACE" remote get-url origin 2>/dev/null || echo "$GIT_URL")"
		git -C "$WORKSPACE" submodule foreach --recursive --quiet 'echo "source.$displaypath=$(git rev-parse HEAD) $(git remote get-url origin)"'
	} > "$ARTIFACTS_DIR/B3DFrameworkManifest.txt"

	echo "Computing checksums..."
	if command -v sha256sum >/dev/null 2>&1; then SHA256="sha256sum"; else SHA256="shasum -a 256"; fi
	(cd "$ARTIFACTS_DIR" && find . -type f ! -name B3DFrameworkChecksums.sha256 -print0 | sort -z | while IFS= read -r -d '' file; do
		echo "$($SHA256 "$file" | cut -d' ' -f1)  ${file#./}"
	done) > "$ARTIFACTS_DIR/B3DFrameworkChecksums.sha256"

	echo "Creating $ARCHIVE_NAME..."
	(cd "$ARTIFACTS_DIR" && cmake -E tar cf "$ARCHIVE_PATH" --format=zip -- *)
	echo "Framework archive ready: $ARCHIVE_PATH ($(du -h "$ARCHIVE_PATH" | cut -f1))"
fi

echo "=== Build complete ==="
