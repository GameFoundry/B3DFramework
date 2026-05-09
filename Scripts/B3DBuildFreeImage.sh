#!/bin/sh
# Copyright 2025-2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Builds FreeImage 3.18 from source"
echo ""

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Download/extract (SourceForge tarball)
if [ ! -d "FreeImage" ] || [ ! -f "FreeImage/Makefile" ]; then
	rm -rf FreeImage 2>/dev/null || true
	echo "Downloading FreeImage source..."
	curl -L -o FreeImage.zip "https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip"
	unzip -o FreeImage.zip

	# Apply C++17 compatibility patch
	cd FreeImage
	git init
	git add -A
	git commit -m "Initial"
	git apply "$CurrentDirectory/Patches/FreeImage.patch" || exit 1
	cd ..
fi
cd FreeImage

# Setup output folders
OutputFolder="$PlatformDependencyFolder/freeimg"

B3DCleanDependencyFolder "$OutputFolder"
mkdir -p "$OutputFolder/include"

# Platform-specific build
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	mkdir -p "$OutputFolder/lib/Debug"
	mkdir -p "$OutputFolder/lib/Release"
	mkdir -p "$OutputFolder/bin/Debug"
	mkdir -p "$OutputFolder/bin/Release"

	# Find MSBuild via vswhere
	VSWHERE="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
	VSINSTALL=$("$VSWHERE" -latest -requires Microsoft.Component.MSBuild -property installationPath)
	MSBUILD="$VSINSTALL/MSBuild/Current/Bin/MSBuild.exe"

	if [ ! -f "$MSBUILD" ]; then
		echo "[Error] MSBuild not found. Please install Visual Studio with C++ workload."
		exit 1
	fi

	# Build with MSBuild (VS 2017 solution, override SDK to latest). The FreeImage solution
	# produces a DLL plus import lib — both must ship: the .lib for link-time resolution,
	# the .dll for runtime loading.
	"$MSBUILD" FreeImage.2017.sln -p:Configuration=Release -p:Platform=x64 -p:PlatformToolset=v143 -p:WindowsTargetPlatformVersion=10.0 || exit 1
	"$MSBUILD" FreeImage.2017.sln -p:Configuration=Debug -p:Platform=x64 -p:PlatformToolset=v143 -p:WindowsTargetPlatformVersion=10.0 || exit 1

	cp -p Dist/x64/FreeImage.lib "$OutputFolder/lib/Release/"
	cp -p Dist/x64/FreeImaged.lib "$OutputFolder/lib/Debug/"
	cp -p Dist/x64/FreeImage.dll "$OutputFolder/bin/Release/"
	cp -p Dist/x64/FreeImaged.dll "$OutputFolder/bin/Debug/"

elif [[ "$Platform" == "darwin"* ]]; then
	mkdir -p "$OutputFolder/lib"

	make -f Makefile.osx clean 2>/dev/null || true
	make -f Makefile.osx || exit 1

	cp -p Dist/libfreeimage.a "$OutputFolder/lib/"

elif [[ "$Platform" == "linux-gnu"* ]]; then
	mkdir -p "$OutputFolder/lib"

	make -f Makefile.gnu clean 2>/dev/null || true
	make -f Makefile.gnu CFLAGS="-fPIC" CXXFLAGS="-fPIC" || exit 1

	cp -p Dist/libfreeimage.a "$OutputFolder/lib/"
fi

# Copy headers
cp -p Source/FreeImage.h "$OutputFolder/include/"

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "FreeImage has been built and installed to:"
echo "  $OutputFolder"
echo ""
