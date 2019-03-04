---
title: Logging	messages
---
Logging can be a useful way to debug issues during development, or notify the user that an error occurred. In bs::f it is handled though the @bs::Debug class. Use @bs::gDebug for an easy way to access the **Debug** instance.

Use any of these methods to log a new message:
 - @bs::Debug::logDebug - Logs an informative message.
 - @bs::Debug::logWarning - Log a message that an issue happened, but it's non critical.
 - @bs::Debug::logError - Log a message that an issue happened, and it's critical.
 
~~~~~~~~~~~~~{.cpp}
UINT32 x = 5;

gDebug().logDebug("Value of x is: " + toString(x));

if(x != 5)
	gDebug().logError("X must equal 5!");
~~~~~~~~~~~~~

All logged messages will be output to the standard console output, as well as the attached debugger output (if any).

Messages are also saved internally, and can be output to a .html file by calling @bs::Debug::saveLog.

~~~~~~~~~~~~~{.cpp}
gDebug().saveLog("C:\myLog.html");
~~~~~~~~~~~~~