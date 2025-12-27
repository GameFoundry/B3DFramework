//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Debug
	 *  @{
	 */

	/** Represents verbosity level at which a specific log message will be displayed. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Debug)) LogVerbosity
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Debug)) LogEntry
	{
	public:
		LogEntry() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		LogEntry(String message, LogVerbosity verbosity, String categoryName)
			: Message(std::move(message)), Verbosity(verbosity), CategoryName(std::move(categoryName)), LocalTime(std::time(nullptr))
		{}

		String Message;
		LogVerbosity Verbosity = LogVerbosity::Info;
		String CategoryName;
		u64 LocalTime = 0; /**< Local time of message */
	};

	/** Base class for all log categories. */
	struct B3D_EXPORT LogCategoryBase
	{
	public:
		LogCategoryBase(const char* name, LogVerbosity defaultMaximumVerbosity);
		~LogCategoryBase();

		/** Returns the name of the category. */
		const char* GetName() const { return mName; }

		/** Determines the maximum verbosity reported by this category. Verbosity levels higher than this will not be reported. */
		LogVerbosity GetMaximumVerbosity() const { return mMaximumVerbosity; }

		/** Returns true is the provided verbosity suppressed for this category. */
		bool IsVerbositySupressed(LogVerbosity verbosity)
		{
			return (u32)verbosity > (u32)mMaximumVerbosity;
		}

	private:
		friend class Log;

		/** @copydoc GetMaximumVerbosity */
		void SetMaximumVerbosity(LogVerbosity maximumVerbosity) { mMaximumVerbosity = maximumVerbosity; }

		const char* mName;
		LogVerbosity mDefaultMaximumVerbosity;
		LogVerbosity mMaximumVerbosity;
	};

	/** Templated base class for a category with some particular default verbosity. */
	template <LogVerbosity DefaultRunTimeVerbosity, LogVerbosity CompileTimeVerbosity = LogVerbosity::Any>
	struct LogCategory : public LogCategoryBase
	{
		static constexpr LogVerbosity kCompileTimeVerbosity = CompileTimeVerbosity;

		B3D_FORCEINLINE LogCategory(const char* name)
			: LogCategoryBase(name, DefaultRunTimeVerbosity)
		{ }
	};

	/**
	 * Used for logging messages. Messages can be categorized and filtered by verbosity, the log can be saved to a file
	 * and send out callbacks when a new message is added.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT Log
	{
	public:
		Log() = default;
		~Log();

		/**
		 * Logs a new message.
		 *
		 * @param	message			The message describing the log entry.
		 * @param	verbosity		Verbosity of the message, determining its importance.
		 * @param	categoryName	Category of the message, determining which system is it relevant to.
		 */
		void LogMessage(const String& message, LogVerbosity verbosity, const String& categoryName);

		/** Removes all log entries. */
		void Clear();

		/**
		 * Removes all log entries for a specific category and/or verbosity level.
		 *
		 * @param	categoryName	Name of the category to clear. Specify empty string to clear all categories.
		 * @param	verbosity		Verbosity level to clear.
		 */
		void Clear(const String& categoryName, LogVerbosity verbosity = LogVerbosity::Any);

		/** Returns all existing log entries. */
		Vector<LogEntry> GetEntries() const;

		/**
		 * Returns the latest unread entry from the log queue, and removes the entry from the unread entries list.
		 *
		 * @param	outEntry	Entry that was retrieved, or undefined if no entries exist.
		 * @return				True if an unread entry was retrieved, false otherwise.
		 */
		bool GetUnreadEntry(LogEntry& outEntry);

		/**
		 * Returns the last available log entry.
		 *
		 * @param	outEntry	Entry that was retrieved, or undefined if no entries exist.
		 * @return				True if an entry was retrieved, false otherwise.
		 */
		bool GetLastEntry(LogEntry& outEntry);

		/**
		 * Returns a hash value that is modified whenever entries in the log change. This can be used for
		 * checking for changes by external systems.
		 */
		u64 GetHash() const { return mHash; }

		/** Changes the maximum verbosity for a particular log category. Verbosity levels higher than specified verbosity will not be logged. */
		static void SetCategoryMaximumVerbosity(const char* name, LogVerbosity maximumVerbosity);

		/**
		 * @name Internal
		 * @{
		 */

		/** Registers the new category object. */
		static void RegisterCategory(LogCategoryBase& category);

		/** Unregisters an existing category object. */
		static void UnregisterCategory(LogCategoryBase& category);

		/** @} */

	private:
		friend class Debug;

		/** Returns a modifyable map containing all the log categories. */
		static UnorderedMultimap<const char*, LogCategoryBase*>& GetCategoriesMap();

		/** Returns all log entries, including those marked as unread. */
		Vector<LogEntry> GetAllEntries() const;

		Vector<LogEntry> mEntries;
		Queue<LogEntry> mUnreadEntries;
		u64 mHash = 0;

		mutable RecursiveMutex mMutex;
	};

	/** @} */
} // namespace b3d
