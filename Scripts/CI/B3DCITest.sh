#!/bin/bash
set -e

# Requires bash >= 4 (associative arrays, ${var,,}). macOS ships 3.2: the BansheeForge agent runs
# this through Homebrew bash when installed (brew install bash).
if [ "${BASH_VERSINFO[0]}" -lt 4 ]; then
	echo "::error::bash >= 4 required, found $BASH_VERSION (on macOS: brew install bash)"
	exit 1
fi

BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"
BUILD_DIR="$WORKSPACE/Build"

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

# Binary layout and executable suffix per platform. $ARCH comes from the agent (x64/arm64);
# CMake places binaries under bin/<arch>/<config>.
case "$PLATFORM" in
	win32)
		ARCH="${ARCH:-x64}"
		EXE_SUFFIX=".exe"
		;;
	darwin)
		ARCH="${ARCH:-$(uname -m | sed 's/x86_64/x64/')}"
		EXE_SUFFIX=""
		;;
	linux)
		ARCH="${ARCH:-x64}"
		EXE_SUFFIX=""
		;;
	*)
		# Console platforms run their tests through an overlay (see below); binaries are not local.
		ARCH="${ARCH:-x64}"
		EXE_SUFFIX=""
		;;
esac
BIN_DIR="$BUILD_DIR/bin/$ARCH/$BUILD_TYPE"

echo "::phase::setup"
echo "Workspace: $WORKSPACE"
echo "Platform: $PLATFORM ($ARCH)"
echo "Build type: $BUILD_TYPE"
echo "Binary directory: $BIN_DIR"
echo "Results directory: $RESULTS_DIR"

mkdir -p "$RESULTS_DIR"
mkdir -p "$RESULTS_DIR/snapshots"

# Track test failures
FAILED_TESTS=()

echo "::phase::unit_tests"

TEST_RUNNER="$BIN_DIR/UnitTestRunner$EXE_SUFFIX"

if [ ! -f "$TEST_RUNNER" ]; then
	echo "::error::UnitTestRunner not found at $TEST_RUNNER"
	exit 1
fi

echo "Running unit tests..."

cd "$BIN_DIR"
set +e
"$TEST_RUNNER" \
	--headless \
	--gpu.PreferIntegrated=true \
	--debug.DisableErrorDialogs=true \
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
# Categories differ per platform: Windows exercises Vulkan and D3D12, macOS exercises Metal and
# Vulkan via MoltenVK. The first category on each platform is that platform's default backend and
# passes no backend argument, so NullBackends (which hardcodes the null GPU backend and therefore
# only runs in that category) gets no conflicting override. The Editor category runs the editor itself in headless mode and
# captures a screenshot of its UI, using the same snapshot mechanism as the example snapshot tests.
# It is skipped on platforms where the editor binary was not built.
COMMON_TESTS="Audio Decals GUI GUICulling Lighting LowLevelRendering Particles Physics PhysicallyBasedShading SkeletalAnimation VectorGraphics"

declare -A CATEGORY_ARGS=()
declare -A CATEGORY_TESTS=()

case "$PLATFORM" in
	win32)
		SNAPSHOT_CATEGORIES=("Vulkan" "D3D12" "Editor")
		CATEGORY_ARGS["Vulkan"]=""
		CATEGORY_ARGS["D3D12"]="--gpu.backend=bsfD3D12GpuBackend"
		CATEGORY_ARGS["Editor"]=""
		CATEGORY_TESTS["Vulkan"]="$COMMON_TESTS NullBackends"
		CATEGORY_TESTS["D3D12"]="$COMMON_TESTS"
		CATEGORY_TESTS["Editor"]="Editor"
		;;
	darwin)
		SNAPSHOT_CATEGORIES=("Metal" "Vulkan" "Editor")
		CATEGORY_ARGS["Metal"]=""
		CATEGORY_ARGS["Vulkan"]="--gpu.backend=bsfVulkanGpuBackend"
		CATEGORY_ARGS["Editor"]=""
		CATEGORY_TESTS["Metal"]="$COMMON_TESTS NullBackends"
		CATEGORY_TESTS["Vulkan"]="$COMMON_TESTS"
		CATEGORY_TESTS["Editor"]="Editor"
		;;
	linux)
		SNAPSHOT_CATEGORIES=("Vulkan" "Editor")
		CATEGORY_ARGS["Vulkan"]=""
		CATEGORY_ARGS["Editor"]=""
		CATEGORY_TESTS["Vulkan"]="$COMMON_TESTS NullBackends"
		CATEGORY_TESTS["Editor"]="Editor"
		;;
	*)
		# Console platforms declare their categories from an overlay.
		SNAPSHOT_CATEGORIES=()
		;;
esac

# Skip the Editor category when the editor was not built for this platform (e.g. a port in progress).
if [ ! -f "$BIN_DIR/Banshee3D$EXE_SUFFIX" ] && [ "${CATEGORY_TESTS[Editor]:-}" = "Editor" ]; then
	echo "Editor binary not found at $BIN_DIR/Banshee3D$EXE_SUFFIX; skipping Editor snapshot category"
	FILTERED=()
	for CATEGORY in "${SNAPSHOT_CATEGORIES[@]}"; do
		[ "$CATEGORY" != "Editor" ] && FILTERED+=("$CATEGORY")
	done
	SNAPSHOT_CATEGORIES=("${FILTERED[@]}")
	unset 'CATEGORY_TESTS[Editor]'
fi

# ---------------------------------------------------------------------------
# Platform overlays with proprietary SDKs (kept outside this repository) may
# extend the test run. An overlay is sourced when present and may append to
# SNAPSHOT_CATEGORIES/CATEGORY_TESTS, register a custom per-test runner
# function in CATEGORY_RUNNERS (invoked as <runner> <category> <test-name> in
# place of the local-executable path below), and append functions to
# OVERLAY_UNIT_TEST_HOOKS to run additional unit test suites. Overlays are
# expected to deactivate themselves when their SDK or binaries are missing.
# ---------------------------------------------------------------------------
declare -A CATEGORY_RUNNERS=()
OVERLAY_UNIT_TEST_HOOKS=()

for OVERLAY_SCRIPT in "$WORKSPACE"/Framework/Platform/*/Scripts/CI/B3DCITestOverlay.sh; do
	if [ -f "$OVERLAY_SCRIPT" ]; then
		echo "Loading test overlay: $OVERLAY_SCRIPT"
		source "$OVERLAY_SCRIPT"
	fi
done

for OVERLAY_HOOK in "${OVERLAY_UNIT_TEST_HOOKS[@]}"; do
	"$OVERLAY_HOOK"
done

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
		--debug.DisableErrorDialogs=true \
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
		# Categories provided by a platform overlay run through its registered runner
		if [ -n "${CATEGORY_RUNNERS[$CATEGORY]:-}" ]; then
			"${CATEGORY_RUNNERS[$CATEGORY]}" "$CATEGORY" "$TEST_NAME"
			continue
		fi

		# The Editor snapshot runs the editor executable itself; all other tests are example exes
		if [ "$TEST_NAME" = "Editor" ]; then
			EXE="$BIN_DIR/Banshee3D$EXE_SUFFIX"
		else
			EXE="$BIN_DIR/$TEST_NAME$EXE_SUFFIX"
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
