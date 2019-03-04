---
title: Crash handling
---

Whenever possible you should try to avoid triggering a crash, and instead try to recover from error conditions as best as possible. When crashing cannot be avoided you can use @bs::CrashHandler to report fatal errors. Call @bs::CrashHandler::reportCrash to manually trigger such an error. An error will be logged and a message box with relevant information displayed.

~~~~~~~~~~~~~{.cpp}
gCrashHandler().reportCrash("My error", "My error description");
~~~~~~~~~~~~~

You can also use @ref BS_EXCEPT macro, which internally calls **CrashHandler::reportCrash()** but automatically adds file/line information and terminates the process after reporting the crash.

~~~~~~~~~~~~~{.cpp}
BS_EXCEPT(InternalErrorException, "My error description");
~~~~~~~~~~~~~

**CrashHandler** also provides @bs::CrashHandler::getStackTrace method that allows you to retrieve a stack trace to the current method, which can be useful for logging, or custom crash handling.