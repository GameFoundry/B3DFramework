#!/bin/sh

CurrentDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PlatformDependencyFolder="$CurrentDirectory/../Dependencies"

# Target platform selection. Pass `--target <platform>` to cross-compile a dependency for a
# non-host platform; with no flag we build for the host (derived from $OSTYPE). Parsing is
# done without consuming the caller's positional parameters (these scripts are sourced), so
# per-dependency scripts keep their own argv intact.
Platform=""
_expectTarget=0
for _arg in "$@"; do
	if [ "$_expectTarget" -eq 1 ]; then
		Platform="$_arg"
		_expectTarget=0
		continue
	fi
	case "$_arg" in
		--target) _expectTarget=1 ;;
		--target=*) Platform="${_arg#*=}" ;;
	esac
done
Platform="${Platform:-$OSTYPE}"

# Extra arguments forwarded to every dependency's `cmake` configure step. Empty for the
# built-in host platforms; a platform fragment (sourced below) may set it, e.g. to inject a
# cross-compilation toolchain file. Left intentionally unquoted at the call sites so multiple
# arguments word-split correctly.
B3D_EXTRA_CMAKE_ARGS="${B3D_EXTRA_CMAKE_ARGS:-}"

if [[ $Platform == "win32" || $Platform == "msys" ]]; then
	DefaultCMakeGenerator="Visual Studio 17 2022"
	SharedLibraryExtension=".dll"
	StaticLibraryExtension=".lib"
	ImportLibraryExtension=".lib"
	StaticLibraryPrefix=""
	ExecutableExtension=".exe"
elif [[ $Platform == "darwin"* ]]; then
	DefaultCMakeGenerator="Ninja Multi-Config"
	SharedLibraryExtension=".dylib"
	StaticLibraryExtension=".a"
	ImportLibraryExtension=""
	StaticLibraryPrefix="lib"
	ExecutableExtension=""
elif [[ $Platform == "linux-gnu"* ]]; then
	DefaultCMakeGenerator="Ninja Multi-Config"
	SharedLibraryExtension=".so"
	StaticLibraryExtension=".a"
	ImportLibraryExtension=""
	StaticLibraryPrefix="lib"
	ExecutableExtension=""
else
	# Non-built-in target: defer to a platform fragment that lives next to the platform's
	# overlay. The fragment must set DefaultCMakeGenerator and the *LibraryExtension / prefix
	# variables (and typically B3D_EXTRA_CMAKE_ARGS). Keeping it out-of-tree lets proprietary
	# console targets stay self-contained.
	PlatformBuildFragment="$CurrentDirectory/../Platform/$Platform/Scripts/B3DBuildPlatform.sh"
	if [ ! -f "$PlatformBuildFragment" ]; then
		echo "[Error] Unknown platform '$Platform'. Built-in values: win32, darwin, linux-gnu."
		echo "        For other targets, provide a build fragment at:"
		echo "        $PlatformBuildFragment"
		exit 1
	fi
	. "$PlatformBuildFragment"
fi

# CMake generator used by B3DBuild*.sh scripts. Override by exporting B3D_CMAKE_GENERATOR
# before running a script, e.g. `B3D_CMAKE_GENERATOR="Ninja" ./B3DBuildSnappy.sh`.
CMakeGenerator="${B3D_CMAKE_GENERATOR:-$DefaultCMakeGenerator}"

# Wipes a dependency output folder while preserving .reqversion
B3DCleanDependencyFolder() {
	FolderToClean="$1"
	if [ -z "$FolderToClean" ]; then
		echo "[Error] B3DCleanDependencyFolder called without a folder argument."
		return 1
	fi

	if [ ! -d "$FolderToClean" ]; then
		mkdir -p "$FolderToClean"
		return 0
	fi

	ReqVersionFile="$FolderToClean/.reqversion"
	if [ -f "$ReqVersionFile" ]; then
		SavedReqVersionFile=$(mktemp)
		cp "$ReqVersionFile" "$SavedReqVersionFile"
		rm -rf "$FolderToClean"
		mkdir -p "$FolderToClean"
		cp "$SavedReqVersionFile" "$ReqVersionFile"
		rm -f "$SavedReqVersionFile"
	else
		rm -rf "$FolderToClean"
		mkdir -p "$FolderToClean"
	fi
}