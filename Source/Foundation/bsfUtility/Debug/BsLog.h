//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsTime.h"

namespace bs
{
	/** @addtogroup Debug
	 *  @{
	 */

	/** Represents verbosity level at which a specific log message will be displayed. */
	enum class BS_SCRIPT_EXPORT(m:Debug) LogVerbosity
	{
		/** Fatal error happened that application cannot recover from and will crash. */
		Fatal,
		/** An error happened that will not result in an immediate crash but may cause serious problems. */
		Error,
		/** Something went wrong but the application will not crash, although invalid behaviour might be observed. */
		Warning,
		/** An informational message will be logged, can be used for debugging and tracing. */
		Info,
		/** Same as Info, but the message will only be logged to the log file and not any console output. */
		Log,
		/** 
		 * Messages that can provide additional information and warnings, but are too spammy to be displayed under normal 
		 * circumstances. 
		 */
		Verbose,
		/** Same as Verbose, but for even spammier messages. */
		VeryVerbose,
		/** Meta-type encompassing all verbosity types. Should not be used for logging directly. */
		Any
	};

	/** A single log entry, containing a message and a channel the message was recorded on. */
	class BS_UTILITY_EXPORT LogEntry
	{
	public:
		LogEntry() = default;
		LogEntry(String msg, LogVerbosity verbosity, UINT32 category)
			:mMsg(std::move(msg)), mVerbosity(verbosity), mCategory(category), mLocalTime(std::time(nullptr))
		{ }

		/** Determines how important is the message and when should it be displayed. */
		LogVerbosity getVerbosity() const { return mVerbosity; }

		/** Category of the system the message originated from. */
		UINT32 getCategory() const { return mCategory; }

		/** Text of the message. */
		const String& getMessage() const { return mMsg; }

		/** Local time of message being registered as a text */
		const std::time_t& getLocalTime() const { return mLocalTime; }

	private:
		String mMsg;
		LogVerbosity mVerbosity;
		UINT32 mCategory;
		std::time_t mLocalTime;
	};

	/**
	 * Used for logging messages. Messages can be categorized and filtered by verbosity, the log can be saved to a file
	 * and send out callbacks when a new message is added.
	 * 			
	 * @note	Thread safe.
	 */
	class BS_UTILITY_EXPORT Log
	{
	public:
		Log() = default;
		~Log();

		/**
		 * Logs a new message. 
		 *
		 * @param[in]	message		The message describing the log entry.
		 * @param[in]	verbosity	Verbosity of the message, determining its importance.
		 * @param[in]	category	Category of the message, determining which system is it relevant to.
		 */
		void logMsg(const String& message, LogVerbosity verbosity, UINT32 category);

		/** Removes all log entries. */
		void clear();

		/** 
		 * Removes all log entries for a specific verbosity level and/or category. Specify -1 to clear all verbosity levels 
		 * and/or categories.
		 */
		void clear(LogVerbosity verbosity, UINT32 category);

		/** Returns all existing log entries. */
		Vector<LogEntry> getEntries() const;

		/**
		 * Returns the latest unread entry from the log queue, and removes the entry from the unread entries list.
		 * 			
		 * @param[out]	entry	Entry that was retrieved, or undefined if no entries exist.
		 * @return				True if an unread entry was retrieved, false otherwise.
		 */
		bool getUnreadEntry(LogEntry& entry);

		/**
		 * Returns the last available log entry.
		 *
		 * @param[out]	entry	Entry that was retrieved, or undefined if no entries exist.
		 * @return				True if an entry was retrieved, false otherwise.
		 */
		bool getLastEntry(LogEntry& entry);

		/**
		 * Returns a hash value that is modified whenever entries in the log change. This can be used for
		 * checking for changes by external systems.
		 */
		UINT64 getHash() const { return mHash; }

	private:
		friend class Debug;

		/** Returns all log entries, including those marked as unread. */
		Vector<LogEntry> getAllEntries() const;

		Vector<LogEntry> mEntries;
		Queue<LogEntry> mUnreadEntries;
		UINT64 mHash = 0;
		mutable RecursiveMutex mMutex;
	};

	/** @} */
}
