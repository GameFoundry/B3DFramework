//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Debug/BsDebug.h"
#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/** Interface for a RTTI iterator. */
	class IRTTIIterator
	{
	public:
		virtual ~IRTTIIterator() = default;

		/** Returns true if the iterator points to a valid value. */
		virtual bool IsValid() const = 0;

		/** Resets the iterator to the beginning of the container. */
		virtual void ResetToBeginning() = 0;

		/** Resets the iterator to the end of the container. */
		virtual void ResetToEnd() = 0;

		/** Returns the number of elements in the container. */
		virtual u64 GetElementCount() const = 0;

		/** Assigns the value at the current iterator location. */
		virtual void SetValue(const void* value) = 0;

		/** Returns the current value of the iterator. */
		virtual const void* GetValue() const = 0;

		/** Increment operator. */
		virtual void Increment() = 0;
	};

	/** Provides an adapter that allows TRTTIIterator<T> to iterate over some type T that might not provide the default iterator interface. */
	template<class T, bool IsContainer>
	struct TRTTIIteratorAdapter { };

	/** Implementation that allows RTTIIterator to access a non-container type, as a faux iterator with one entry. */
	template<class T>
	struct TRTTIIteratorAdapter<T, false>
	{
		using IteratorType = T*;
		using ElementType = T;

		static IteratorType Begin(T& container) { return &container; }
		static IteratorType End(T& container) { return nullptr; }
		static bool IsValid(T& container, IteratorType iterator) { return iterator != nullptr; }
		static IteratorType Insert(T& container, IteratorType location, const ElementType& element) { *location = element; return location; }
		static IteratorType Insert(T& container, IteratorType location, ElementType&& element) { *location = std::move(element); return location; }
		static IteratorType Increment(IteratorType iterator) { iterator = nullptr; return iterator; }
		static ElementType& GetValue(IteratorType iterator) { return *iterator; }
		static u64 Size(T& container) { return 1; }
		
	};

	/** Implementation that allows RTTIIterator to access a standard container. */
	template<class T>
	struct TRTTIIteratorAdapter<T, true>
	{
		using IteratorType = typename T::iterator;
		using ElementType = typename T::value_type;

		static IteratorType Begin(T& container) { return container.begin(); }
		static IteratorType End(T& container) { return container.end(); }
		static bool IsValid(T& container, IteratorType iterator) { return iterator != container.end(); }
		static IteratorType Insert(T& container, IteratorType location, const ElementType& element) { return container.insert(location, element); }
		static IteratorType Insert(T& container, IteratorType location, ElementType&& element) { return container.insert(location, std::move(element)); }
		static IteratorType Increment(IteratorType iterator) { ++iterator; return iterator; }
		static ElementType& GetValue(IteratorType iterator) { return *iterator; }
		static u64 Size(T& container) { return (u64)container.size(); }
	};

	/**
	 * Wraps a container that can be used for sequentially reading container contents, inserting new elements in the container, and retrieving container element count.
	 *
	 * @tparam DataType		Data type to iterate over.
	 * @tparam IsContainer	Set to true if you wish to iterate over DataType as a container with multiple elements. If false, the iterator
	 *						acts as a faux iterator with a single entry, directly referencing the provided data type.
	 */
	template <class DataType, bool IsContainer>
	class TRTTIIterator : public IRTTIIterator
	{
	public:
		using IteratorAdapter = TRTTIIteratorAdapter<DataType, IsContainer>;
		using IteratorType = typename IteratorAdapter::IteratorType;
		using ElementType = typename IteratorAdapter::ElementType;

		TRTTIIterator(DataType& value)
			: mValue(&value), mIterator(IteratorAdapter::Begin(value))
		{}

		bool IsValid() const override { return IteratorAdapter::IsValid(*mValue, mIterator); }
		void ResetToBeginning() override { mIterator = IteratorAdapter::Begin(*mValue); }
		void ResetToEnd() override { mIterator = IteratorAdapter::End(*mValue); }
		u64 GetElementCount() const override { return IteratorAdapter::Size(*mValue); }
		void SetValue(const void* value) override { operator=(*static_cast<const ElementType*>(value)); }
		const void* GetValue() const override { return &(*mIterator); }
		void Increment() override { operator++(); }

		/** Assigns (copies) the value at the current iterator location. */
		TRTTIIterator& operator=(const ElementType& value)
		{
			mIterator = IteratorAdapter::Insert(*mValue, mIterator, value);
			mIterator = IteratorAdapter::Increment(mIterator);

			return *this;
		}

		/** Assigns (moves) the value at the current iterator location. */
		TRTTIIterator& operator=(ElementType&& value)
		{
			mIterator = IteratorAdapter::Insert(*mValue, mIterator, std::move(value));
			mIterator = IteratorAdapter::Increment(mIterator);

			return *this;
		}

		/** Returns the current value of the iterator. */
		ElementType& operator*()
		{
			return IteratorAdapter::GetValue(mIterator);
		}

		/** Pre-increment operator. */
		TRTTIIterator& operator++()
		{
			mIterator = IteratorAdapter::Increment(mIterator);
			return *this;
		}

	protected:
		DataType* mValue = nullptr;
		IteratorType mIterator;
	};

	/** Deleter that can be passed to unique pointer referencing TRTTIIterator<DataType, IsContainer>. */
	template<typename DataType, bool IsContainer>
	struct TRTTIIteratorDeleter
	{
		TRTTIIteratorDeleter(FrameAllocator* allocator = nullptr)
			: mAllocator(allocator)
		{ }

		void operator()(TRTTIIterator<DataType, IsContainer>* iterator)
		{
			if(B3D_ENSURE(mAllocator != nullptr))
				mAllocator->Destruct(iterator);
		}

	private:
		FrameAllocator* mAllocator;
	};

	/** @} */
	/** @} */
} // namespace bs
