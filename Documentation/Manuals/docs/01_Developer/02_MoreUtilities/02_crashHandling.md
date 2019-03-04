Crash handling							{#crashHandling}
===============
[TOC]

Whenever possible you should try to avoid triggering a crash, and instead try to recover from error conditions as best as possible. When crashing cannot be avoided you can use @ref bs::CrashHandler "CrashHandler" to report fatal errors. Call @ref bs::CrashHandler::reportCrash "CrashHandler::reportCrash()" to manually trigger such an error. An error will be logged and a message box with relevant information displayed.

~~~~~~~~~~~~~{.cpp}
gCrashHandler().reportCrash("My error", "My error description");
~~~~~~~~~~~~~

You can also use @ref BS_EXCEPT macro, which internally calls **CrashHandler::reportCrash()** but automatically adds file/line information and terminates the process after reporting the crash.

~~~~~~~~~~~~~{.cpp}
BS_EXCEPT(InternalErrorException, "My error description");
~~~~~~~~~~~~~

**CrashHandler** also provides @ref bs::CrashHandler::getStackTrace "CrashHandler::getStackTrace()" method that allows you to retrieve a stack trace to the current method, which can be useful for logging, or custom crash handling.