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

### Debugging

When runtime debugging is required (breakpoints, call stacks, variable inspection, crash/assert investigation), use the `clion-debug` skill — it covers connecting to CLion's MCP servers, launching CLion if needed, building, and driving the debugger.

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
 - Avoid dynamic memory allocations whenever possible
 - Use FrameAllocator for temporary heap allocations
 - Re-use temporary containers (e.g. Vector, TArray). Allocate them as class members instead of locally in functions, so their memory can be re-used.
 - Use TInlineArray when allocating past a certain count is unlikely

## Code Style and Conventions

### Line wrapping

- Prefer keeping statements, function calls, declarations, and conditions on a single line, even when long.
- Only wrap a line when the unwrapped form is exceptionally difficult to read.
- Do not enforce a fixed maximum line length.

### Naming Conventions

- **Classes/Structs**: Capital case, no abbreviations (e.g., `SceneObject`, `Rigidbody`, 'cutRangeCount' vs 'numCutRanges', named index variable in loops, e.g. `elementIndex` vs `i`)
- **Methods**: Capital case, no abbreviations (e.g., `SetPosition()`, `GetVelocity()`)
- **Member variables**: Capital case for public fields (e.g., `Spring.Stiffness`), `m` prefix for non-public fields (e.g. `Spring.mInternalData`)
- **Local variables**: camelCase with full names, no abbreviations (e.g., `rigidbodySceneObject`, not `rbSO`)
- **Output parameters**: prefix with `out` for any parameter the function writes through — non-const reference, pointer, or array out-param (e.g., `outVariation`, `outRGBA`, `outSource`). This is an engine-wide convention.
- **Spell the type noun out**: `Parameters` not `Params`, `Information` not `Info`, `Description` not `Desc`; and name a local for the thing it holds or builds, including that noun — `TextureCreateInformation inputTextureCreateInformation;`, `inputTexture` (not `inputInfo`, not bare `input`); `gTextureCompressParameters` (not `gTextureCompressParams`)
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
- Do not mention the caller/user of a method/class when writing documentation. Each method/class should be treated as a black box that may be potentially used for other purpose than the originally intended one.
- Keep implementation comments to a minimum:
  - You can split up larger code blocks into meaningful sections using an implementation comment
  - You can comment on non-obvious constraints, workarounds for bugs or other things that might be missed without the comment
  - Otherwise use meaningful variable names so the code is self-documenting, avoid abbreviations
  - A short trailing `//` may state a literal's or field's units/meaning (e.g. `outPixelFormat = PF_RG32U; // 64-bit block`, `B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gSize)  // decoded output size in pixels`)

## Architectural Changes

When adding features or refactoring, don't localize a change just to keep it small — if reshaping a higher-level system yields a cleaner API, expand the scope to do it right.

- Don't be afraid to make API or higher-level architectural changes when they make the API cleaner; avoid contorting a new feature to fit an existing API that ends up with code smell.
- Prefer redesigning the API to fit the feature cleanly over bolting the feature onto the side of a system that wasn't shaped for it.
- Clean means easy to use and easy to understand: clear, non-convoluted method names that say what they do.
- Clean means no code duplication: factor shared logic into one place rather than copy-pasting across call sites.
- Clean means symmetry: paired operations match (e.g. `begin`/`end`, `acquire`/`release`, `create`/`destroy`).

## Planning

- When producing an implementation plan, if a big/pivotal decision arises — one the rest of the plan hinges on (e.g. a core data-flow choice, an interface boundary, a correctness assumption that determines whether the whole approach works) — stop and ask before finishing the plan. Do not bake the decision in as an assumption and present a completed plan around it. Surface it early so it can be settled first.

## Other notes

- **Exclusions**: Never modify or include in refactors, code in Dependencies or ThirdParty folders.
- Don't surround single line blocks with {}
- Separate logical sub-steps with a blank line — e.g. fill a descriptor / `…CreateInformation`, then a blank line before the `Create…` call that consumes it; keep a block of local declarations apart from the statement that uses them
- Never use dynamic_cast
- Always use tabs, not spaces. This includes all comments.
- When creating new files make sure to add a copyright notice as in the other files, with the current year updated
- Shared pointers (TShared) should be returned by const & whenever possible
- When performing refactors (either simple renames, or more architectural changes) always look up the modified types in the .md manuals in Documentation/docs, and update appropriate manuals.
