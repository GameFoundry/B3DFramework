#!/bin/sh

. ./B3DBuildCommon.sh

# Determine platform
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	echo "Building for Windows."
    echo ""
    echo "IMPORTANT: "
    echo " - Make sure to install all prerequisites as specified here: https://github.com/dotnet/runtime/blob/main/docs/workflow/requirements/windows-requirements.md"
    echo " - If you receive an error that .NET runtime is in use, shut down all programs that may use it (such as Visual Studio)"
    echo " - If you receive an error that files cannot be created or opened. Try enabling long paths in the OS and Git."
    echo "   - On Windows edit registry HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled 1 (DWORD)"
    echo "   - On Git: git config --system core.longpaths true"
    echo "   - If the error keeps occuring, you can move the script directory to drive root, and then copy dependencies to correct location after the build."
    echo " - If you receive a ILLinker error during compilation, try running the script again until it succeeds"
    echo ""
    sleep 2
else
	echo "[Error] This build script is not currently supported on the current platform: $Platform."
	exit 1
fi

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Pinned .NET runtime version (patch must match this version)
DOTNET_VERSION="v9.0.11"

# Clone
if [ -d "DotNetRuntime" ]; then
    cd DotNetRuntime
    git fetch --tags
    git checkout $DOTNET_VERSION
else
    git clone https://github.com/dotnet/runtime.git DotNetRuntime
    cd DotNetRuntime
    git checkout $DOTNET_VERSION
	git apply "$CurrentDirectory/Patches/Mono.patch" || exit 1
fi

# Setup Mono output folders
MonoOutputFolder="$PlatformDependencyFolder/DotNETCoreMono"

rm -rf $MonoOutputFolder
mkdir -p "$MonoOutputFolder/include/"
mkdir -p "$MonoOutputFolder/lib/"
mkdir -p "$MonoOutputFolder/bin/"
mkdir -p "$MonoOutputFolder/bin/Assemblies/"

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    mkdir -p "$MonoOutputFolder/lib/Release/"
    mkdir -p "$MonoOutputFolder/lib/Debug/"

    mkdir -p "$MonoOutputFolder/bin/Release/"
    mkdir -p "$MonoOutputFolder/bin/Debug/"
fi

# Essential assemblies for basic C# scripting (minimal set).
RequiredAssembliesFile="$CurrentDirectory/../Source/CMake/RequiredNETAssemblies.txt"
ESSENTIAL_ASSEMBLIES=()
while IFS= read -r line || [ -n "$line" ]; do
    line="${line%%#*}"                              # strip trailing comment
    line="$(printf '%s' "$line" | tr -d '[:space:]')" # trim whitespace (names have none)
    [ -z "$line" ] && continue
    ESSENTIAL_ASSEMBLIES+=("$line")
done < "$RequiredAssembliesFile"

# Helper to copy only essential assemblies from a directory (dll, pdb, and xml)
copy_essential_assemblies()
{
    local sourceDir="$1"
    local destDir="$2"

    for assembly in "${ESSENTIAL_ASSEMBLIES[@]}"; do
        # Copy .dll
        if [ -f "$sourceDir/$assembly.dll" ]; then
            cp -p -- "$sourceDir/$assembly.dll" "$destDir/"
        fi
        # Copy .pdb (debug symbols)
        if [ -f "$sourceDir/$assembly.pdb" ]; then
            cp -p -- "$sourceDir/$assembly.pdb" "$destDir/"
        fi
        # Copy .xml (documentation)
        if [ -f "$sourceDir/$assembly.xml" ]; then
            cp -p -- "$sourceDir/$assembly.xml" "$destDir/"
        fi
    done
}

# Helper to copy libraries for different configurations/platforms/architectures
copy_libraries()
{
    local ilDir="artifacts/bin/mono/$1.$2.$3/IL"
    local runtimeDir="artifacts/bin/runtime/net9.0-$1-$3-$2"

    # Copy only essential assemblies instead of everything
    copy_essential_assemblies "$ilDir" "$MonoOutputFolder/bin/Assemblies/"
    copy_essential_assemblies "$runtimeDir" "$MonoOutputFolder/bin/Assemblies/"

    # Copy native runtime library
    cp -p -- "artifacts/bin/mono/$1.$2.$3/coreclr$SharedLibraryExtension" "$MonoOutputFolder/bin/$4"

    # Copy the AOT cross compiler (built via the BuildMonoAOTCrossCompiler property).
    # It lives under cross/<rid>/ where rid maps windows->win-<arch>, else <os>-<arch>.
    local ridOs="$1"
    if [[ "$1" == "windows" ]]; then ridOs="win"; fi
    local crossExe="artifacts/bin/mono/$1.$2.$3/cross/$ridOs-$2/mono-aot-cross$ExecutableExtension"
    if [ -f "$crossExe" ]; then
        cp -p -- "$crossExe" "$MonoOutputFolder/bin/$4"
    else
        echo "[Warning] AOT cross compiler not found at $crossExe"
    fi

    if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
        cp -p -- "artifacts/obj/mono/$1.$2.$3/out/lib/coreclr.import.lib" "$MonoOutputFolder/lib/$4"

        if [[ "$3" == "Debug" ]]; then
            cp -p -- "artifacts/bin/mono/$1.$2.$3/coreclr.pdb" "$MonoOutputFolder/bin/$4"
        fi
    fi
}


# Request the Mono AOT cross compiler. This is an MSBuild property; we pass it via the environment
export BuildMonoAOTCrossCompiler=true

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
    # MAX_PATH workaround: Native CMake/Ninja build's intermediate object paths (artifacts\obj\...\<hash>\*.obj) can easily
	# overflow Windows' 260-char limit for a few files, and cl.exe/rc.exe then fail intermittently with
    # "C1083: Cannot open compiler generated file: ''". Map the clone to a short virtual drive and
    # build from there so every path stays short. subst is per-session and needs no admin; the
    # mapping is torn down on EXIT even if the build fails. (dotnet/runtime recommends a short build
    # path over relying on LongPathsEnabled, which cl.exe does not honor reliably.)
    cloneUnixPath="$PWD"
    cloneWinPath="$(pwd -W | tr '/' '\\' 2>/dev/null)"
    substDrive=""
    for L in B Y X W V U T S R Q P O N M; do
        if subst "${L}:" "$cloneWinPath" >/dev/null 2>&1; then
            substDrive="$L"
            break
        fi
    done

    cleanup_subst() {
        if [ -n "$substDrive" ]; then
            cd "$cloneUnixPath" 2>/dev/null
            MSYS2_ARG_CONV_EXCL='*' MSYS_NO_PATHCONV=1 subst "${substDrive}:" /d >/dev/null 2>&1
        fi
    }
    trap cleanup_subst EXIT

    if [ -n "$substDrive" ] && cd "/$(printf '%s' "$substDrive" | tr 'A-Z' 'a-z')" 2>/dev/null; then
        echo "[Info] MAX_PATH workaround: building via $substDrive:\\ -> $cloneWinPath"
    else
        echo "[Warning] Could not set up the short-path (subst) MAX_PATH workaround; building from"
        echo "          the deep clone path. If the native build fails with 'C1083: Cannot open"
        echo "          compiler generated file', enable OS/Git long paths or move the clone nearer"
        echo "          the drive root."
        cd "$cloneUnixPath"
    fi

    # Build & copy debug
    ./build.cmd mono+libs -configuration Debug
    copy_libraries windows x64 Debug Debug/

    # Build & copy release
    ./build.cmd mono+libs -configuration Release
    copy_libraries windows x64 Release Release/

    # Copy includes
    cp -a -r -- "artifacts/bin/mono/windows.x64.Debug/include/mono-2.0/." "$MonoOutputFolder/include/"
elif [[ "$Platform" == "darwin"* ]]; then
    # Build & copy release
    ./build.sh mono+libs -configuration Release
    copy_libraries osx arm64 Release ""

    # Copy includes
    cp -a -r -- "artifacts/bin/mono/osx.arm64.Release/include/mono-2.0/." "$MonoOutputFolder/include/"
else
    # Build & copy release
    ./build.sh mono+libs -configuration Release
    copy_libraries linux x64 Release ""

    # Copy includes
    cp -a -r -- "artifacts/bin/mono/linux.x64.Release/include/mono-2.0/." "$MonoOutputFolder/include/"
fi

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "Mono has been built and installed to:"
echo "  $MonoOutputFolder"
echo ""
