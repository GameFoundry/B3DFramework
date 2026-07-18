#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds NVIDIA PhysX 3.4 from source"
echo ""

# PhysX 3.4 is not CMake-based: each platform ships hand-written build files under
# PhysX_3.4/Source/compiler/<platform>/. On Windows we use the VS 2017 solution
# retargeted to the v143 toolset + latest Windows SDK; on Linux we use the supplied
# Makefile; on macOS we use the supplied Xcode project.

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

PHYSX_BRANCH="3.4"

# Clone from NVIDIAGameWorks/PhysX, 3.4 branch
if [ -d "PhysX" ]; then
	cd PhysX
	git fetch origin $PHYSX_BRANCH
	git reset --hard origin/$PHYSX_BRANCH
else
	git clone --branch $PHYSX_BRANCH https://github.com/NVIDIAGameWorks/PhysX.git PhysX
	cd PhysX
fi

PhysXRoot="$(pwd)"

# Apply patch. Carries five kinds of fixes:
# - PsAllocator.h: pre-standard <typeinfo.h> -> <typeinfo> (header removed from modern Windows SDK).
# - vcxproj TreatWarningAsError: true -> false. Newer MSVC versions emit warnings the 2018-era
#   sources weren't written against (signed/unsigned, narrowing, etc.), and there's no intent
#   to fix each one upstream.
# - vcxproj RuntimeLibrary: /MT(d) -> /MD(d). PhysX 3.4 defaults to the static CRT, but Banshee
#   uses the dynamic CRT; linking the static PhysX3Extensions.lib otherwise fails with
#   LNK2038 "MT_StaticRelease doesn't match MD_DynamicRelease". release/profile -> /MD ;
#   debug/checked -> /MDd (Banshee maps its Debug config onto PhysX's checked). The checked
#   config also drops NDEBUG from its PreprocessorDefinitions since /MDd auto-defines _DEBUG,
#   and PxPreprocessor.h rejects both being defined at once.
# - PxPreprocessor.h: detect macOS vs. iOS via TargetConditionals instead of architecture,
#   which misclassified Apple Silicon Macs as iOS.
# - make_osx64 Makefiles: build for the host arch instead of hardcoded x86_64, locate the SDK
#   via xcrun (works without full Xcode), drop -msse2/-Werror, raise deployment target to 11.0.
# - PsUnixFPU.cpp, GuSIMDHelpers.h, SnSerialUtils.cpp: gate x86 intrinsics/serialization tags
#   on the architecture rather than the OS, so ARM64 macOS takes the NEON/generic paths.
echo "Applying PhysX patch..."
git apply "$CurrentDirectory/Patches/PhysX.patch" || exit 1

# Setup output folders
OutputFolder="$PlatformDependencyFolder/PhysX"
B3DCleanDependencyFolder "$OutputFolder"

mkdir -p "$OutputFolder/include"
mkdir -p "$OutputFolder/lib/Release"
mkdir -p "$OutputFolder/lib/Debug"
mkdir -p "$OutputFolder/bin/Release"
mkdir -p "$OutputFolder/bin/Debug"

# Banshee's FindPhysX.cmake maps its Debug configuration to PhysX's "checked" config
# (not PhysX "debug", which is unusably slow — checked keeps optimisations on but
# adds runtime validation). The PhysX DLLs are named PhysX3*_x64 for release and
# PhysX3*CHECKED_x64 for checked.
PhysXReleaseConfig="release"
PhysXDebugConfig="checked"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	# PhysX 3.4 ships a VS 2017 solution (vc15win64). Retarget to v143 (VS 2022) so
	# users don't need VS 2017 build tools side-installed.
	VSWHERE="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
	VSINSTALL=$("$VSWHERE" -latest -requires Microsoft.Component.MSBuild -property installationPath)
	MSBUILD="$VSINSTALL/MSBuild/Current/Bin/MSBuild.exe"

	if [ ! -f "$MSBUILD" ]; then
		echo "[Error] MSBuild not found. Please install Visual Studio with C++ workload."
		exit 1
	fi

	SolutionPath="$PhysXRoot/PhysX_3.4/Source/compiler/vc15win64/PhysX.sln"
	SolutionDir="$(dirname "$SolutionPath")"

	cd "$SolutionDir"

	# Build release + checked. The solution already wires PxShared (PxFoundation, PxPvdSDK)
	# as project references, so a single MSBuild invocation per config produces everything.
	MSBuildArgs="-p:Platform=x64 -p:PlatformToolset=v143 -p:WindowsTargetPlatformVersion=10.0 -m"
	"$MSBUILD" PhysX.sln -p:Configuration=release $MSBuildArgs || exit 1
	"$MSBUILD" PhysX.sln -p:Configuration=checked $MSBuildArgs || exit 1

	# Output is split across two trees: PhysX_3.4/{Bin,Lib}/vc15win64 holds the main PhysX
	# libs; PxShared/{bin,lib}/vc15win64 holds PxFoundation + PxPvdSDK. Both must be
	# shipped — PhysX3 DLLs runtime-link to PxFoundation via PX_FOUNDATION_DLL=1.
	PhysXBinFolder="$PhysXRoot/PhysX_3.4/Bin/vc15win64"
	PhysXLibFolder="$PhysXRoot/PhysX_3.4/Lib/vc15win64"
	PxSharedBinFolder="$PhysXRoot/PxShared/bin/vc15win64"
	PxSharedLibFolder="$PhysXRoot/PxShared/lib/vc15win64"

	# Helper: copy a DLL + PDB if present, from one of two source folders.
	fnCopyDll() {
		local dllName="$1"
		local destFolder="$2"
		for srcFolder in "$PhysXBinFolder" "$PxSharedBinFolder"; do
			if [ -f "$srcFolder/${dllName}.dll" ]; then
				cp -p "$srcFolder/${dllName}.dll" "$destFolder/" || return 1
				[ -f "$srcFolder/${dllName}.pdb" ] && cp -p "$srcFolder/${dllName}.pdb" "$destFolder/"
				return 0
			fi
		done
		echo "[Error] DLL not found: ${dllName}.dll"
		return 1
	}

	# Helper: copy an import/static lib, from one of two source folders.
	fnCopyLib() {
		local libName="$1"
		local destFolder="$2"
		for srcFolder in "$PhysXLibFolder" "$PxSharedLibFolder"; do
			if [ -f "$srcFolder/${libName}.lib" ]; then
				cp -p "$srcFolder/${libName}.lib" "$destFolder/" || return 1
				return 0
			fi
		done
		echo "[Error] Lib not found: ${libName}.lib"
		return 1
	}

	# Release DLLs + PDBs. PhysX3/PhysX3Common/PhysX3Cooking/PhysX3CharacterKinematic are
	# SHARED libs Banshee consumes; PxFoundation/PxPvdSDK are PxShared deps pulled in at
	# runtime via PX_FOUNDATION_DLL=1.
	for DllName in PhysX3_x64 PhysX3Common_x64 PhysX3Cooking_x64 PhysX3CharacterKinematic_x64 PxFoundation_x64 PxPvdSDK_x64; do
		fnCopyDll "$DllName" "$OutputFolder/bin/Release" || exit 1
	done

	# Release import libs + PhysX3Extensions static lib. FindPhysX.cmake searches for
	# PhysX3Extensions with no _x64 suffix — matches upstream naming.
	for LibName in PhysX3_x64 PhysX3Common_x64 PhysX3Cooking_x64 PhysX3CharacterKinematic_x64 PhysX3Extensions PxFoundation_x64 PxPvdSDK_x64; do
		fnCopyLib "$LibName" "$OutputFolder/lib/Release" || exit 1
	done

	# Checked DLLs + PDBs (these land in bin/Debug as the debug configuration).
	for DllName in PhysX3CHECKED_x64 PhysX3CommonCHECKED_x64 PhysX3CookingCHECKED_x64 PhysX3CharacterKinematicCHECKED_x64 PxFoundationCHECKED_x64 PxPvdSDKCHECKED_x64; do
		fnCopyDll "$DllName" "$OutputFolder/bin/Debug" || exit 1
	done

	# Checked import libs + PhysX3ExtensionsCHECKED static lib.
	for LibName in PhysX3CHECKED_x64 PhysX3CommonCHECKED_x64 PhysX3CookingCHECKED_x64 PhysX3CharacterKinematicCHECKED_x64 PhysX3ExtensionsCHECKED PxFoundationCHECKED_x64 PxPvdSDKCHECKED_x64; do
		fnCopyLib "$LibName" "$OutputFolder/lib/Debug" || exit 1
	done

elif [[ "$Platform" == "linux-gnu"* ]]; then
	# PhysX 3.4 Linux build uses plain Makefiles; no configure step.
	cd "$PhysXRoot/PhysX_3.4/Source/compiler/linux64"

	make -j"$(nproc)" $PhysXReleaseConfig || exit 1
	make -j"$(nproc)" $PhysXDebugConfig || exit 1

	BinFolder="$PhysXRoot/PhysX_3.4/Bin/linux64"
	LibFolder="$PhysXRoot/PhysX_3.4/Lib/linux64"

	# Linux uses shared objects for main PhysX libs; static deps for Extensions.
	for SoName in libPhysX3_x64.so libPhysX3Common_x64.so libPhysX3Cooking_x64.so libPhysX3CharacterKinematic_x64.so libPxFoundation_x64.so libPxPvdSDK_x64.so; do
		cp -p "$BinFolder/release/${SoName}" "$OutputFolder/bin/Release/" || exit 1
		cp -p "$BinFolder/checked/${SoName%.so}CHECKED.so" "$OutputFolder/bin/Debug/" 2>/dev/null || true
	done

	cp -p "$LibFolder/release/libPhysX3Extensions.a" "$OutputFolder/lib/Release/" || exit 1
	cp -p "$LibFolder/checked/libPhysX3ExtensionsCHECKED.a" "$OutputFolder/lib/Debug/" || exit 1

elif [[ "$Platform" == "darwin"* ]]; then
	# The supplied Xcode project predates Apple Silicon; the (patched) Makefile build targets
	# the host architecture and needs only the Command Line Tools, not a full Xcode install.
	cd "$PhysXRoot/PhysX_3.4/Source/compiler/make_osx64"

	make -j"$(sysctl -n hw.ncpu)" $PhysXReleaseConfig || exit 1
	make -j"$(sysctl -n hw.ncpu)" $PhysXDebugConfig || exit 1

	# Static archives only, split across the main PhysX tree and the PxShared tree.
	PhysXLibFolder="$PhysXRoot/PhysX_3.4/Lib/osx64"
	PxSharedLibFolder="$PhysXRoot/PxShared/lib/osx64"

	# Copy all static archives expected by FindPhysX.cmake's Apple branch (checked -> Debug).
	for LibName in libLowLevel libLowLevelAABB libLowLevelCloth libLowLevelDynamics libLowLevelParticles libPhysX3 libPhysX3Common libPhysX3Cooking libPhysX3CharacterKinematic libPhysX3Extensions libSceneQuery libSimulationController; do
		cp -p "$PhysXLibFolder/${LibName}.a" "$OutputFolder/lib/Release/" || exit 1
		cp -p "$PhysXLibFolder/${LibName}CHECKED.a" "$OutputFolder/lib/Debug/" || exit 1
	done
	for LibName in libPsFastXml libPxFoundation libPxPvdSDK libPxTask; do
		cp -p "$PxSharedLibFolder/${LibName}.a" "$OutputFolder/lib/Release/" || exit 1
		cp -p "$PxSharedLibFolder/${LibName}CHECKED.a" "$OutputFolder/lib/Debug/" || exit 1
	done
fi

# Merge include trees: PhysX public API + PxShared foundation/task/pvd/etc. bsfPhysX
# #includes both via unqualified paths (e.g. "foundation/Px.h"), so both roots
# must flatten into Dependencies/PhysX/include.
echo "Copying headers..."
cp -rp "$PhysXRoot/PhysX_3.4/Include/." "$OutputFolder/include/" || exit 1
cp -rp "$PhysXRoot/PxShared/include/." "$OutputFolder/include/" || exit 1

# License file for redistribution
cp -p "$PhysXRoot/README.md" "$OutputFolder/License.txt" 2>/dev/null || true

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "PhysX 3.4 has been built and installed to:"
echo "  $OutputFolder"
echo ""
