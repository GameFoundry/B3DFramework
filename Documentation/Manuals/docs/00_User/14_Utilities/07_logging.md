Logging	messages								{#logging}
===============

Logging can be a useful way to debug issues during development, or notify the user that an error occurred. In bs::f it is handled though the @ref bs::Debug "Debug" class. Use @ref bs::gDebug "gDebug()" for an easy way to access the **Debug** instance.

Use any of these methods to log a new message:
 - @ref bs::Debug::logDebug "Debug::logDebug" - Logs an informative message.
 - @ref bs::Debug::logWarning "Debug::logWarning" - Log a message that an issue happened, but it's non critical.
 - @ref bs::Debug::logError "Debug::logError" - Log a message that an issue happened, and it's critical.
 
~~~~~~~~~~~~~{.cpp}
UINT32 x = 5;

gDebug().logDebug("Value of x is: " + toString(x));

if(x != 5)
	gDebug().logError("X must equal 5!");
~~~~~~~~~~~~~

All logged messages will be output to the standard console output, as well as the attached debugger output (if any).

Messages are also saved internally, and can be output to a .html file by calling @ref bs::Debug::saveLog "Debug::saveLog()".

~~~~~~~~~~~~~{.cpp}
gDebug().saveLog("C:\myLog.html");
~~~~~~~~~~~~~