---
title: Crash handling
---

Whenever possible you should try to avoid triggering a crash, and instead try to recover from error conditions as best as possible. When crashing cannot be avoided you can use @b3d::CrashHandler to report fatal errors. Call @b3d::CrashHandler::ReportCrash to manually trigger such an error. An error will be logged and a message box with relevant information displayed.

~~~~~~~~~~~~~{.cpp}
CrashHandler::Instance().ReportCrash("My error", "My error description");
~~~~~~~~~~~~~

You can also use the @B3D_LOG macro with the **Fatal** level, which internally calls **CrashHandler::ReportCrash()** and automatically adds file/line information and terminates the process after reporting the crash.

~~~~~~~~~~~~~{.cpp}
B3D_LOG(Fatal, LogUncategorized, "My error description");
~~~~~~~~~~~~~

**CrashHandler** also provides @b3d::CrashHandler::GetStackTrace method that allows you to retrieve a stack trace to the current method, which can be useful for logging, or custom crash handling.
