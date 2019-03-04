Coding style							{#codeStyle}
===============
[TOC]

When making changes or additions that you plan on contributing to bs::f, you must follow the same coding style as the rest of the codebase. This document tries to list the most important aspects of the style.

**Spacing**
 - Use tabs instead of spaces for indentation
 - Always put a space after a semicolon (e.g. `for(int i = 0; i < 5; i++)`).
 - Always put a space between operators (e.g. `5 + 2`).
 - Separate meaningful parts of the code with empty lines
 - Always but a blank line after a block {}
 - No single line should be longer than column 124 (set up a guideline in your editor)
 
**Brackets**
 - Both opening and closing brackets should be in their own line
  - The exceptions are empty methods and small methods defined directly in class declarations
 - Conditionals or loops that have just one line should omit the brackets
  
**Naming**
 - Methods, fields and local variables must be in camelCase with first letter as lowercase (e.g. `void myMethod(); int myVar;`).
 - Class/struct/enum names must be in PascalCase with first letter as uppercase (e.g. `class MyClass`).
 - Non-public class/struct fields should be prefixed with "m", followed by a capital first letter (e.g. `int mClassMemberField;`).
 - Non-constant global variables and methods should be prefixed with an "g", followed by a capital first letter (e.g. `const Application& %gApplication();`).
 - Constant global variables should be written in all caps with underscores separating the words (e.g. `int NUM_ELEMENTS = 5;`).
 - Macros should be written in all caps with underscores separating the words, and prefixed with "BS_" (e.g. \#`define BS_EXCEPT`).
 - Static non-constant variables should be prefixed with an "s", followed by a capital first letter (e.g. `bool sIsInitialized;`).
 - Static constant variables should be written in all caps with underscores separating the words (e.g. `int NUM_ELEMENTS = 5;`).
 - Everything needs to be part of the "bs" namespace
  - Types used primarily on the core thread should be part of the "bs::ct" namespace
 - Use "Base" suffix to mark base classes unless a more generalized name is more appropriate
 - Prefix templated base classes/structs with a "T" unless a more generalized name is more appropriate
 - Use "C" prefix for component implementations
 - Use "H" prefix for component and resource handles
 
**Class interface**
 - Carefully construct class interfaces, keeping only relevant methods public and everything else private/protected/internal
 - Public fields should be avoided (except for simple structures) and getter/setter methods should be provided instead
 - If a class needs to communicate with another class within a sub-system, but the functionality isn't something a normal user is meant to use, provide "internal" methods. Those may be (depending on what is more convenient):
  - Private methods that are exposed to the other class using the `friend` syntax
  - Public methods that are prefixed with "_" (e.g. `void _internalMethod();`).
 - Class entries should be listed in this order:
  - typedefs
  - nested classes/structures
  - public methods
  - internal methods
  - public fields
  - internal fields
  - protected methods
  - private methods
  - protected fields
  - private fields
 - Each major class should belong to its own .h file, prefixed with "Bs" followed by the class name
  - If helper types are required, they can belong to the same file, but prefer to list them after the main class when possible
 
**Function/method interface**
 - If method doesn't modify data, always mark it as const (getters especially) (e.g. `int getValue() const;`)
 - Always pass non-primitive parameters by reference unless `null` is a valid value in which case use a pointer (e.g. `void myMethod(int a, const SomeStruct& b)`)
 - Reference/pointer parameters that won't be modified by a function should always be `const` (e.g. `bool processData(const SomeStruct& input, SomeStruct& output)`)
 
**Implementation**
 - Avoid the use of `auto` except for:
  - Very long type names (like iterators), in which case include indication of the type in variable name (e.g. `auto iter = ...;`)
  - Variables where type can be otherwise deduced (e.g. casts, `auto var = (UINT32)otherVar;`)
 - Use built-in typedefs for standard library containers (e.g. `Vector`) and shared pointers (`SPtr`).
 - Don't allocate memory using `new/delete` or `malloc/free`, instead use bs::f allocators
 - No code warnings under default compiler warning settings are allowed. Fix all your warnings or if absolutely not possible isolate that bit of code and disable that specific warning (but only in that bit of code).
 - Never use `using namespace` in a header
 
**Documentation**
 - Documentation must be in JavaDoc format
 - Write **meaningful** documentation
  - Avoid writing documentation that provides information that can be deduced from class/method name and instead try to provide the reader with deeper understanding. For example `/** Gets value */ getValue(); ` is pointless documentation
  - Avoid giving away implementation details in documentation. Implementation should be allowed to change, and as long as the interface remains the same, the documentation should remain the same
 - Document every class/structure/enum with a short description on what their purpose is
  - Optionally provide a use-case example
  - Document every enum member
  - Document struct fields unless they have getter/setter methods that are already documented
  - Getter/Setter methods should have the same documentation (e.g. document the setter, then use \@copydoc on getter)
  - Documentation for private class fields is optional
 - Document every method, both public and private
  - The only exceptions being very trivial methods whose use can be deduced from their name
  - You are allowed to omit documentation for method parameters & return value if it is clear from the general method description what they do
  - If you document one parameter you must document them all, as well as the return value
 - Document implementations
  - Prefer writing self-documenting code as opposed to writing comments
   - Give variables meaningful names
   - If an expression is too complex break it down using temporary variables using meaningful names
   - If it is still unclear, write comments
  - Carefully document complex mathematical expressions
   - Add a comment what the expression does or a hyperlink to expression source
   - If simplified or re-arranged, provide reference expression in comments, as well as explanation on how the current expression was derived (or a link to one)
 - Use \@`addtogroup` to group classes into relevant sections (e.g. a class doing something physics-related should be in the "Physics" group)
  - Classes/structs/functions/enums that aren't meant to be used by normal users should be placed in a separate group to avoid polluting the user-facing documentation (e.g. user-facing physics functionality should be in the "Physics" group, while non-user-facing should be in "Physics_Internal" group)
 - Use \@`name Internal` to mark public methods inside a class that they are not meant for normal use (i.e. they might be used for internal communication within a sub-system only)
 - Documentation should produce no Doxygen warnings when it is generated
 - If it is not obvious for which thread is a class/method meant to be used on, note it in the documentation
 