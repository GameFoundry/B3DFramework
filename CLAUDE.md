# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

bsf (Banshee 3D Framework) is a high-performance, multi-threaded game engine framework written in C++17. The framework uses a plugin-based architecture with support for multiple rendering backends (Vulkan, DirectX 12), physics (PhysX), audio (OpenAudio, FMOD), and includes a C# scripting system.

## Build System

The project uses CMake (3.31.0+) as its build system. All builds are configured from the repository root.

### Building

```bash
# Create build directory
mkdir Build
cd Build

# Generate build files (example for Visual Studio 2017 64-bit)
cmake -G "Visual Studio 15 2017 Win64" ../

# Build the project using generated files
cd Build
cmake -- build .

# Build output goes to build/bin
```

### Build Targets

- **bsf** - Framework dynamic library
- **UnitTestRunner** - Running unit tests

### Testing

```bash
# Build test runner
cmake --build . --target UnitTestRunner --config RelWithDebInfo

# Run test runner
./UnitTestRunner.exe
```

## Architecture

### Repository Structure

```
Framework/
├── Source/			  	# Framework source code
│   └── Tests/        		# Framework unit tests
├── Documentation/    		# User manuals and API documentation
├── Data/              		# Built-in assets and resources
│   └── Raw/           		# Non-imported data files
│   └── Shaders/   			# BSL shader files
├── Tools/		   		# Standalone applications
│   └── BansheeCodeGenerator/  	# Clang-based C++ code generator
│   └── BansheeDocGenerator/  	# API documentation generator
│   └── BansheeForge/  		# CI system
└── Build/                 		# Generated build files (not in repo)
```

### Key Design Patterns

- **Component-Based Architecture**: GameObject-like `SceneObject` with attachable `Component` classes
- **Module System**: Core systems are modules (singletons accessed via `GetModuleName()` if provided or `ModuleName::Instance()` otherwise)
- **Handle System**: Type-safe handles (`HSceneObject`, `HRigidbody`, etc.) wrap shared pointers
- **Resource System**: Resources (`Mesh`, `Texture`, `Material`) are loaded/managed centrally
- **Multi-Threading**: Task-based parallelism throughout the engine, internally using fibers rather than threads (See Scheduler)
- **Event System**: Components expose events (e.g., `OnCollisionBegin.Connect(callback)`)
- **Memory allocation**: `B3DNew`/`B3DDelete` instead of `new/delete`, `B3DAllocate/B3DFree` instead of `malloc/free`

## Code Style and Conventions

### Naming Conventions

- **Classes/Structs**: Capital case, no abbreviations (e.g., `SceneObject`, `Rigidbody`, 'cutRangeCount' vs 'numCutRanges', named index variable in loops, e.g. `elementIndex` vs `i`)
- **Methods**: Capital case, no abbreviations (e.g., `SetPosition()`, `GetVelocity()`)
- **Member variables**: Capital case for public fields (e.g., `Spring.Stiffness`), `m` prefix for non-public fields (e.g. `Spring.mInternalData`)
- **Local variables**: camelCase with full names, no abbreviations (e.g., `rigidbodySceneObject`, not `rbSO`)
- **Lambdas**: Prefix with `fn`, explicit captures only (e.g., `auto fnProcess = [this, &data](int x) { ... }`)
- **Handles**: Prefix with `H` (e.g., `HSceneObject`, `HRigidbody`)
- **Smart pointers**: `TShared<T>` for shared pointers, `TUnique<T>` for unique pointers. `B3DMakeShared` or `B3DMakeSharedFromExisting` to create shared pointer, `B3DMakeUnique` to create unique pointer.
- **Constants**: `k` prefix (e.g., `Vector3::kZero`, `Quaternion::kIdentity`)
- **Code documentation**: The codebase consistently uses Doxygen-style documentation

### API Patterns

- **Logging**: `B3D_LOG(Level, LogCategory, "message {0}", param)` - never use `GetDebug().LogDebug()`

### Comments

- When writing new implementation comments do not narrate the rationale for an edit or reference what used to be there - that belongs in the commit message, not the source. 
- Keep existing implementation comments. If copying or refactoring code, keep the comment if still relevant. This also applies to TODO comments.
- Write Doxygen API documentation on classes/methods/fields. Write smart and concise documentation that does not include implementation details, and also doesn't repeat what is obvious from method/class name or signature, but instead tells you how to use the API.
- Keep implementation comments to a minimum:
  - You can split up larger code blocks into meaningful sections using an implementation comment
  - You can comment on non-obvious constraints, workarounds for bugs or other things that might be missed without the comment
  - Otherwise use meaningful variable names so the code is self-documenting, avoid abbreviations

## Other notes

- **Exclusions**: Never modify or include in refactors, code in Dependencies or ThirdParty folders.
- Don't surround single line blocks with {}
- Never use dynamic_cast
- Use Edit tool for any non-trivial file modifications
- Only use Bash for actual terminal operations (git, cmake, build commands)
- Avoid sed/awk/perl for inserting/replacing code blocks
- There's a file modification bug in Claude Code. The workaround is: always use complete absolute Windows paths with drive letters and backslashes for ALL file operations.
- Be aware that the first Edit attempt might fail due to hidden whitespace/line ending differences. If Edit fails, use cat -A to inspect the actual bytes, but then trust what Read shows you for the replacement
- Always use tabs, not spaces. This includes all comments.
- When creating new files make sure to add a copyright notice as in the other files, with the current year updated
- Shared pointers (TShared) should be returned by const & whenever possible
- When performing refactors (either simple renames, or more architectural changes) always look up the modified types in the .md manuals in Documentation/docs, and update appropriate manuals.
