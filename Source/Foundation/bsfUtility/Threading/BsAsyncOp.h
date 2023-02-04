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

	/** Flag used for creating async operations signaling that we want to create an empty AsyncOp with no internal memory storage. */
	struct B3D_UTILITY_EXPORT AsyncOpEmpty
	{};

	/** @} */
	/** @} */

	/** @addtogroup Threading
	 *  @{
	 */

	/** Common base for all TAsyncOp specializations. */
	class B3D_UTILITY_EXPORT AsyncOpBase
	{
	protected:
		struct AsyncOpData
		{
			Any ReturnValue;
			Function<void()> FlushCallback;
			SmallVector<Function<void(const AsyncOpBase&)>, 1> CompletionCallbacks;
			bool IsCompleted = false;
			Mutex Mutex;
			Signal Signal;
		};

	public:
		AsyncOpBase()
			: mData(B3DMakeShared<AsyncOpData>())
		{}

		AsyncOpBase(AsyncOpEmpty empty)
		{}

		AsyncOpBase(const AsyncOpBase& other) = default;
		AsyncOpBase(AsyncOpBase&& other)
			: mData(std::exchange(other.mData, nullptr))
		{ }

		AsyncOpBase& operator=(const AsyncOpBase& other) = default;
		AsyncOpBase& operator=(AsyncOpBase&& other)
		{
			if(&other != this)
			{
				mData = std::exchange(other.mData, nullptr);
			}

			return *this;
		}

		/** Returns true if the async operation has completed. */
		bool HasCompleted() const
		{
			if(mData == nullptr)
				return false;

			Lock lock(mData->Mutex);
			return mData->IsCompleted;
		}

		/** Assigns a callback that will trigger during a BlockUntilComplete() call. Allows the operation to perform additional work before waiting on completion. */
		void SetFlushCallback(Function<void()> callback)
		{
			if(mData == nullptr)
				mData = B3DMakeShared<AsyncOpData>();

			Lock lock(mData->Mutex);
			mData->FlushCallback = std::move(callback);
		}

		/** Assigns a callback that triggers when the async operation completes. Triggers immediately if already completed. */
		void DoOnComplete(Function<void(const AsyncOpBase&)> callback)
		{
			if(mData == nullptr)
				mData = B3DMakeShared<AsyncOpData>();

			{
				Lock lock(mData->Mutex);

				if(!mData->IsCompleted)
				{
					mData->CompletionCallbacks.Add(std::move(callback));
					return;
				}
			}

			callback(*this);
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
			// If not initialized, nothing to wait on
			if(mData == nullptr)
			{
				B3D_LOG(Error, Generic, "Unable to block until complete. Async operation was never initialized with data.");
				return;
			}

			Function<void()> flushCallback;
			{
				Lock lock(mData->Mutex);
				flushCallback = mData->FlushCallback;
			}

			if(flushCallback != nullptr)
			{
				if(mData->IsCompleted)
					return;

				// No need to flush if already completed
				{
					Lock lock(mData->Mutex);
					if(mData->IsCompleted)
						return;
				}

				flushCallback();
			}

			Lock lock(mData->Mutex);
			while(!mData->IsCompleted)
				mData->Signal.wait(lock);
		}

		/**
		 * Retrieves the value returned by the async operation as a generic type. Only valid if hasCompleted() returns
		 * true.
		 */
		Any GetGenericReturnValue() const
		{
			if(mData == nullptr)
				return Any();

			Lock lock(mData->Mutex);
#if B3D_DEBUG
			if(!mData->IsCompleted)
				B3D_LOG(Error, Generic, "Trying to get AsyncOp return value but the operation hasn't completed.");
#endif

			return mData->ReturnValue;
		}

	protected:
		SPtr<AsyncOpData> mData;
	};

	/**
	 * Object you may use to check on the results of an asynchronous operation. Contains uninitialized data until
	 * hasCompleted() returns true.
	 *
	 * @note
	 * You are allowed (and meant to) to copy this by value.
	 */
	template <class ReturnType>
	class TAsyncOp : public AsyncOpBase
	{
	public:
		using ReturnValueType = ReturnType;

		TAsyncOp() = default;

		TAsyncOp(AsyncOpEmpty empty)
			: AsyncOpBase(empty)
		{}

		TAsyncOp(const TAsyncOp& other) = default;
		TAsyncOp(TAsyncOp&& other)
			: AsyncOpBase(std::move(other))
		{ }

		TAsyncOp& operator=(const TAsyncOp& other) = default;
		TAsyncOp& operator=(TAsyncOp&& other)
		{
			return static_cast<TAsyncOp&>(AsyncOpBase::operator=(std::move(other)));
		}

		/** Retrieves the value returned by the async operation. Only valid if hasCompleted() returns true. */
		ReturnType GetReturnValue() const
		{
			B3D_ASSERT(mData != nullptr);

			Lock lock(mData->Mutex);

#if B3D_DEBUG
			if(!mData->IsCompleted)
				B3D_LOG(Error, Generic, "Trying to get AsyncOp return value but the operation hasn't completed.");
#endif

			return AnyCast<ReturnType>(mData->ReturnValue);
		}

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Mark the async operation as completed, without setting a return value. */
		void CompleteOperation()
		{
			if(mData == nullptr)
				mData = B3DMakeShared<AsyncOpData>();

			SmallVector<Function<void(const AsyncOpBase&)>, 1> callbacks;
			{
				Lock lock(mData->Mutex);

				mData->IsCompleted = true;
				mData->Signal.notify_all();

				std::swap(callbacks, mData->CompletionCallbacks);
			}

			for(auto& callback : callbacks)
			{
				if(callback == nullptr)
					continue;

				callback(*this);
			}
		}

		/** Mark the async operation as completed. */
		void CompleteOperation(const ReturnType& returnValue)
		{
			if(mData == nullptr)
				mData = B3DMakeShared<AsyncOpData>();

			SmallVector<Function<void(const AsyncOpBase&)>, 1> callbacks;
			{
				Lock lock(mData->Mutex);

				mData->ReturnValue = returnValue;
				mData->IsCompleted = true;
				mData->Signal.notify_all();

				std::swap(callbacks, mData->CompletionCallbacks);
			}

			for(auto& callback : callbacks)
			{
				if(callback == nullptr)
					continue;

				callback(*this);
			}
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
