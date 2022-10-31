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
		Mutex MMutex;
		Signal MCondition;
	};

	/**
	 * Flag used for creating async operations signaling that we want to create an empty AsyncOp with no internal
	 * memory storage.
	 */
	struct BS_UTILITY_EXPORT AsyncOpEmpty
	{};

	/** @} */
	/** @} */

	/** @addtogroup Threading
	 *  @{
	 */

	/** Common base for all TAsyncOp specializations. */
	class BS_UTILITY_EXPORT AsyncOpBase
	{
	private:
		struct AsyncOpData
		{
			AsyncOpData() = default;

			Any MReturnValue;
			volatile std::atomic<bool> MIsCompleted{ false };
		};

	public:
		AsyncOpBase()
			: mData(B3DMakeShared<AsyncOpData>())
		{}

		AsyncOpBase(AsyncOpEmpty empty)
		{}

		AsyncOpBase(const SPtr<AsyncOpSyncData>& syncData)
			: mData(B3DMakeShared<AsyncOpData>()), mSyncData(syncData)
		{}

		AsyncOpBase(AsyncOpEmpty empty, const SPtr<AsyncOpSyncData>& syncData)
			: mSyncData(syncData)
		{}

		/** Returns true if the async operation has completed. */
		bool HasCompleted() const
		{
			return mData->MIsCompleted.load(std::memory_order_acquire);
		}

		/**
		 * Blocks the caller thread until the AsyncOp completes.
		 *
		 * @note
		 * Do not call this on the thread that is completing the async op, as it will cause a deadlock. Make sure the
		 * command you are waiting for is actually queued for execution because a deadlock will occur otherwise.
		 */
		void BlockUntilComplete() const
		{
			if(mSyncData == nullptr)
			{
				BS_LOG(Error, Generic, "No sync data is available. Cannot block until AsyncOp is complete.");
				return;
			}

			Lock lock(mSyncData->MMutex);
			while(!HasCompleted())
				mSyncData->MCondition.wait(lock);
		}

		/**
		 * Retrieves the value returned by the async operation as a generic type. Only valid if hasCompleted() returns
		 * true.
		 */
		Any GetGenericReturnValue() const
		{
#if BS_DEBUG_MODE
			if(!HasCompleted())
				BS_LOG(Error, Generic, "Trying to get AsyncOp return value but the operation hasn't completed.");
#endif

			return mData->MReturnValue;
		}

	protected:
		SPtr<AsyncOpData> mData;
		SPtr<AsyncOpSyncData> mSyncData;
	};

	/**
	 * Object you may use to check on the results of an asynchronous operation. Contains uninitialized data until
	 * hasCompleted() returns true.
	 *
	 * @note
	 * You are allowed (and meant to) to copy this by value.
	 */
	template <class ReturnType>
	class BS_UTILITY_EXPORT TAsyncOp : public AsyncOpBase
	{
	public:
		using ReturnValueType = ReturnType;

		TAsyncOp() = default;

		TAsyncOp(AsyncOpEmpty empty)
			: AsyncOpBase(empty)
		{}

		TAsyncOp(const SPtr<AsyncOpSyncData>& syncData)
			: AsyncOpBase(syncData)
		{}

		TAsyncOp(AsyncOpEmpty empty, const SPtr<AsyncOpSyncData>& syncData)
			: AsyncOpBase(empty, syncData)
		{}

		/** Retrieves the value returned by the async operation. Only valid if hasCompleted() returns true. */
		ReturnType GetReturnValue() const
		{
#if BS_DEBUG_MODE
			if(!HasCompleted())
				BS_LOG(Error, Generic, "Trying to get AsyncOp return value but the operation hasn't completed.");
#endif

			return any_cast<ReturnType>(mData->MReturnValue);
		}

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Mark the async operation as completed, without setting a return value. */
		void CompleteOperationInternal()
		{
			mData->MIsCompleted.store(true, std::memory_order_release);

			if(mSyncData != nullptr)
				mSyncData->MCondition.notify_all();
		}

		/** Mark the async operation as completed. */
		void CompleteOperationInternal(const ReturnType& returnValue)
		{
			mData->MReturnValue = returnValue;
			CompleteOperationInternal();
		}

		/** @} */
	protected:
		template <class ReturnType2>
		friend bool operator==(const TAsyncOp<ReturnType2>&, std::nullptr_t);
		template <class ReturnType2>
		friend bool operator!=(const TAsyncOp<ReturnType2>&, std::nullptr_t);
	};

	/**	Checks if an AsyncOp is null. */
	template <class ReturnType>
	bool operator==(const TAsyncOp<ReturnType>& lhs, std::nullptr_t rhs)
	{
		return lhs.mData == nullptr;
	}

	/**	Checks if an AsyncOp is not null. */
	template <class ReturnType>
	bool operator!=(const TAsyncOp<ReturnType>& lhs, std::nullptr_t rhs)
	{
		return lhs.mData != nullptr;
	}

	/** @copydoc TAsyncOp */
	using AsyncOp = TAsyncOp<Any>;

	/** @} */
} // namespace bs
