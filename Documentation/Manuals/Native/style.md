Coding style							{#style}
===============
[TOC]

This document provides information on the coding style that should be used when contributing to Banshee. Although you should remember the notes in this document, you can get away without remembering all of this by just following the style in existing code. This guide applies only to C++ code.

Guidelines
 - Documentation
   - Write JavaDoc style documentation.
   - Document every class/structure/enum.
   - Document every non-trivial method (i.e. anything that you can't deduce what it does from its name).
   - You are allowed to omit documenting method parameters if its clear from the general method descriptions what they do.
   - Documentation of fields in classes and enums is optional if it's clear what they do or their documentation is available elsewhere.
   - Write RELEVANT documentation. For example `/** Gets value */ getValue(); ` is pointless documentation. Try to give the user a deeper understanding of what the code does from what he can already deduce by reading the name of the method.
   - Use \@`addtogroup` to group classes into relevant sections.
    - Use separate groups for code that can be considered internal (i.e. more specialized, not meant for normal use) to clean up the user-facing documentation.
	- Use \@`name Internal` to group internal methods within a class for a similar purpose.
   - Documentation should produce no Doxygen warnings when generating.
   - If it is not obvious for which thread is a class/method meant to be used on, note it in the documentation.
 - Prefixes/Sufixes
   - Use "_" prefix to identify a method is internal. A method is internal if it is public but it is not meant to be used by the normal user. You can alternatively use a private method and C++ `friend` syntax. Although the former approach is sometimes preferable to avoid exposing too much of the interface.
   - Use "Core" suffix to note that a class is used on the core thread, but also has a sim-thread counterpart. The sim-thread counterpart should share the name of the class but without the "Core" suffix.
   - Use "Base" suffix to mark base classes unless a more generalized name is more appropriate.
   - Prefix templated base classes/structs with a "T" unless a more generalized name is more appropriate.
   - Use "C" prefix for component implementations.
 - Naming
  - Methods, fields and local variables should be written in camelCase with first letter as lowercase (e.g. `void myMethod(); int myVar;`).
  - Class/struct/enum names should be in PascalCase with first letter as uppercase (e.g. `class MyClass`).
  - Non-public class/struct fields should be prefixed with "m", followed by a capital first letter (e.g. `int mClassMemberField;`).
  - Non-constant global variables and methods should be prefixed with an "g", followed by a capital first letter (e.g. `void gApplication();`).
  - Constant global variables should be written in all caps with underscores separating the words (e.g. `int NUM_ELEMENTS = 5;`).
  - Macros should be written in all caps with underscores separating the words (e.g. \#`define BS_EXCEPT`).
  - Static non-constant variables should be prefixed with an "s", followed by a capital first letter (e.g. `bool sIsInitialized;`).
  - Static constant variables should be written in all caps with underscores separating the words (e.g. `int NUM_ELEMENTS = 5;`).
 - Brackets/Spacing
  - Use indentation for new blocks
  - Use tabs for one indentation step
  - Brackets {} should always be on their own line unless they're empty or for small method definitions directly in class declaration
  - Conditionals or loops that have just one line of content should omit the brackets
  - Always put a space after a semicolon (e.g. for(int i = 0; i < 5; i++)).
  - Always put a space between operators (e.g. 5 + 2).
  - Separate meaningful parts of the code in a method with empty lines
  - Always but a blank line after a block {}
  - No single line should be longer than column 124 (set up a guideline in your editor)
 - If method doesn't modify data, always mark it as const (getters especially)
 - Always pass non-primitive parameters by reference unless `null` is a valid value in which case use a pointer
 - Reference/pointer parameters that won't be modified by a function should always be `const`
 - Prefer using `enum class` to `enum`
 - Use built-in typedefs for standard library containers (e.g. `Vector`) and shared pointers (`SPtr`).
 - Don't allocate memory using `new/delete` or `malloc/free`, instead use Banshee's allocators
 - Carefully construct class interfaces, keeping only relevant methods public and everything else private/protected/internal
 - No code warnings under default compiler warning settings are allowed. Fix all your warnings or if absolutely not possible isolate that bit of code and disable that specific warning (but only in that bit of code).
 - Prefer putting helper types (e.g. structs, enums) to the bottom of the include file if possible and keep the first class in the file the primary class
 - Put everything in BansheeEngine namespace unless specializing code for a third party library
 - Never use `using namespace` in a header
 - Public fields should be avoided (except for simple structures) and instead getter/setter methods should be provided
 - In a class list elements in this order:
    - typedefs
    - nested classes/structures
	- public methods
	- public fields
	- internal methods
	- internal fields
	- protected methods
	- protected fields
	- private methods
    - private fields