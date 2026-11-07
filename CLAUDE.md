# CLAUDE.md

## Project Overview

bsf (Banshee 3D Framework) is a multi-threaded game engine framework written in C++17. It uses a plugin-based architecture with rendering backends (Vulkan, DirectX 12, Metal), physics (PhysX), audio (OpenAudio, FMOD), and a C# scripting system.

## Build System

CMake 4.2 or newer. Windows builds require Visual Studio 2026 with the v145 toolset. Configure from the repository root into `Build/`:

```bash
cmake -S . -B Build -G "Visual Studio 18 2026" -A x64
cmake --build Build --config RelWithDebInfo
```

Binaries go to `Build/bin/x64/<Config>`. Main targets are **bsf** (framework library) and **UnitTestRunner** (unit tests; prefer the `run-tests` skill to run it).

When running any engine executable unattended (examples, UnitTestRunner, BansheeCookTool), always pass `--debug.DisableErrorDialogs=true`. Without it, Debug-build asserts and aborts pop up interactive dialogs that block the run until a human dismisses them; with it, errors go to stderr and the process fails fast with a proper exit code.

## Memory Allocation

- `B3DNew`/`B3DDelete` instead of `new/delete`, `B3DAllocate/B3DFree` instead of `malloc/free`
- Avoid dynamic memory allocations whenever possible
- Use FrameAllocator for temporary heap allocations
- Function-local temporary containers use the frame allocator variants (FrameVector, FrameUnorderedMap, FrameSet, etc.) instead of Vector/UnorderedMap/Set, inside a FrameAllocatorScope so their memory is released when the function ends
- Re-use temporary containers (e.g. Vector, TArray). Allocate them as class members instead of locally in functions, so their memory can be re-used.
- Use TInlineArray when allocating past a certain count is unlikely

## Code Style and Conventions

Match the surrounding code. The rules below are the ones most often gotten wrong.

### Naming

- **No abbreviations** in any name. Spell the type noun out: `Parameters` not `Params`, `Information` not `Info`, `Description` not `Desc`. Name loop indices (`elementIndex`, not `i`) and counts (`cutRangeCount`, not `numCutRanges`).
- **Locals** are camelCase and named for the thing they hold or build, including that noun: `TextureCreateInformation inputTextureCreateInformation;`, `inputTexture` (not `inputInfo`, not bare `input`), `rigidbodySceneObject` (not `rbSO`).
- **Output parameters**: prefix with `out` for any parameter the function writes through — non-const reference, pointer, or array out-param (e.g., `outVariation`, `outRGBA`, `outSource`).
- **Lambdas**: prefix with `fn`, explicit captures only (e.g., `auto fnProcess = [this, &data](int x) { ... }`).
- **Smart pointers**: `TShared<T>` / `TUnique<T>`, created with `B3DMakeShared` / `B3DMakeSharedFromExisting` / `B3DMakeUnique`. Return `TShared` by const reference whenever possible.

### Code

- All types must provide constructors; do not use aggregate brace initialization anywhere.
- Never use `dynamic_cast` (the engine is built without C++ RTTI on some platforms).
- Never delete a field from an RTTI type or change what its ID stores. Comment out the old `B3D_RTTI_MEMBER*` line so it stays visible that the ID was used, and give any replacement a new ID. Never re-use an ID, because existing saved data still holds values under it.
- Don't surround single-line blocks with `{}`.
- Do not introduce private or internal helper methods with only one caller; inline the code at the call site with a short implementation comment instead.
- Prefer keeping statements, function calls, declarations, and conditions on a single line, even when long. Do not enforce a fixed maximum line length.
- Separate logical sub-steps with a blank line — e.g. fill a descriptor / `…CreateInformation`, then a blank line before the `Create…` call that consumes it; keep a block of local declarations apart from the statement that uses them.
- When creating new files add the copyright notice used in the other files, with the current year.

### Control flow

- When a function handles alternative cases, write them as explicit branches (`if`/`else`), not as an early `return` from one case followed by the other case's code after the `if`. Early returns are for guards: rejecting invalid input or nothing to do, before the function's main work.

### Comments

- When writing new implementation comments do not narrate the rationale for an edit or reference what used to be there - that belongs in the commit message, not the source.
- Keep existing implementation comments. If copying or refactoring code, keep the comment if still relevant. This also applies to TODO comments.
- Write Doxygen API documentation on classes/methods/fields. Write smart and concise documentation that does not include implementation details, and also doesn't repeat what is obvious from method/class name or signature, but instead tells you how to use the API.
- Do not mention the caller/user of a method/class when writing documentation. Each method/class should be treated as a black box that may be potentially used for other purpose than the originally intended one.
- Keep implementation comments to a minimum:
  - You can split up larger code blocks into meaningful sections using an implementation comment
  - You can comment on non-obvious constraints, workarounds for bugs or other things that might be missed without the comment
  - Otherwise use meaningful variable names so the code is self-documenting
  - A short trailing `//` may state a literal's or field's units/meaning (e.g. `outPixelFormat = PF_RG32U; // 64-bit block`, `B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gSize)  // decoded output size in pixels`)

## Architectural Changes

Don't localize a change just to keep it small. If reshaping a higher-level system yields a cleaner API, expand the scope and redesign it rather than bolting the feature on. Clean means clear method names, no duplicated logic, and symmetric paired operations (`Begin`/`End`, `Acquire`/`Release`, `Create`/`Destroy`).

## Answering Questions and Planning

- Be succinct: give the answer directly, note important gotchas, optionally a short example. No restating context, no exhaustive option surveys.
- When producing an implementation plan, if a big/pivotal decision arises — one the rest of the plan hinges on (e.g. a core data-flow choice, an interface boundary, a correctness assumption that determines whether the whole approach works) — stop and ask before finishing the plan. Do not bake the decision in as an assumption and present a completed plan around it.

## Agent Workflow

- Memory holds only knowledge that outlives a single task: user preferences, recurring traps, environment facts, and design rationale not visible in the code. Keep task progress, decisions and next steps in a task-specific .md file, not in memory. When a task finishes, reduce its notes to the lasting lessons or delete them.
- Do not create temporary files in the repository folder unless the location is already .gitignored. Put temporaries (scratch scripts, logs, task .md files) in `../Temp/<session name>/`, relative to the main repository checkout.
- Put non-temporary outputs that should not be committed (reports, mockups, comparison results) in `../Artifacts/<session name>/`.
- Create git worktrees under `../Worktrees/`, relative to the main repository checkout (e.g. `../Worktrees/Framework/<name>` for Framework branches).

## Other notes

- Never modify or include in refactors code in `Dependencies` or `ThirdParty` folders.
- When performing refactors (simple renames or architectural changes) look up the modified types in the manuals under `Documentation/Manuals/docs` and update them.
