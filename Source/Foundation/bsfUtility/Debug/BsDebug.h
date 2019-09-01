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
	
	/** Type of the log that will be saved. */
	enum class SavedLogType
	{
		HTML = 0,
		Textual = 1
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

		/**
		 * Logs a new message.
		 *
		 * @param[in]	message		The message describing the log entry.
		 * @param[in]	verbosity	Verbosity of the message, determining its importance.
		 * @param[in]	category	Category of the message, determining which system is it relevant to.
		 */
		void log(const String& message, LogVerbosity verbosity, UINT32 category = 0);

		/** Retrieves the Log used by the Debug instance. */
		Log& getLog() { return mLog; }

		/** Converts raw pixels into a BMP image and saves it as a file */
		void writeAsBMP(UINT8* rawPixels, UINT32 bytesPerPixel, UINT32 width, UINT32 height, const Path& filePath,
			bool overwrite = true) const;

		/**
		 * Saves a log about the current state of the application to the specified location.
		 *
		 * @param	path	Absolute path to the log filename.
		 * @param   type    Format of the saved log.
		 */
		void saveLog(const Path& path, SavedLogType type = SavedLogType::HTML) const;
		
		/**
		 * Saves a log about the current state of the application to the specified location as a HTML file.
		 *
		 * @param	path	Absolute path to the log filename.
		 */
		void saveHtmlLog(const Path& path) const;
		
		/**
		 * Saves a log about the current state of the application to the specified location as a text file.
		 *
		 * @param	path	Absolute path to the log filename.
		 */
		void saveTextLog(const Path& path) const;
		
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

		/** This allows setting a log callback that can override the default action in log */
		void setLogCallback(
			std::function<bool(const String& message, LogVerbosity verbosity, UINT32 category)> callback)
		{
			mCustomLogCallback = callback;
		}

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
		std::function<bool(const String& message, LogVerbosity verbosity, UINT32 category)> mCustomLogCallback;
	};

	/** A simpler way of accessing the Debug module. */
	BS_UTILITY_EXPORT Debug& gDebug();

#ifndef BS_LOG_VERBOSITY
	#if BS_DEBUG_MODE
		#define BS_LOG_VERBOSITY LogVerbosity::Log
	#else
		#define BS_LOG_VERBOSITY LogVerbosity::Warning
	#endif
#endif

/**
 * Defines a new log category to use with BS_LOG. Each category must have a unique ID. A matching call to
 * BS_LOG_CATEGORY_IMPL must be done in the source file.
 */
#define BS_LOG_CATEGORY(name, id) struct LogCategory##name { enum { _id = id }; static bool sRegistered; };

/** Registers the name of the category. Should be placed in the implementation file for each corresponding BS_LOG_CATEGORY call. */
#define BS_LOG_CATEGORY_IMPL(name) bool LogCategory##name::sRegistered = Log::_registerCategory(LogCategory##name::_id, #name);

/** Get the ID of the log category based on its name. */
#define BS_LOG_GET_CATEGORY_ID(category) LogCategory##category::_id

#define BS_LOG(verbosity, category, message, ...)													\
  do																								\
  {																									\
	using namespace ::bs;																			\
	if ((INT32)LogVerbosity::verbosity <= (INT32)BS_LOG_VERBOSITY)									\
	{																								\
	  gDebug().log(StringUtil::format(message, ##__VA_ARGS__) + String("\n\t\t in ") +				\
					   __PRETTY_FUNCTION__ + " [" + __FILE__ + ":" + toString(__LINE__) + "]\n",	\
				   LogVerbosity::verbosity, LogCategory##category::_id);							\
	}																								\
  } while (0)

  BS_LOG_CATEGORY(Uncategorized, 0)
  BS_LOG_CATEGORY(FileSystem, 1)
  BS_LOG_CATEGORY(RTTI, 2)
  BS_LOG_CATEGORY(Generic, 3)
  BS_LOG_CATEGORY(Platform, 4)
  BS_LOG_CATEGORY(Serialization, 5)

  /** @} */
}
