//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DSignal.h"
#include "B3DWaitGroup.h"
#include "Utility/B3DAny.h"

namespace b3d
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
	class B3D_UTILITY_EXPORT AsyncOp
	{
	protected:
		struct AsyncOpData
		{
			Any ReturnValue; // TODO - TAsyncOp should be specialized so we don't need another heap allocation here
			bool IsCompleted = false;
			WaitGroup ContinuationWaitGroup;
			Mutex Mutex;
			Signal Signal;
		};

	public:
		AsyncOp()
			: mData(B3DMakeShared<AsyncOpData>())
		{}

		AsyncOp(AsyncOpEmpty empty)
		{}

		AsyncOp(const AsyncOp& other) = default;
		AsyncOp(AsyncOp&& other)
			: mData(std::exchange(other.mData, nullptr))
		{ }

		AsyncOp& operator=(const AsyncOp& other) = default;
		AsyncOp& operator=(AsyncOp&& other)
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

		/** Calls the provided callback when the async operation completes. Callback is guaranteed to happen on the calling thread. */
		template<class F>
		void DoWhenComplete(F&& callback)
		{
			// If not initialized, nothing to wait on
			if(mData == nullptr)
			{
				B3D_LOG(Error, Generic, "Unable to trigger callback. Async operation was never initialized with data.");
				return;
			}

			bool isCompleted = false;
			{
				Lock lock(mData->Mutex);
				isCompleted = mData->IsCompleted;

				if(!isCompleted)
					mData->ContinuationWaitGroup.Increment();
			}

			if (isCompleted)
			{
				callback();
				return;
			}

			auto fnContinuation = [data = mData, callback = std::move(callback)]()
			{
				Lock lock(data->Mutex);
				data->Signal.Wait(lock, [data = data.get()]() { return data->IsCompleted; });

				callback();

				data->ContinuationWaitGroup.NotifyDone();
			};

			Scheduler* const scheduler = Scheduler::Get();
			if (!B3D_ENSURE(scheduler))
				return;

			scheduler->Post(SchedulerTask(std::move(fnContinuation), "AsyncOp continuation", SchedulerTaskFlag::SameThread));
		}

		/**
		 * Blocks the caller thread until the AsyncOp completes.
		 *
		 * @param	blockUntilCallbacksComplete		If true, this method will block until all registered completion callbacks finished executing as well. Otherwise, it will just wait
		 *											until the operation has completed, but callbacks might have not been triggered yet.
		 */
		void BlockUntilComplete(bool blockUntilCallbacksComplete = true) const
		{
			// If not initialized, nothing to wait on
			if(mData == nullptr)
			{
				B3D_LOG(Error, Generic, "Unable to block until complete. Async operation was never initialized with data.");
				return;
			}

			Lock lock(mData->Mutex);
			mData->Signal.Wait(lock, [this]() { return mData->IsCompleted; });

			if (blockUntilCallbacksComplete)
			{
				// Also need to wait for all continuation callbacks to fire
				mData->ContinuationWaitGroup.Wait();
			}
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
	 * HasCompleted() returns true.
	 *
	 * @note
	 * You are allowed (and meant to) to copy this by value.
	 */
	template <class ReturnType>
	class TAsyncOp : public AsyncOp
	{
	public:
		using ReturnValueType = ReturnType;

		TAsyncOp() = default;

		TAsyncOp(AsyncOpEmpty empty)
			: AsyncOp(empty)
		{}

		TAsyncOp(const TAsyncOp& other) = default;
		TAsyncOp(TAsyncOp&& other)
			: AsyncOp(std::move(other))
		{ }

		TAsyncOp& operator=(const TAsyncOp& other) = default;
		TAsyncOp& operator=(TAsyncOp&& other)
		{
			return static_cast<TAsyncOp&>(AsyncOp::operator=(std::move(other)));
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

			{
				Lock lock(mData->Mutex);

				mData->IsCompleted = true;
				mData->Signal.NotifyAll();
			}
		}

		/** Mark the async operation as completed. */
		void CompleteOperation(const ReturnType& returnValue)
		{
			if(mData == nullptr)
				mData = B3DMakeShared<AsyncOpData>();

			{
				Lock lock(mData->Mutex);

				mData->ReturnValue = returnValue;
				mData->IsCompleted = true;
				mData->Signal.NotifyAll();
			}
		}

		/** @} */
	protected:
		template <class ReturnType2>
		friend bool operator==(const TAsyncOp<ReturnType2>&, std::nullptr_t);
		template <class ReturnType2>
		friend bool operator!=(const TAsyncOp<ReturnType2>&, std::nullptr_t);
	};

	template <>
	class TAsyncOp<void> : public AsyncOp
	{
	public:
		TAsyncOp() = default;

		TAsyncOp(AsyncOpEmpty empty)
			: AsyncOp(empty)
		{}

		TAsyncOp(const TAsyncOp& other) = default;

		TAsyncOp(TAsyncOp&& other)
			: AsyncOp(std::move(other))
		{}

		TAsyncOp& operator=(const TAsyncOp& other) = default;

		TAsyncOp& operator=(TAsyncOp&& other)
		{
			return static_cast<TAsyncOp&>(AsyncOp::operator=(std::move(other)));
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

			{
				Lock lock(mData->Mutex);

				mData->IsCompleted = true;
				mData->Signal.NotifyAll();
			}
		}

		/** @} */
	protected:
		friend bool operator==(const TAsyncOp<void>&, std::nullptr_t);
		friend bool operator!=(const TAsyncOp<void>&, std::nullptr_t);
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

	/** @} */
} // namespace b3d
