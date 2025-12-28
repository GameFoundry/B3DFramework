//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DLog.h"

namespace b3d
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Debug), Singleton(GetDebug)) Debug
	{
	public:
		Debug() = default;

		/**
		 * Logs a new message.
		 *
		 * @param	message			The message describing the log entry.
		 * @param	verbosity		Verbosity of the message, determining its importance.
		 * @param	categoryName	Category of the message, determining which system is it relevant to.
		 */
		B3D_SCRIPT_EXPORT()
		void Log(const String& message, LogVerbosity verbosity, const String& categoryName);

		/**
		 * Removes all log entries for a specific category and/or verbosity level.
		 *
		 * @param	categoryName	Name of the category to clear. Specify null to clear all categories.
		 * @param	verbosity		Verbosity level to clear.
		 */
		B3D_SCRIPT_EXPORT()
		void ClearLog(const String& categoryName, LogVerbosity verbosity = LogVerbosity::Any) { return mLog.Clear(categoryName, verbosity); }

		/** Returns all existing log entries. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(LogEntries))
		Vector<LogEntry> GetLogEntries() { return mLog.GetEntries(); }

		/** Retrieves the Log used by the Debug instance. */
		class Log& GetLog() { return mLog; }

		/** Converts raw pixels into a BMP image and saves it as a file */
		void WriteAsBmp(u8* rawPixels, u32 bytesPerPixel, u32 width, u32 height, const Path& filePath, bool overwrite = true) const;

		/**
		 * Saves a log about the current state of the application to the specified location.
		 *
		 * @param	path	Absolute path to the log filename.
		 * @param   type    Format of the saved log.
		 */
		void SaveLog(const Path& path, SavedLogType type = SavedLogType::HTML) const;

		/**
		 * Saves a log about the current state of the application to the specified location as a HTML file.
		 *
		 * @param	path	Absolute path to the log filename.
		 */
		void SaveHtmlLog(const Path& path) const;

		/**
		 * Saves a log about the current state of the application to the specified location as a text file.
		 *
		 * @param	path	Absolute path to the log filename.
		 */
		void SaveTextLog(const Path& path) const;

		/**
		 * Triggered when a new entry in the log is added.
		 *
		 * @note	Main thread only.
		 */
		B3D_SCRIPT_EXPORT()
		Event<void(const LogEntry&)> OnLogEntryAdded;

		/**
		 * Triggered whenever one or multiple log entries were added or removed. Triggers only once per frame.
		 *
		 * @note	Main thread only.
		 */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnLogModified;

		/** This allows setting a log callback that can override the default action in log */
		void SetLogCallback(
			std::function<bool(const String& message, LogVerbosity verbosity, const char* categoryName)> callback)
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
		 * @note	Main thread only.
		 */
		void TriggerCallbacksInternal();

		/** @} */
	private:
		u64 mLogHash = 0;
		class Log mLog;
		std::function<bool(const String& message, LogVerbosity verbosity, const char* categoryName)> mCustomLogCallback;
	};

	/** Returns the Debug class singleton. */
	B3D_EXPORT Debug& GetDebug();

#ifndef B3D_LOG_VERBOSITY
#	if B3D_DEBUG
#		define B3D_LOG_VERBOSITY LogVerbosity::Log
#	else
#		define B3D_LOG_VERBOSITY LogVerbosity::Warning
#	endif
#endif

/** Defines a new log category to use with B3D_LOG. A matching call to B3D_LOG_CATEGORY must be done in the implementation file. */
#define B3D_LOG_CATEGORY_EXTERN(Name, DefaultRunTimeVerbosity)											\
	extern struct LogCategory##Name : public b3d::LogCategory<b3d::LogVerbosity::DefaultRunTimeVerbosity> \
	{																									\
		B3D_FORCEINLINE LogCategory##Name()																\
			: LogCategory(#Name) {}																		\
	} LogCategory##Name##Instance;

/** Defines a new log category to use with B3D_LOG. A matching call to B3D_LOG_CATEGORY_EXTERN must be done in the header file. */
#define B3D_LOG_CATEGORY(Name) LogCategory##Name LogCategory##Name##Instance;

/** Defines a new log category to use with B3D_LOG. Can only be used in implementation files. Category is only valid for a single implementation file. */
#define B3D_LOG_CATEGORY_STATIC(Name, DefaultRunTimeVerbosity)												\
	static  struct LogCategory##Name : public b3d::LogCategory<b3d::LogVerbosity::DefaultRunTimeVerbosity>	\
	{																										\
		B3D_FORCEINLINE LogCategory##Name()																	\
			: LogCategory(#Name) {}																			\
	} LogCategory##Name##Instance;

#define B3D_LOG(Verbosity, Category, Message, ...)                                                                                                                                                                           \
	do                                                                                                                                                                                                                       \
	{                                                                                                                                                                                                                        \
		using namespace ::b3d;                                                                                                                                                                                                \
		if((i32)LogVerbosity::Verbosity <= (i32)LogCategory##Category::kCompileTimeVerbosity && !LogCategory##Category##Instance.IsVerbositySupressed(LogVerbosity::Verbosity))                                              \
		{                                                                                                                                                                                                                    \
			GetDebug().Log(StringUtil::Format(Message "\n\t\t in ", ##__VA_ARGS__) + __PRETTY_FUNCTION__ + " [" + __FILE__ + ":" + ToString(__LINE__) + "]\n", LogVerbosity::Verbosity, LogCategory##Category##Instance.GetName());	 \
		}                                                                                                                                                                                                                    \
	}                                                                                                                                                                                                                        \
	while(0)

	// Same as B3D_LOG, except Message is expected to be String rather than const char*.
#define B3D_LOG_STRING(Verbosity, Category, Message, ...)                                                                                                                                                                           \
	do                                                                                                                                                                                                                       \
	{                                                                                                                                                                                                                        \
		using namespace ::b3d;                                                                                                                                                                                                \
		if((i32)LogVerbosity::Verbosity <= (i32)LogCategory##Category::kCompileTimeVerbosity && !LogCategory##Category##Instance.IsVerbositySupressed(LogVerbosity::Verbosity))                                              \
		{                                                                                                                                                                                                                    \
			GetDebug().Log(StringUtil::Format(Message + "\n\t\t in ", ##__VA_ARGS__) + __PRETTY_FUNCTION__ + " [" + __FILE__ + ":" + ToString(__LINE__) + "]\n", LogVerbosity::Verbosity, LogCategory##Category##Instance.GetName());	 \
		}                                                                                                                                                                                                                    \
	}                                                                                                                                                                                                                        \
	while(0)

	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogUncategorized, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogFileSystem, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogRTTI, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogGeneric, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogPlatform, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogSerialization, Log)
	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogUnitTest, Log)

	// Ensure that our ensure macro implementation is correctly not inlined on MSVC
	template <typename ReturnType = void, class Function>
	ReturnType B3D_FORCENOINLINE B3D_CODE_SECTION(".debug.") ExecuteEnsureCallback(Function&& function)
	{
		return function();
	}

	#define B3D_ENSURE_IMPLEMENTATION(captureScope, alwaysCheck, expression, ...) \
	(B3D_LIKELY(!!(expression)) || (ExecuteEnsureCallback<bool>([captureScope]() B3D_FORCENOINLINE B3D_CODE_SECTION(".debug.") { \
			static bool sHasFailureBeenReported = false; \
			if (!sHasFailureBeenReported || alwaysCheck) \
			{ \
				sHasFailureBeenReported = true; \
				B3D_LOG(Error, LogUncategorized, ##__VA_ARGS__); \
				return true; \
			} \
			return false; }) && ([]() { B3D_BREAK(); }(), false)))

	#define B3D_ENSURE(InExpression) B3D_ENSURE_IMPLEMENTATION(, true, InExpression,"")
	#define B3D_ENSURE_LOG(InExpression, InFormat, ...) B3D_ENSURE_IMPLEMENTATION(&, true, InExpression, InFormat, ##__VA_ARGS__)
	#define B3D_ENSURE_ONCE(InExpression) B3D_ENSURE_IMPLEMENTATION(, false, InExpression, "")
	#define B3D_ENSURE_ONCE_LOG(InExpression, InFormat, ...) B3D_ENSURE_IMPLEMENTATION(&, false, InExpression, InFormat, ##__VA_ARGS__)


	/** @} */
} // namespace b3d
