#!/bin/sh
# Copyright 2026 Marko Pintera. All rights reserved.

. ./B3DBuildCommon.sh

echo "Provides the DirectX Shader Compiler (DXIL and SPIR-V code generation)"
echo ""

# Pinned release. Windows and Linux use Microsoft's prebuilt archives, macOS has none and builds from the same tag.
DirectXShaderCompilerTag="v1.9.2609"
WindowsArchive="dxc_2026_09_29.zip"
LinuxArchive="linux_dxc_2026_09_28.x86_x64.tar.gz"
ReleaseUrl="https://github.com/microsoft/DirectXShaderCompiler/releases/download/$DirectXShaderCompilerTag"

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

OutputFolder="$PlatformDependencyFolder/DirectXShaderCompiler"

# Called only once the new files are ready, so a failed download or build leaves the existing copy in place
PrepareOutputFolder() {
	B3DCleanDependencyFolder "$OutputFolder" || exit 1
	mkdir -p "$OutputFolder/include" "$OutputFolder/lib" "$OutputFolder/bin" || exit 1
}

if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	rm -rf DirectXShaderCompiler
	mkdir -p DirectXShaderCompiler
	cd DirectXShaderCompiler

	curl -fL -o "$WindowsArchive" "$ReleaseUrl/$WindowsArchive" || exit 1
	# The archive uses backslash path separators, which unzip accepts with a warning (exit code 1)
	unzip -qo "$WindowsArchive"
	[ $? -le 1 ] || exit 1

	PrepareOutputFolder
	cp -r inc/* "$OutputFolder/include/" || exit 1
	cp lib/x64/dxcompiler.lib "$OutputFolder/lib/" || exit 1
	cp bin/x64/dxcompiler.dll bin/x64/dxil.dll bin/x64/dxc.exe "$OutputFolder/bin/" || exit 1
	cp LICENSE-* LICENCE-* "$OutputFolder/" || exit 1
elif [[ "$Platform" == "linux-gnu"* ]]; then
	rm -rf DirectXShaderCompiler
	mkdir -p DirectXShaderCompiler
	cd DirectXShaderCompiler

	curl -fL -o "$LinuxArchive" "$ReleaseUrl/$LinuxArchive" || exit 1
	tar -xzf "$LinuxArchive" || exit 1

	PrepareOutputFolder
	cp -r include/dxc/* "$OutputFolder/include/" || exit 1
	cp lib/libdxcompiler.so* lib/libdxil.so* "$OutputFolder/lib/" || exit 1
elif [[ "$Platform" == "darwin"* ]]; then
	if [ -d "DirectXShaderCompiler" ]; then
		cd DirectXShaderCompiler
		git fetch --tags
		git checkout $DirectXShaderCompilerTag
		git submodule update --init --recursive
	else
		git clone --recursive --branch $DirectXShaderCompilerTag https://github.com/microsoft/DirectXShaderCompiler.git DirectXShaderCompiler || exit 1
		cd DirectXShaderCompiler
	fi

	cmake -S . -B build -G "$CMakeGenerator" -C cmake/caches/PredefinedParams.cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_SPIRV_CODEGEN=ON \
		-DSPIRV_BUILD_TESTS=OFF \
		-DLLVM_INCLUDE_TESTS=OFF \
		-DHLSL_INCLUDE_TESTS=OFF \
		$B3D_EXTRA_CMAKE_ARGS || exit 1

	cmake --build build --config Release --target dxcompiler || exit 1

	# Multi-config generators put the output in a folder named after the configuration
	BuiltLibraryFolder="build/Release/lib"
	[ -d "$BuiltLibraryFolder" ] || BuiltLibraryFolder="build/lib"

	PrepareOutputFolder
	cp -r include/dxc/* "$OutputFolder/include/" || exit 1
	cp "$BuiltLibraryFolder"/libdxcompiler.dylib* "$OutputFolder/lib/" || exit 1
else
	echo "[Error] The DirectX Shader Compiler is host-only tooling, and this script doesn't support platform: $Platform."
	exit 1
fi

B3DStampDependencyBuild "$OutputFolder"

echo ""
echo "======================================================================"
echo "Build complete!"
echo "======================================================================"
echo ""
echo "DirectX Shader Compiler has been installed to:"
echo "  $OutputFolder"
echo ""
