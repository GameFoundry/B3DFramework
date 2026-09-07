---
title: Logging messages
---
Logging can be a useful way to debug issues during development, or notify the user that an error occurred. In the framework it is handled though the @b3d::Debug class, which can recognize what severity the message is and from what component or module it came. Use @b3d::GetDebug for an easy way to access the **Debug** instance.

To log a new message, you can use two of the following methods:
 - Use the @B3D_LOG macro (recommended).
 - Use the @b3d::Debug::Log function directly.

~~~~~~~~~~~~~{.cpp}
u32 variableX = 5;

B3D_LOG(Info, LogGeneric, "Value of x is: {0}", variableX);

if(variableX != 5)
	B3D_LOG(Error, LogGeneric, "X must equal 5!");
~~~~~~~~~~~~~

All logged messages will be output to the standard console output, as well as the attached debugger output (if any).

## Recoverable checks

**B3D_ENSURE(expression)** evaluates the expression once and returns its boolean result. A failed check logs an error and continues execution. If a debugger is attached, it pauses at a resumable breakpoint before returning false. **B3D_ENSURE_LOG(expression, format, ...)** also includes a formatted error message.

**B3D_ENSURE_ONCE** and **B3D_ENSURE_ONCE_LOG** report and break only on the first failure at each call site. They still evaluate the expression and return its result on subsequent calls. Use **PlatformUtility::IsDebuggerAttached()** to query whether the process currently has a debugger attached.

~~~~~~~~~~~~~{.cpp}
if(!B3D_ENSURE_LOG(resource != nullptr, "Required resource is unavailable"))
	return;
~~~~~~~~~~~~~

## Saving and categorizing logs

Messages are also saved internally, and can be output to a either .html file or to a text file by calling @b3d::Debug::SaveLog.

~~~~~~~~~~~~~{.cpp}
GetDebug().SaveLog("C:/myLog.html", SavedLogType::HTML);
GetDebug().SaveLog("C:/myLog.txt", SavedLogType::Textual);
~~~~~~~~~~~~~

Sometimes you want to register your own log categories than the what the framework provides. For that you can use @B3D_LOG_CATEGORY_EXTERN, @B3D_LOG_CATEGORY, or @B3D_LOG_CATEGORY_STATIC macros to create custom log categories.

For categories shared across multiple files, use the **B3D_LOG_CATEGORY_EXTERN** and **B3D_LOG_CATEGORY** pair:

~~~~~~~~~~~~~{.cpp}
// In header file
B3D_LOG_CATEGORY_EXTERN(UserApp, Log)
B3D_LOG_CATEGORY_EXTERN(UserModule, Log)

// In implementation file
B3D_LOG_CATEGORY(UserApp)
B3D_LOG_CATEGORY(UserModule)
~~~~~~~~~~~~~

For categories used only within a single implementation file, use **B3D_LOG_CATEGORY_STATIC**:

~~~~~~~~~~~~~{.cpp}
// In .cpp file only - no header declaration needed
B3D_LOG_CATEGORY_STATIC(LocalCache, Log)
B3D_LOG_CATEGORY_STATIC(HelperFunctions, Warning)

B3D_LOG(Info, LocalCache, "Cache initialized with {0} entries", entryCount);
B3D_LOG(Warning, HelperFunctions, "Helper function fallback used");
~~~~~~~~~~~~~

The **B3D_LOG_CATEGORY_STATIC** macro is more convenient for private implementation details as it doesn't require a separate header declaration and the category remains local to the translation unit.

Once registered, you can use these custom categories with the B3D_LOG macro:

~~~~~~~~~~~~~{.cpp}
B3D_LOG(Info, UserApp, "Application started successfully");
B3D_LOG(Warning, UserModule, "Module initialization delayed");
~~~~~~~~~~~~~
