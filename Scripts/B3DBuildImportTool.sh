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

cmake $CMakeGenerator -DCMAKE_INSTALL_PREFIX="$PlatformDependencyFolder/tools/bsfImportTool" -DCMAKE_CXX_FLAGS="-DB3D_IS_IMPORT_TOOL" "$CurrentDirectory/.."

cmake --build . --config $CMakeBuildConfig --target bsfImportTool
cmake --build . --config $CMakeBuildConfig --target install
