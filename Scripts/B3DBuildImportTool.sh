#!/bin/sh

. ./B3DBuildCommon.sh
CMakeBuildConfig="${2:-Release}"

cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencyBuilds
cd DependencyBuilds

mkdir -p B3DImportTool
cd B3DImportTool

cmake -S "$CurrentDirectory/.." -B . -G "$CMakeGenerator" \
	-DCMAKE_INSTALL_PREFIX="$PlatformDependencyFolder/tools/bsfImportTool" \
	-DCMAKE_CXX_FLAGS="-DB3D_IS_IMPORT_TOOL" \
	-DB3D_BUILD_EXAMPLES=OFF \
	-DB3D_BUILD_TESTS=OFF || exit 1

cmake --build . --config "$CMakeBuildConfig" --target BansheeImportTool || exit 1
cmake --install . --config "$CMakeBuildConfig" || exit 1
