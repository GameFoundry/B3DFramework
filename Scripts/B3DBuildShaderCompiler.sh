#!/bin/sh

. ./B3DBuildCommon.sh

echo "Builds B3DShaderCompiler (XShaderCompiler) from source"
echo ""

# ======================================================================================================
# Optional, overlay-gated shader-compiler backends
# ======================================================================================================
# A platform overlay in the engine tree (Framework/Platform/<Name>) may ship an optional shader output
# backend for that platform. The backend's source is a git submodule of the B3DShaderCompiler repo at
# src/Compiler/Backend/<Name> (the submodule path's leaf matches the overlay name). In a shared build it
# compiles to a separately loadable module, which the compiler installs under a dedicated CMake install
# component named backend_<Name> (its source-folder leaf). We pull that one component straight into the
# overlay's private dependency folder with `cmake --install --component backend_<Name>` -- so the script
# never has to know, or parse out of the CMake sources, the backend module's own DLL id.

# Names of the overlays whose optional shader backend is present and enrolled for this build. Populated by
# B3DPrepareOverlayBackend, consumed by the per-config install loop and the final summary. Space-separated
# (overlay names are single tokens), so a plain `for overlay in $ActiveBackendOverlays` walks it.
ActiveBackendOverlays=""

# Enrolls the named platform overlay's optional shader backend for this build, if it applies. A backend is
# host-only tooling and, for now, Windows-only (the .dll routing layout; Linux/macOS host backends are
# deferred), and each overlay is independent -- one being present says nothing about another. When the
# overlay is present AND its access-restricted backend submodule (src/Compiler/Backend/<overlay>) checks
# out with sources, the overlay is appended to $ActiveBackendOverlays; otherwise it is silently skipped.
# An unauthorized checkout or an unfinalized submodule URL leaves the submodule empty -- the backend's own
# CMakeLists self-guards on that, so configure stays safe, and skipping it here keeps the rest of the
# build from failing loudly. Call once per candidate overlay, after the compiler repo is cloned.
# @1 = overlay name. Needs $Platform, $CurrentDirectory, $CompilerRepositoryRoot.
B3DPrepareOverlayBackend() {
	overlayName="$1"

	case "$Platform" in win32|msys) ;; *) return 0 ;; esac
	[ -d "$CurrentDirectory/../Platform/$overlayName" ] || return 0

	echo "$overlayName overlay detected -- initializing its optional shader backend submodule..."
	git -C "$CompilerRepositoryRoot" submodule update --init -- "src/Compiler/Backend/$overlayName" || true
	if ! ls "$CompilerRepositoryRoot/src/Compiler/Backend/$overlayName"/*.cpp >/dev/null 2>&1; then
		echo "[Warning] $overlayName backend submodule is empty or unavailable; building without it."
		return 0
	fi

	ActiveBackendOverlays="$ActiveBackendOverlays $overlayName"
}

# Installs ONLY the named overlay's shader backend DLL (+PDB) straight into that overlay's private
# dependency folder, via the backend's dedicated install component. The component is named after the
# overlay (== the backend's source-folder leaf), so we never need to know -- or parse out of the CMake
# sources -- the backend module's own DLL id, and no other present overlay's backend can come along. Run
# from the cmake_build directory. @1 = overlay name, @2 = CMake config to install (RelWithDebInfo |
# Release), @3 = engine output sub-folder (Debug | Release). Needs $CurrentDirectory, $SharedLibraryExtension.
B3DInstallOverlayBackend() {
	overlayName="$1"
	cmakeConfiguration="$2"
	outputConfiguration="$3"

	backendDestination="$CurrentDirectory/../Platform/$overlayName/Dependencies/B3DShaderCompilerBackend/bin/$outputConfiguration"
	cmake --install . \
		--config "$cmakeConfiguration" \
		--component "backend_$overlayName" \
		--prefix "$backendDestination" || exit 1

	# The build already errors out if the backend failed to compile, but a silent no-op install (e.g. a
	# component-name drift) would otherwise leave the overlay quietly backend-less -- so confirm a DLL
	# actually landed. This is a presence check, not routing: it neither names nor parses the DLL's id.
	if ! ls "$backendDestination"/xsc_backend_*"$SharedLibraryExtension" >/dev/null 2>&1; then
		echo "[Error] Overlay '$overlayName': its shader backend install component placed no DLL for the"
		echo "        $outputConfiguration configuration. Aborting to avoid a silently incomplete build."
		exit 1
	fi
}

# Check prerequisites
if ! command -v cmake &> /dev/null; then
	echo "[Error] CMake is not installed. Please install CMake 2.8 or later."
	exit 1
fi

# B3DShaderCompiler is host-only tooling: shader cross-compilation runs on the host, never on a
# device. Reject a non-host --target rather than silently producing an unusable artifact.
case "$Platform" in
	win32|msys|darwin*|linux-gnu*) ;;
	*)
		echo "[Error] B3DShaderCompiler is a host-only build tool (it cross-compiles shaders on the host)."
		echo "        It cannot be built for the non-host target '$Platform'. Run without --target,"
		echo "        or pass a host platform."
		exit 1
		;;
esac

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

# Absolute path to the compiler repo root (cwd here). B3DPrepareOverlayBackend uses it (via `git -C`) so
# it works from any later working directory, and the install step derives the backend submodule path from
# it too.
CompilerRepositoryRoot="$(pwd)"

# Enroll each candidate platform overlay's optional shader backend (no-op when its overlay is absent or
# its access-restricted submodule is unavailable). Add one line per overlay as new backends appear, e.g.
# B3DPrepareOverlayBackend "XBOX".
B3DPrepareOverlayBackend "PS5"

# Setup B3DShaderCompiler output folders
ShaderCompilerOutputFolder="$PlatformDependencyFolder/XShaderCompiler"

echo "Output folder: $ShaderCompilerOutputFolder"

# Read existing version before cleaning the folder
VersionFile="$ShaderCompilerOutputFolder/.version"
if [ -f "$VersionFile" ]; then
	CurrentVersion=$(cat "$VersionFile")
	NewVersion=$((CurrentVersion + 1))
else
	NewVersion=0
fi

B3DCleanDependencyFolder "$ShaderCompilerOutputFolder"
mkdir -p "$ShaderCompilerOutputFolder/include/"
mkdir -p "$ShaderCompilerOutputFolder/bin/"
mkdir -p "$ShaderCompilerOutputFolder/lib/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	mkdir -p "$ShaderCompilerOutputFolder/bin/Release/"
	mkdir -p "$ShaderCompilerOutputFolder/bin/Debug/"
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
# - XSC_SHARED_LIB=ON: Build a shared library (xsc_core.dll + import xsc_core.lib).
# - XSC_BUILD_HLSL=ON: Register the HLSL round-trip output backend; the engine's "hlsl" shading language
#   cross-compiles through it (register normalization + cross-stage-unique auto-binding).
echo "Configuring CMake..."

cmake .. -G "$CMakeGenerator" \
	-DINSTALL_OUTPUT_PATH="$ShaderCompilerOutputFolder" \
	-DXSC_BUILD_SHELL=OFF \
	-DXSC_BUILD_DEBUGGER=OFF \
	-DXSC_BUILD_TESTS=OFF \
	-DXSC_SHARED_LIB=ON \
	-DXSC_BUILD_HLSL=ON || exit 1

# Builds one CMake configuration, installs the core into the public folder and reorganizes it into the
# engine's dependency layout, then installs any enrolled overlay backends for this config.
# @1 = CMake config to build (RelWithDebInfo | Release), @2 = engine output sub-folder (Debug | Release).
B3DBuildAndInstallConfiguration() {
	cmakeConfiguration="$1"
	outputConfiguration="$2"

	echo "Building $cmakeConfiguration configuration..."
	cmake --build . --config "$cmakeConfiguration" || exit 1

	echo "Installing + routing $cmakeConfiguration -> $outputConfiguration..."
	cmake --build . --config "$cmakeConfiguration" --target install || exit 1

	# The default install drops the core FLAT at the root of the public dependency folder; relocate it into
	# the engine's dependency layout (headers install straight to include/Xsc, so they need no move).
	sharedLibrary="${StaticLibraryPrefix}xsc_core${SharedLibraryExtension}"
	if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
		mv "$ShaderCompilerOutputFolder/$sharedLibrary" "$ShaderCompilerOutputFolder/bin/$outputConfiguration/" || exit 1
		mv "$ShaderCompilerOutputFolder/xsc_core.pdb" "$ShaderCompilerOutputFolder/bin/$outputConfiguration/" 2>/dev/null || true
		mv "$ShaderCompilerOutputFolder/xsc_core${ImportLibraryExtension}" "$ShaderCompilerOutputFolder/lib/$outputConfiguration/" || exit 1
	else
		mv "$ShaderCompilerOutputFolder/$sharedLibrary"* "$ShaderCompilerOutputFolder/lib/" 2>/dev/null || true
	fi

	# Optional platform backend(s): install each enrolled overlay's backend ONLY into its own overlay. This
	# is a separate, component-scoped install; the default install above is backend-free. No-op on a Unix
	# host -- backends are deferred there, so the enrolled-overlays list is empty.
	for overlay in $ActiveBackendOverlays; do
		B3DInstallOverlayBackend "$overlay" "$cmakeConfiguration" "$outputConfiguration"
	done
}

# Build based on platform
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	# Debug first, then Release.
	B3DBuildAndInstallConfiguration "Debug" "Debug"
	B3DBuildAndInstallConfiguration "Release" "Release"
else
	# Host Unix (macOS/Linux): a single Release shared library (it serves both engine configs). Optional
	# shader backends are deferred here (a host backend would be libxsc_backend_*.{so,dylib}).
	B3DBuildAndInstallConfiguration "Release" "Release"
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
echo "Headers location:  $ShaderCompilerOutputFolder/include/Xsc"
echo "Binaries location: $ShaderCompilerOutputFolder/bin"
echo "Import libraries:  $ShaderCompilerOutputFolder/lib"
for overlay in $ActiveBackendOverlays; do
	echo "$overlay backend:       $CurrentDirectory/../Platform/$overlay/Dependencies/B3DShaderCompilerBackend/bin"
done
echo ""
