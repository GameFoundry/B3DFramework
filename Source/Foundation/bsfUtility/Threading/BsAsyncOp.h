//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Error/BsException.h"
#include "Utility/BsAny.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Threading-Internal
	 *  @{
	 */

	/** Thread synchronization primitives used by AsyncOps and their callers. */
	class BS_UTILITY_EXPORT AsyncOpSyncData
	{
	public:
		Mutex mMutex;
		Signal mCondition;
	};

	/**
	 * Flag used for creating async operations signaling that we want to create an empty AsyncOp with no internal 
	 * memory storage.
	 */
	struct BS_UTILITY_EXPORT AsyncOpEmpty {};

	/** @} */
	/** @} */

	/** @addtogroup Threading
	 *  @{
	 */

	/**
	 * Object you may use to check on the results of an asynchronous operation. Contains uninitialized data until 
	 * hasCompleted() returns true. 
	 * 			
	 * @note	
	 * You are allowed (and meant to) to copy this by value.
	 * @note
	 * You'll notice mIsCompleted isn't synchronized. This is because we're okay if mIsCompleted reports true a few cycles 
	 * too late, which is not relevant for practical use. And in cases where you need to ensure operation has completed 
	 * you will usually use some kind of synchronization primitive that includes a memory barrier anyway.
	 */
	class BS_UTILITY_EXPORT AsyncOp
	{
	private:
		struct AsyncOpData
		{
			AsyncOpData() = default;

			Any mReturnValue;
			volatile std::atomic<bool> mIsCompleted{false};
		};

	public:
		AsyncOp()
			:mData(bs_shared_ptr_new<AsyncOpData>())
		{ }

		AsyncOp(AsyncOpEmpty empty)
		{ }

		AsyncOp(const SPtr<AsyncOpSyncData>& syncData)
			:mData(bs_shared_ptr_new<AsyncOpData>()), mSyncData(syncData)
		{ }

		AsyncOp(AsyncOpEmpty empty, const SPtr<AsyncOpSyncData>& syncData)
			:mSyncData(syncData)
		{ }

		/** Returns true if the async operation has completed. */
		bool hasCompleted() const;

		/**
		 * Blocks the caller thread until the AsyncOp completes.
		 *
		 * @note
		 * Do not call this on the thread that is completing the async op, as it will cause a deadlock. Make sure the 
		 * command you are waiting for is actually queued for execution because a deadlock will occur otherwise.
		 */
		void blockUntilComplete() const;

		/** Retrieves the value returned by the async operation. Only valid if hasCompleted() returns true. */
		template <typename T>
		T getReturnValue() const 
		{ 
#if BS_DEBUG_MODE
			if(!hasCompleted())
				BS_EXCEPT(InternalErrorException, "Trying to get AsyncOp return value but the operation hasn't completed.");
#endif
			// Be careful if cast throws an exception. It doesn't support casting of polymorphic types. Provided and returned
			// types must be EXACT. (You'll have to cast the data yourself when completing the operation)
			return any_cast<T>(mData->mReturnValue);
		}

		/** 
		 * Retrieves the value returned by the async operation as a generic type. Only valid if hasCompleted() returns 
		 * true. 
		 */
		Any getGenericReturnValue() const { return mData->mReturnValue; }

	public: // ***** INTERNAL ******
		/** @name Internal 
		 *  @{
		 */

		/** Mark the async operation as completed. */
		void _completeOperation(Any returnValue);

		/** Mark the async operation as completed, without setting a return value. */
		void _completeOperation();

		/** @} */
	private:
		friend bool operator==(const AsyncOp&, std::nullptr_t);
		friend bool operator!=(const AsyncOp&, std::nullptr_t);

		SPtr<AsyncOpData> mData;
		SPtr<AsyncOpSyncData> mSyncData;
	};

	/**	Checks if an AsyncOp is null. */
	inline bool operator==(const AsyncOp& lhs, std::nullptr_t rhs)
	{	
		return lhs.mData == nullptr;
	}

	/**	Checks if an AsyncOp is not null. */
	inline bool operator!=(const AsyncOp& lhs, std::nullptr_t rhs)
	{	
		return lhs.mData != nullptr;
	}

	/** @} */
}
