//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2024 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	class IProjectLibraryEventNotifier;
	class ProjectLibraryDatabase;

	/** @addtogroup Library
	 *  @{
	 */

	/** Possible states that a long operation may be in. */
	enum class LongOperationState
	{
		NotStarted, /**< Operation has been created, but is not yet running. */
		Running, /**< Operation is currently queued for execution or being executed. */
		Failed, /**< Operation has finished running because it encountered an error. */
		Aborted, /**< Operation has finished running because it was aborted by the user. */
		Finished /**< Operation has successfully finished running. */
	};

	/** Flags used to control the behaviour of LongAsynchronousOperation. */
	enum class LongOperationFlag
	{
		None = 0,
		ReportsProgress = 1 << 0, /**< Operation does not report progress percentages. */
		CanBeAborted = 1 << 1 /**< User can be abort the operation. */
	};

	using LongOperationFlags = Flags<LongOperationFlag>;
	B3D_FLAGS_OPERATORS(LongOperationFlag)

	/**
	 * Tracks execution of a long running operation (usually asynchronous operations taking multiple seconds/minutes).
	 * Provides standardized of providing information about progress of the operation. 
	 */
	class B3D_EXPORT LongOperationTracker : public IScriptExportable
	{
	public:
		/**
		 * @param name			Name that describes the operation being executed.
		 * @param flags			Flags that describe how the operation is being executed.
		 * @param description	Additional information about the operation.
		 * @param category		Category that may be used for grouping similar operations together.
		 */
		LongOperationTracker(String name, LongOperationFlags flags = LongOperationFlag::None, String description = StringUtil::kBlank, String category = StringUtil::kBlank);
		virtual ~LongOperationTracker();

		/** Name of the operation, to be used in UI and debugging. */
		const String& GetName() const { return mName; }

		/** Description of the operation, to be used in UI and debugging. */
		const String& GetDescription() const { return mDescription; }

		/** Category of the operation, if provided. Can be used for grouping similar operations. */
		const String& GetCategory() const { return mCategory; }

		/** Returns true if the operation can be aborted. */
		bool CanBeAborted() const { return mFlags.IsSet(LongOperationFlag::CanBeAborted); }

		/** Returns true if progress between range [0, 1] is being reported. */
		bool ReportsProgress() const { return !mFlags.IsSet(LongOperationFlag::ReportsProgress); }

		/** Returns the current operation progress. Values between [0, 1] are reported only if operation can report progress. */
		float GetProgressPercent() const;

		/**
		 * @name Internal
		 * @{
		 */

		/** To be called by external code when the operation starts executing. */
		void NotifyOperationStarted();

		/**
		 * To be called by external code when the operation finished running and no errors were reported. Does nothing if operation
		 * is already in one of the finished states (success, failed or error).
		 */
		void NotifyOperationFinished();

		/**
		 * To be called by external code when the operation has processed the user's abort request. Does nothing if operation
		 * is already in one of the finished states (success, failed or error).
*/
		void NotifyOperationAborted();

		/**
		 * To be called by external code when the operation has failed with some error. Does nothing if operation
		 * is already in one of the other finished states (success or error). If operation is already in a failed state
		 * appends the provided error message to the existing error message.
		 */
		void NotifyOperationFailed(const String& error);

		/** Changes the progress percent of the operation. Thread safe. */
		void NotifyProgressChanged(float progressPercent);

		/** @} */
	protected:
		mutable Mutex mMutex;
		LongOperationState mState = LongOperationState::NotStarted;
		float mProgressPercent = 0.0f;
		String mError; /**< Error reported if the operation failed. */

		// Immutable
		String mName;
		String mDescription;
		String mCategory;
		LongOperationFlags mFlags;
	};

	/**
	 * Automatically notifies the tracker that the operation started when the scoped tracker is constructed, and finished when the scoped tracker
	 * is destructed (unless the operation was aborted or failed meanwhile).
	 */
	class ScopedLongOperationTracker : public LongOperationTracker
	{
	public:
		/** @copydoc LongOperationTracker::LongOperationTracker */
		ScopedLongOperationTracker(String name, LongOperationFlags flags = LongOperationFlag::None, String description = StringUtil::kBlank, String category = StringUtil::kBlank);
		~ScopedLongOperationTracker();
		
	};

	inline ScopedLongOperationTracker::ScopedLongOperationTracker(String name, LongOperationFlags flags, String description, String category)
		: LongOperationTracker(std::move(name), flags, std::move(description), std::move(category))
	{
		NotifyOperationStarted();
	}

	inline ScopedLongOperationTracker::~ScopedLongOperationTracker()
	{
		NotifyOperationFinished();
	}

	/** @} */
} // namespace b3d
