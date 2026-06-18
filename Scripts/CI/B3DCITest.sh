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

echo "::phase::snapshot_tests"

# List of example executables for snapshot testing
# Note: CustomMaterials is disabled in CMakeLists.txt (outdated shader language)
SNAPSHOT_TESTS=(
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
	"NullBackends"
)

for TEST_NAME in "${SNAPSHOT_TESTS[@]}"; do
	EXAMPLE_EXE="$BIN_DIR/$TEST_NAME.exe"

	if [ ! -f "$EXAMPLE_EXE" ]; then
		echo "::error::Example executable not found: $EXAMPLE_EXE"
		FAILED_TESTS+=("$TEST_NAME (not found)")
		continue
	fi

	echo "Running snapshot test: $TEST_NAME"

	mkdir -p "$RESULTS_DIR/snapshots/$TEST_NAME"

	set +e
	"$EXAMPLE_EXE" \
		--headless \
		--gpu.PreferIntegrated=true \
		--enable-test-snapshot \
		--test-output-path="$RESULTS_DIR/snapshots/$TEST_NAME" \
		--test-name="$TEST_NAME" \
		--exit-after-n-frames=100 \
		--capture-frame=50 2>&1 | tee "$RESULTS_DIR/snapshots/$TEST_NAME/${TEST_NAME}_log.txt"
	SNAPSHOT_EXIT_CODE=${PIPESTATUS[0]}
	set -e

	if [ $SNAPSHOT_EXIT_CODE -ne 0 ]; then
		echo "::error::Snapshot test $TEST_NAME failed with exit code $SNAPSHOT_EXIT_CODE"
		FAILED_TESTS+=("$TEST_NAME")
	fi
done

echo "::phase::editor_snapshot"

# Run the editor itself in headless mode and capture a screenshot of its UI, using
# the same snapshot mechanism as the example snapshot tests above.
EDITOR_NAME="Editor"
EDITOR_EXE="$BIN_DIR/Banshee3D.exe"

if [ ! -f "$EDITOR_EXE" ]; then
	echo "::error::Editor executable not found: $EDITOR_EXE"
	FAILED_TESTS+=("$EDITOR_NAME (not found)")
else
	echo "Running editor snapshot test: $EDITOR_NAME"

	mkdir -p "$RESULTS_DIR/snapshots/$EDITOR_NAME"

	set +e
	"$EDITOR_EXE" \
		--headless \
		--gpu.PreferIntegrated=true \
		--enable-test-snapshot \
		--test-output-path="$RESULTS_DIR/snapshots/$EDITOR_NAME" \
		--test-name="$EDITOR_NAME" \
		--exit-after-n-frames=100 \
		--capture-frame=50 2>&1 | tee "$RESULTS_DIR/snapshots/$EDITOR_NAME/${EDITOR_NAME}_log.txt"
	EDITOR_EXIT_CODE=${PIPESTATUS[0]}
	set -e

	if [ $EDITOR_EXIT_CODE -ne 0 ]; then
		echo "::error::Editor snapshot test failed with exit code $EDITOR_EXIT_CODE"
		FAILED_TESTS+=("$EDITOR_NAME")
	fi
fi

if [ ${#FAILED_TESTS[@]} -gt 0 ]; then
	echo "::error::${#FAILED_TESTS[@]} test(s) failed:"
	for TEST in "${FAILED_TESTS[@]}"; do
		echo "  - $TEST"
	done
	exit 1
fi

echo "All tests passed"
