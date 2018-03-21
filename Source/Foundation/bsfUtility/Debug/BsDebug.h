//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Debug/BsLog.h"

namespace bs
{
	class Log;

	/** @addtogroup Debug
	 *  @{
	 */

	/** Available types of channels that debug messages can be logged to. */
	enum class DebugChannel
	{
		Debug, Warning, Error, CompilerWarning, CompilerError
	};

	/**
	 * Utility class providing various debug functionality.
	 *
	 * @note	Thread safe.
	 */
	class BS_UTILITY_EXPORT Debug
	{
	public:
		Debug() = default;

		/** Adds a log entry in the "Debug" channel. */
		void logDebug(const String& msg);

		/** Adds a log entry in the "Warning" channel. */
		void logWarning(const String& msg);

		/** Adds a log entry in the "Error" channel. */
		void logError(const String& msg);

		/** Adds a log entry in the specified channel. You may specify custom channels as needed. */
		void log(const String& msg, UINT32 channel);

		/** Retrieves the Log used by the Debug instance. */
		Log& getLog() { return mLog; }

		/** Converts raw pixels into a BMP image and saves it as a file */
		void writeAsBMP(UINT8* rawPixels, UINT32 bytesPerPixel, UINT32 width, UINT32 height, const Path& filePath, 
			bool overwrite = true) const;

		/**
		 * Saves a log about the current state of the application to the specified location.
		 * 
		 * @param	path	Absolute path to the log filename.
		 */
		void saveLog(const Path& path) const;

		/**
		 * Triggered when a new entry in the log is added.
		 * 			
		 * @note	Sim thread only.
		 */
		Event<void(const LogEntry&)> onLogEntryAdded;

		/**
		 * Triggered whenever one or multiple log entries were added or removed. Triggers only once per frame.
		 * 			
		 * @note	Sim thread only.
		 */
		Event<void()> onLogModified;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Triggers callbacks that notify external code that a log entry was added.
		 * 			
		 * @note	Sim thread only.
		 */
		void _triggerCallbacks();

		/** @} */
	private:
		UINT64 mLogHash = 0;
		Log mLog;
	};

	/** A simpler way of accessing the Debug module. */
	BS_UTILITY_EXPORT Debug& gDebug();

/** Shortcut for logging a message in the debug channel. */
#define LOGDBG(x) bs::gDebug().logDebug((x) + String("\n\t\t in ") + __PRETTY_FUNCTION__ + " [" + __FILE__ + ":" + toString(__LINE__) + "]\n");

/** Shortcut for logging a message in the warning channel. */
#define LOGWRN(x) bs::gDebug().logWarning((x) + String("\n\t\t in ") + __PRETTY_FUNCTION__ + " [" + __FILE__ + ":" + toString(__LINE__) + "]\n");

/** Shortcut for logging a message in the error channel. */
#define LOGERR(x) bs::gDebug().logError((x) + String("\n\t\t in ") + __PRETTY_FUNCTION__ + " [" + __FILE__ + ":" + toString(__LINE__) + "]\n");

/** Shortcut for logging a verbose message in the debug channel. Verbose messages can be ignored unlike other log messages. */
#define LOGDBG_VERBOSE(x) ((void)0)

/** Shortcut for logging a verbose message in the warning channel. Verbose messages can be ignored unlike other log messages. */
#define LOGWRN_VERBOSE(x) ((void)0)

	/** @} */
}
