# CLAUDE.md — Framework/Source/CMake

Guidance for working with the CMake helper modules in this folder. These rules are derived from the
existing files (e.g. `Globbing.cmake`, `Utility.cmake`, `PostBuild.cmake`, `MonoAot.cmake`); match
them when adding or editing modules here.

## What lives here

- `*.cmake` — reusable helper modules, each defining `B3D…`-prefixed functions/macros. Pulled in
  through `Prerequisites.cmake` (or included directly where needed). Add a new module's `include()`
  to `Prerequisites.cmake` rather than scattering includes.
- `*.cmake.in` / `*.h.in` — `configure_file` templates (`@VAR@` / `#cmakedefine01` substitution).
- `Scripts/` — `cmake -P` worker scripts and their `.in` templates.
- `Modules/` — `find_package` modules.
- Plain-text data consumed by the modules (e.g. `RequiredNETAssemblies.txt`).

## Formatting

- **Indent with tabs**, one tab per nesting level. Never spaces. (Verify with `cat -A`: indentation
  shows as `^I`.)
- Continuation/wrapped `message()` strings are indented to align under the opening argument.
- Close scopes with the bare form: `endfunction()`, `endif()`, `endforeach()` — don't repeat the
  condition (older files like `Utility.cmake` occasionally do, e.g. `endif(... STREQUAL ...)`; do
  not copy that, the bare form is preferred).

## Naming

- **Functions and macros:** `B3D` + `PascalCase`, verb-first, no abbreviations — e.g.
  `B3DGlobSourceFiles`, `B3DSetUpPostBuildAndInstallSteps`, `B3DAddPostBuildAOTCompileIfNeeded`.
- **Cache / option / global variables:** `B3D_UPPER_SNAKE_CASE` — e.g. `B3D_MONO_AOT`,
  `B3D_FRAMEWORK_SOURCE_FOLDER`. `mark_as_advanced()` cache variables that users shouldn't normally
  touch.
- **Local variables and function parameters:** `camelCase` — e.g. `sourceFiles`, `currentPath`,
  `assemblyNames`, `compilerExecutablePath`. Spell names out; avoid abbreviations like `dir`/`asm`/
  `src` (prefer `folder`, `assembly`, `source`).
- **Output parameters:** prefix with `out` (`outSourceFiles`, `outFolderNames`) or name them
  `…Variable` (`outputVariable`), and write back with `set(${outVar} … PARENT_SCOPE)`.
- **`configure_file` substitution variables:** `Prefix_UPPER_SNAKE` — e.g. `RunAssetImport_INPUT_FOLDER`.
- **`-D` arguments to `cmake -P` worker scripts:** `UPPER_SNAKE_CASE` — e.g.
  `COMPILER_EXECUTABLE_PATH`, `ASSEMBLY_FILE_NAME`.

## Documentation comments

Document every `B3D…` function/macro with a comment block immediately above it:

1. One or more sentences describing what it does (and notable side effects / ordering requirements).
2. A blank `#` separator line.
3. Tab-aligned `# @param<TAB>name<TAB>description` lines, one per parameter.
4. Cross-reference other parameters in descriptions with `@p name`.

```cmake
# Parses all source files in the provided folder recursively, and outputs the list of files in @p outSourceFiles.
# For each sub-folder found it also creates a source group with the folder hierarchy.
#
# @param	parentPath		Folder path in which to perform the search. All output paths will be relative to this path.
# @param	outSourceFiles	List of all found source files, relative to @p parentPath.
function(B3DGlobSourceFiles parentPath outSourceFiles)
```

- A bare one-line file-header comment at the top of the module is fine for newer modules
  (`# Build-time Mono AOT compilation helpers.`). Some older modules use a `####…` banner block;
  don't add new banners — prefer the single descriptive line.
- For `cmake -P` worker scripts (driven by `-D` args, not function parameters), document the inputs
  with a `Required -D arguments:` / `Optional:` block instead of `@param` — see
  `RunMonoAotCompiler.cmake`.
- Inline comments are short and lowercase: `# strip trailing comment`.

## Options

Declare build options with `set`, not `option()`:

```cmake
set(B3D_MY_FEATURE OFF CACHE BOOL "If <true|enabled>, <what it does>. <Caveats / requirements>.")
```

- Default `OFF` for opt-in features. Write a full, self-contained help string (the existing options
  read as complete sentences).
- Surface a C/C++ toggle through `B3DFrameworkConfig.h.in` with `#cmakedefine01 B3D_MY_FEATURE`, and
  keep a short comment next to it explaining the runtime effect.

## Patterns and idioms

- **Guard clauses:** early `return()` for no-op / disabled cases (`if(NOT B3D_MONO_AOT) return() endif()`),
  rather than wrapping the whole body in an `if`.
- **Degrade gracefully:** when an optional dependency/tool is missing, `message(WARNING …)` and
  `return()` instead of hard-failing configuration; reserve `message(FATAL_ERROR …)` for genuinely
  required inputs. Prefix grouped diagnostics with a `[Tag]`, e.g. `[MonoAOT]`.
- **Config-matched paths in multi-config generators:** resolve Debug/Release at build time with
  generator expressions — `$<IF:$<CONFIG:Debug>,Debug,Release>` — so a single Visual Studio
  generation covers all configurations. Don't branch on `CMAKE_BUILD_TYPE` for per-config output
  paths.
- **Custom commands:** pass `VERBATIM`, give a human-readable `COMMENT`, and use
  `COMMAND_EXPAND_LISTS` when arguments contain generator-expression lists.
- **Single source of truth:** data shared with other tooling (e.g. the BCL assembly list shared with
  `Framework/Scripts/B3DBuildMono.sh`) lives in a plain-text file read by both sides
  (`RequiredNETAssemblies.txt`); don't duplicate such lists inline.
- **Reference engine variables**, don't hardcode paths: `B3D_FRAMEWORK_SOURCE_FOLDER`,
  `PROJECT_BINARY_DIR`, `CMAKE_EXECUTABLE_SUFFIX`, `<Dependency>_INSTALL_DIR`, etc.

## Out of bounds

- Do **not** edit `.version` / `.reqversion` files under `Framework/Dependencies/…`. They are the
  internal binary-dependency tracking mechanism, ticked automatically by the dependency upload
  script — never by hand or by build logic.
