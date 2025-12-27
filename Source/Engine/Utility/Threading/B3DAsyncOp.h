//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DSignal.h"
#include "B3DWaitGroup.h"
#include "Utility/B3DAny.h"  // Still needed for GetGenericReturnValue()

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Threading-Internal
	 *  @{
	 */

	/** Flag used for creating async operations signaling that we want to create an empty AsyncOp with no internal memory storage. */
	struct B3D_EXPORT AsyncOpEmpty
	{};

	/**
	 * Type-erased function signature for extracting return value as Any.
	 * Used by scripting layer to get values without knowing the concrete type.
	 */
	using GetValueAsFn = Any(*)(const void* data);

	/**
	 * Base structure for AsyncOp data storage. Contains common fields for all async operations.
	 * Derived classes add type-specific return value storage.
	 *
	 * @note NO virtual functions - uses type-erased callback for scripting compatibility.
	 *       Proper cleanup is handled via custom shared_ptr deleter.
	 */
	struct B3D_EXPORT AsyncOpDataBase
	{
		bool IsCompleted = false;
		WaitGroup ContinuationWaitGroup;
		Mutex Mutex;
		Signal Signal;
		GetValueAsFn GetValueFn = nullptr;  // Type-erased value extractor

		~AsyncOpDataBase() = default;  // Non-virtual
	};

	/** Typed data structure that stores the return value inline. */
	template<typename T>
	struct TAsyncOpData final : AsyncOpDataBase
	{
		TOptional<T> ReturnValue;

		/** Static function to extract value as Any (used for scripting). */
		static Any ExtractValue(const void* data)
		{
			auto* self = static_cast<const TAsyncOpData<T>*>(data);
			if (self->ReturnValue.has_value())
				return Any(*self->ReturnValue);
			return Any();
		}
	};

	/** Specialization for void - no return value storage, zero overhead. */
	template<>
	struct TAsyncOpData<void> final : AsyncOpDataBase
	{
		/** Static function returns empty Any for void operations. */
		static Any ExtractValue(const void* /*data*/)
		{
			return Any();
		}
	};

	/** @} */
	/** @} */

	/** @addtogroup Threading
	 *  @{
	 */

	/** Common base for all TAsyncOp specializations. */
	class B3D_EXPORT AsyncOp
	{
	public:
		AsyncOp(AsyncOpEmpty empty)
		{}

		AsyncOp(const AsyncOp& other) = default;

		AsyncOp(AsyncOp&& other)
			: mData(std::exchange(other.mData, nullptr))
		{}

		AsyncOp& operator=(const AsyncOp& other) = default;

		AsyncOp& operator=(AsyncOp&& other)
		{
			if (&other != this)
				mData = std::exchange(other.mData, nullptr);
			return *this;
		}

		/** Returns true if the async operation has completed. */
		bool HasCompleted() const
		{
			if (mData == nullptr)
				return false;

			Lock lock(mData->Mutex);
			return mData->IsCompleted;
		}

		/** Calls the provided callback when the async operation completes. Callback is guaranteed to happen on the calling thread. */
		template<class F>
		void DoWhenComplete(F&& callback)
		{
			// If not initialized, nothing to wait on
			if (mData == nullptr)
			{
				B3D_LOG(Error, Generic, "Unable to trigger callback. Async operation was never initialized with data.");
				return;
			}

			bool isCompleted = false;
			{
				Lock lock(mData->Mutex);
				isCompleted = mData->IsCompleted;

				if (!isCompleted)
					mData->ContinuationWaitGroup.Increment();
			}

			if (isCompleted)
			{
				callback();
				return;
			}

			auto fnContinuation = [data = mData, callback = std::move(callback)]() mutable
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
			if (mData == nullptr)
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
		 * Retrieves the value returned by the async operation as a generic type.
		 * Only valid if HasCompleted() returns true.
		 *
		 * @note Primarily used for scripting interop. Prefer GetReturnValue() in TAsyncOp<T>.
		 */
		Any GetGenericReturnValue() const
		{
			if (mData == nullptr)
				return Any();

			Lock lock(mData->Mutex);
#if B3D_DEBUG
			if (!mData->IsCompleted)
				B3D_LOG(Error, Generic, "Trying to get AsyncOp return value but the operation hasn't completed.");
#endif

			// Call type-erased function pointer (no virtual dispatch)
			if (mData->GetValueFn)
				return mData->GetValueFn(mData.get());
			return Any();
		}

	protected:
		AsyncOp() = default;

		explicit AsyncOp(SPtr<AsyncOpDataBase> data)
			: mData(std::move(data))
		{}

		SPtr<AsyncOpDataBase> mData;
	};

	/**
	 * Object you may use to check on the results of an asynchronous operation. Contains uninitialized data until
	 * HasCompleted() returns true.
	 *
	 * @note
	 * You are allowed (and meant to) to copy this by value.
	 */
	template<typename ReturnType>
	class TAsyncOp : public AsyncOp
	{
	public:
		using ReturnValueType = ReturnType;

		TAsyncOp()
			: AsyncOp(CreateData())
		{}

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

		/**
		 * Retrieves the value returned by the async operation. Only valid if HasCompleted() returns true.
		 *
		 * @return Copy of the return value. Returns by value for thread safety.
		 */
		ReturnType GetReturnValue() const
		{
			B3D_ASSERT(mData != nullptr);

			auto* typedData = static_cast<TAsyncOpData<ReturnType>*>(mData.get());
			Lock lock(typedData->Mutex);

#if B3D_DEBUG
			if (!typedData->IsCompleted)
				B3D_LOG(Error, Generic, "Trying to get AsyncOp return value but the operation hasn't completed.");
			if (!typedData->ReturnValue.has_value())
				B3D_LOG(Error, Generic, "AsyncOp completed but no return value was set.");
#endif

			return typedData->ReturnValue.value_or(ReturnType{});
		}

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Mark the async operation as completed, without setting a return value. */
		void CompleteOperation()
		{
			if (mData == nullptr)
				mData = CreateData();

			auto* typedData = static_cast<TAsyncOpData<ReturnType>*>(mData.get());
			Lock lock(typedData->Mutex);

			typedData->IsCompleted = true;
			typedData->Signal.NotifyAll();
		}

		/** Mark the async operation as completed with the given return value (copy). */
		void CompleteOperation(const ReturnType& returnValue)
		{
			if (mData == nullptr)
				mData = CreateData();

			auto* typedData = static_cast<TAsyncOpData<ReturnType>*>(mData.get());
			Lock lock(typedData->Mutex);

			typedData->ReturnValue = returnValue;
			typedData->IsCompleted = true;
			typedData->Signal.NotifyAll();
		}

		/** Mark the async operation as completed with the given return value (move). */
		void CompleteOperation(ReturnType&& returnValue)
		{
			if (mData == nullptr)
				mData = CreateData();

			auto* typedData = static_cast<TAsyncOpData<ReturnType>*>(mData.get());
			Lock lock(typedData->Mutex);

			typedData->ReturnValue = std::move(returnValue);
			typedData->IsCompleted = true;
			typedData->Signal.NotifyAll();
		}

		/** @} */

	protected:
		template<typename ReturnType2>
		friend bool operator==(const TAsyncOp<ReturnType2>&, std::nullptr_t);
		template<typename ReturnType2>
		friend bool operator!=(const TAsyncOp<ReturnType2>&, std::nullptr_t);

	private:
		/** Creates typed data with custom deleter and sets up type-erased value extractor. */
		static SPtr<AsyncOpDataBase> CreateData()
		{
			auto* data = B3DNew<TAsyncOpData<ReturnType>>();
			data->GetValueFn = &TAsyncOpData<ReturnType>::ExtractValue;

			return SPtr<AsyncOpDataBase>(
				data,
				[](AsyncOpDataBase* p) { B3DDelete(static_cast<TAsyncOpData<ReturnType>*>(p)); }
			);
		}
	};

	/**
	 * Specialization of TAsyncOp for void return type (no return value).
	 * Uses AsyncOpDataTyped<void> which has zero storage overhead for return value.
	 */
	template<>
	class TAsyncOp<void> : public AsyncOp
	{
	public:
		TAsyncOp()
			: AsyncOp(CreateData())
		{}

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

		/** Mark the async operation as completed (no return value). */
		void CompleteOperation()
		{
			if (mData == nullptr)
				mData = CreateData();

			auto* typedData = static_cast<TAsyncOpData<void>*>(mData.get());
			Lock lock(typedData->Mutex);

			typedData->IsCompleted = true;
			typedData->Signal.NotifyAll();
		}

		/** @} */

	protected:
		friend bool operator==(const TAsyncOp<void>&, std::nullptr_t);
		friend bool operator!=(const TAsyncOp<void>&, std::nullptr_t);

	private:
		/** Creates typed data with custom deleter and sets up type-erased value extractor. */
		static SPtr<AsyncOpDataBase> CreateData()
		{
			auto* data = B3DNew<TAsyncOpData<void>>();
			data->GetValueFn = &TAsyncOpData<void>::ExtractValue;

			return SPtr<AsyncOpDataBase>(
				data,
				[](AsyncOpDataBase* p) { B3DDelete(static_cast<TAsyncOpData<void>*>(p)); }
			);
		}
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
