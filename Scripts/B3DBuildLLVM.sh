#!/bin/sh

. ./B3DBuildCommon.sh

echo "Builds LLVM with Clang for purposes of B3DCodeGen"

# Create intermediate folders
cd ..

mkdir -p Intermediate
cd Intermediate

mkdir -p DependencySources
cd DependencySources

# Clone or select the documented LLVM release. Keep reruns pinned instead of pulling into a detached tag.
LLVM_VERSION="llvmorg-20.1.7"

if [ -d "llvm/.git" ]; then
    cd llvm
    git fetch --tags
    git reset --hard
    git checkout --detach "$LLVM_VERSION" || exit 1
else
    git clone https://github.com/llvm/llvm-project.git llvm
    cd llvm
    git checkout --detach "$LLVM_VERSION" || exit 1
fi

# Setup LLVM output folder
LLVMOutputFolder="$PlatformDependencyFolder/LLVM"

B3DCleanDependencyFolder "$LLVMOutputFolder"

mkdir -p build
cmake -S llvm -B build -G "$CMakeGenerator" -DLLVM_ENABLE_PROJECTS="clang" -DCMAKE_INSTALL_PREFIX="$LLVMOutputFolder" -DCMAKE_BUILD_TYPE=Release || exit 1
cmake --build build --target install --config Release || exit 1

echo -e "\nBuild complete. Set clang_INSTALL_DIR in B3D CMake to '$LLVMOutputFolder' to link against the built libraries.\n\n"
echo "IMPORTANT:"
echo " - Use '$LLVMOutputFolder/bin/llvm-config --libs' to determine which LLVM libraries your project needs to link against."
echo " - Use '$LLVMOutputFolder/bin/llvm-config --system-libs' to determine which OS libraries your project needs to link against."
echo " - Visit '$LLVMOutputFolder/lib' to determine which Clang libraries your project needs to link against (all libraries with a clang prefix)."
echo " - Use '$LLVMOutputFolder/bin/llvm-config --cxxflags' to determine which flags need to be passed to the compiler when linking against LLVM/Clang libraries."
echo " - If in doubt, check existing LLVM solution executable projects (e.g. 'clang-check') and see which libraries they link against and which preprocessor defines they set."



