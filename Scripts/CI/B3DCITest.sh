#!/bin/bash
set -e

BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"
BUILD_DIR="$WORKSPACE/Build"
BIN_DIR="$BUILD_DIR/bin/x64/$BUILD_TYPE"

echo "::phase::setup"
echo "Workspace: $WORKSPACE"
echo "Build type: $BUILD_TYPE"
echo "Binary directory: $BIN_DIR"
echo "Results directory: $RESULTS_DIR"

mkdir -p "$RESULTS_DIR"
mkdir -p "$RESULTS_DIR/snapshots"

# Track test failures
FAILED_TESTS=()

echo "::phase::unit_tests"

TEST_RUNNER="$BIN_DIR/UnitTestRunner.exe"

if [ ! -f "$TEST_RUNNER" ]; then
	echo "::error::UnitTestRunner not found at $TEST_RUNNER"
	exit 1
fi

echo "Running unit tests..."

cd "$BIN_DIR"
set +e
./UnitTestRunner.exe \
	--headless \
	--gpu.PreferIntegrated=true \
	--test-output-format=json \
	--test-output-path="$RESULTS_DIR/unit_tests.json" \
	--test-layer=all 2>&1 | tee "$RESULTS_DIR/unit_tests.log"
UNIT_TEST_EXIT_CODE=${PIPESTATUS[0]}
set -e

echo "Unit tests finished with exit code: $UNIT_TEST_EXIT_CODE"

if [ $UNIT_TEST_EXIT_CODE -ne 0 ]; then
	echo "::error::UnitTestRunner failed with exit code $UNIT_TEST_EXIT_CODE"
	exit $UNIT_TEST_EXIT_CODE
fi

# ---------------------------------------------------------------------------
# Snapshot test categories. Each category runs the same snapshot mechanism with
# different engine command-line arguments (e.g. a different GPU backend). The
# categories are declared to BansheeForge by emitting `::snapshot-category::NAME`
# markers on stdout (parsed like `::phase::` markers); emission order determines
# sub-tab order in the UI. Category names must be filesystem/URL-safe
# ([A-Za-z0-9_-]) as they are used as directory names.
# ---------------------------------------------------------------------------
SNAPSHOT_CATEGORIES=("Vulkan" "D3D12" "Editor")

declare -A CATEGORY_ARGS=(
	["Vulkan"]=""
	["D3D12"]="--gpu.backend=bsfD3D12GpuBackend"
	["Editor"]=""
)

# List of example executables for snapshot testing
# Note: CustomMaterials is disabled in CMakeLists.txt (outdated shader language)
# NullBackends hardcodes the null GPU backend so it only runs in the default category
COMMON_TESTS="Audio Decals GUI GUICulling Lighting LowLevelRendering Particles Physics PhysicallyBasedShading SkeletalAnimation VectorGraphics"

# The Editor category runs the editor itself in headless mode and captures a screenshot
# of its UI, using the same snapshot mechanism as the example snapshot tests.
declare -A CATEGORY_TESTS=(
	["Vulkan"]="$COMMON_TESTS NullBackends"
	["D3D12"]="$COMMON_TESTS"
	["Editor"]="Editor"
)

# Declare the categories to BansheeForge up-front (before running any test) so a
# crashed/killed run still reports the full set of categories.
for CATEGORY in "${SNAPSHOT_CATEGORIES[@]}"; do
	echo "::snapshot-category::$CATEGORY"
done

# run_snapshot <category> <test-name> <exe-path>
run_snapshot() {
	local CATEGORY="$1"
	local TEST_NAME="$2"
	local EXE="$3"
	local OUT_DIR="$RESULTS_DIR/snapshots/$CATEGORY/$TEST_NAME"

	if [ ! -f "$EXE" ]; then
		echo "::error::Executable not found: $EXE"
		FAILED_TESTS+=("$CATEGORY/$TEST_NAME (not found)")
		return
	fi

	echo "Running snapshot test: $CATEGORY/$TEST_NAME"

	mkdir -p "$OUT_DIR"

	set +e
	# CATEGORY_ARGS is intentionally unquoted so it word-splits into arguments
	"$EXE" \
		--headless \
		--gpu.PreferIntegrated=true \
		--enable-test-snapshot \
		--test-output-path="$OUT_DIR" \
		--test-name="$TEST_NAME" \
		--exit-after-n-frames=100 \
		--capture-frame=50 \
		${CATEGORY_ARGS[$CATEGORY]} 2>&1 | tee "$OUT_DIR/${TEST_NAME}_log.txt"
	local EXIT_CODE=${PIPESTATUS[0]}
	set -e

	if [ $EXIT_CODE -ne 0 ]; then
		echo "::error::Snapshot test $CATEGORY/$TEST_NAME failed with exit code $EXIT_CODE"
		FAILED_TESTS+=("$CATEGORY/$TEST_NAME")
	fi
}

for CATEGORY in "${SNAPSHOT_CATEGORIES[@]}"; do
	echo "::phase::snapshot_tests_${CATEGORY,,}"

	for TEST_NAME in ${CATEGORY_TESTS[$CATEGORY]}; do
		# The Editor snapshot runs the editor executable itself; all other tests are example exes
		if [ "$TEST_NAME" = "Editor" ]; then
			EXE="$BIN_DIR/Banshee3D.exe"
		else
			EXE="$BIN_DIR/$TEST_NAME.exe"
		fi

		run_snapshot "$CATEGORY" "$TEST_NAME" "$EXE"
	done
done

if [ ${#FAILED_TESTS[@]} -gt 0 ]; then
	echo "::error::${#FAILED_TESTS[@]} test(s) failed:"
	for TEST in "${FAILED_TESTS[@]}"; do
		echo "  - $TEST"
	done
	exit 1
fi

echo "All tests passed"
