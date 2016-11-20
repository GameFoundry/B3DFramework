//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Debug
	 *  @{
	 */

	/** A single log entry, containing a message and a channel the message was recorded on. */
	class BS_UTILITY_EXPORT LogEntry
	{
	public:
		LogEntry() { }
		LogEntry(const String& msg, UINT32 channel);

		/** Channel the message was recorded on. */
		UINT32 getChannel() const { return mChannel; }

		/** Text of the message. */
		const String& getMessage() const { return mMsg; }

	private:
		String mMsg;
		UINT32 mChannel;
	};

	/**
	 * Used for logging messages. Can categorize messages according to channels, save the log to a file
	 * and send out callbacks when a new message is added.
	 * 			
	 * @note	Thread safe.
	 */
	class BS_UTILITY_EXPORT Log
	{
	public:
		Log();
		~Log();

		/**
		 * Logs a new message. 
		 *
		 * @param[in]	message	The message describing the log entry.
		 * @param[in]	channel Channel in which to store the log entry.
		 */
		void logMsg(const String& message, UINT32 channel);

		/** Removes all log entries. */
		void clear();

		/** Removes all log entries in a specific channel. */
		void clear(UINT32 channel);

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
		UINT64 mHash;
		mutable RecursiveMutex mMutex;
	};

	/** @} */
}