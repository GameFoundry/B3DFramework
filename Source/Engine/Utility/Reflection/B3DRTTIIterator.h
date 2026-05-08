//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Debug/B3DDebug.h"
#include "B3DUtilityPrerequisites.h"

namespace b3d
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
		virtual void SeekToBeginning() = 0;

		/** Resets the iterator to the end of the container. */
		virtual void SeekToEnd() = 0;

		/** Returns the number of elements in the container. */
		virtual u64 GetElementCount() const = 0;

		/** Returns the current value of the iterator. */
		virtual const void* GetValue() const = 0;

		/** Increment operator. */
		virtual void Increment() = 0;

		/** Clears all the entries from the underlying container. */
		virtual void Clear() = 0;

		/** Removes the current iterator element. */
		virtual void Erase() = 0;

		/** Makes a copy of this iterator at its current location. */
		virtual TShared<IRTTIIterator> Clone(FrameAllocator& allocator) const = 0;

		/** Seeks the iterator to a particular index. If the index is out of range, or seeking to index is not supported, the iterator is marked as invalid and false is returned. */
		virtual bool SeekToIndex(u64 index) = 0;

		/**
		 * Seeks the iterator to a particular key. The provided value should be a pointer to the key value, or to a tuple. In case the value is a tuple, the first
		 * element is considered to be the key. If the kye is not found, or seeking to key is not supported, the iterator is marked as invalid and false is returned. */
		virtual bool SeekToKey(const void* value) = 0;
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
		static IteratorType InsertAtEnd(T& container, const ElementType& element) { container = element; return &container; }
		static IteratorType InsertAtEnd(T& container, ElementType&& element) { container = std::move(element); return &container; }
		static IteratorType SetValue(T& container, IteratorType location, const ElementType& element) { *location = element; return location; }
		static IteratorType SetValue(T& container, IteratorType location, ElementType&& element) { *location = std::move(element); return location; }
		static IteratorType Increment(IteratorType iterator) { iterator = nullptr; return iterator; }
		static ElementType& GetValue(IteratorType iterator) { return *iterator; }
		static IteratorType Erase(T& container, IteratorType iterator) { return iterator; }
		static void Clear(T& container) { }
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
		static IteratorType InsertAtEnd(T& container, const ElementType& element) { return container.insert(container.end(), element); }
		static IteratorType InsertAtEnd(T& container, ElementType&& element) { return container.insert(container.end(), std::move(element)); }
		static IteratorType SetValue(T& container, IteratorType location, const ElementType& element)
		{
			typename std::iterator_traits<IteratorType>::difference_type arrayIndex = std::distance(container.begin(), location);

			container[arrayIndex] = element;
			return container.begin() + arrayIndex;
		}
		static IteratorType SetValue(T& container, IteratorType location, ElementType&& element)
		{
			typename std::iterator_traits<IteratorType>::difference_type arrayIndex = std::distance(container.begin(), location);

			container[arrayIndex] = std::move(element);
			return container.begin() + arrayIndex;
		}
		static IteratorType Increment(IteratorType iterator) { ++iterator; return iterator; }
		static ElementType& GetValue(IteratorType iterator) { return const_cast<ElementType&>(*iterator); }
		static IteratorType Erase(T& container, IteratorType iterator) { return container.erase(iterator); }
		static void Clear(T& container) { container.clear(); }
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

		TRTTIIterator(DataType& value, IteratorType iterator)
			: mValue(&value), mIterator(iterator)
		{}

		bool IsValid() const override { return IteratorAdapter::IsValid(*mValue, mIterator); }
		void SeekToBeginning() override { mIterator = IteratorAdapter::Begin(*mValue); }
		void SeekToEnd() override { mIterator = IteratorAdapter::End(*mValue); }
		u64 GetElementCount() const override { return IteratorAdapter::Size(*mValue); }
		const void* GetValue() const override { return &(*mIterator); }
		void Increment() override { operator++(); }
		TShared<IRTTIIterator> Clone(FrameAllocator& allocator) const override;
		void Clear() override { IteratorAdapter::Clear(*mValue); }
		void Erase() override
		{
			if(IsValid())
				mIterator = IteratorAdapter::Erase(*mValue, mIterator);
			
		}

		bool SeekToIndex(u64 index) override
		{
			if constexpr(SupportsSeekToIndex())
			{
				if(index >= GetElementCount())
				{
					mIterator = IteratorAdapter::End(*mValue);
					return false;
				}

				mIterator = IteratorAdapter::Begin(*mValue) + index;
				return true;
			}

			return false;
		}

		bool SeekToKey(const void* value) override
		{
			if constexpr(SupportsSeekToKey())
			{
				const typename DataType::key_type& key = *static_cast<const typename DataType::key_type*>(value);
				mIterator = mValue->find(key);

				return IsValid();
			}

			return false;
		}

		/** Assigns (copies) the value at the current iterator location, or if the iterator is not valid inserts the value at the end of the container. */
		TRTTIIterator& operator=(const ElementType& value)
		{
			if(IsValid())
			{
				if constexpr(SupportsSeekToKey()) // Key values are const, so we need to erase and insert value
				{
					mIterator = IteratorAdapter::Erase(*mValue, mIterator);
					mIterator = IteratorAdapter::Insert(*mValue, mIterator, value);
				}
				else // Assuming SeekToIndex() (and therefore random access iterator) is supported instead
					mIterator = IteratorAdapter::SetValue(*mValue, mIterator, value);
			}
			else
				mIterator = IteratorAdapter::InsertAtEnd(*mValue, value);

			return *this;
		}

		/** Assigns (moves) the value at the current iterator location, or if the iterator is not valid inserts the value at the end of the container. */
		TRTTIIterator& operator=(ElementType&& value)
		{
			if(IsValid())
			{
				if constexpr(SupportsSeekToKey()) // Key values are const, so we need to erase and insert value
				{
					mIterator = IteratorAdapter::Erase(*mValue, mIterator);
					mIterator = IteratorAdapter::Insert(*mValue, mIterator, std::move(value));
				}
				else // Assuming SeekToIndex() (and therefore random access iterator) is supported instead
					mIterator = IteratorAdapter::SetValue(*mValue, mIterator, std::move(value));
			}
			else
				mIterator = IteratorAdapter::InsertAtEnd(*mValue, std::move(value));

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

		/** Returns true if SeekToIndex() may be called on the iterator. Generally true for array containers. */
		static constexpr bool SupportsSeekToIndex()
		{
			if constexpr(!IsContainer)
				return false;

			using category = typename std::iterator_traits<IteratorType>::iterator_category;
			if constexpr(std::is_same_v<category, std::random_access_iterator_tag>)
				return true;

			return false;
		}

	private:
		template<typename T, typename = void>
		struct HasKeyType : std::false_type {};

		template<typename T>
		struct HasKeyType<T, std::void_t<typename T::key_type>> : std::true_type {};

		template<typename T, typename = void>
		struct HasFind : std::false_type {};

		template<typename T>
		struct HasFind<T, std::void_t<decltype(std::declval<T>().find(std::declval<typename T::key_type>()))>> : std::true_type
		{};

	public:
		/** Returns true if SeekToKey() may be called on the iterator. Generally true for set/map containers. */
		static constexpr bool SupportsSeekToKey()
		{
			if constexpr(!IsContainer)
				return false;

			if constexpr(HasKeyType<DataType>::value && HasFind<DataType>::value)
				return true;

			return false;
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

	template <class DataType, bool IsContainer>
	TShared<IRTTIIterator> TRTTIIterator<DataType, IsContainer>::Clone(FrameAllocator& allocator) const
	{
		auto* const iterator = allocator.Construct<TRTTIIterator>(*mValue, mIterator);
		return B3DMakeSharedFromExisting<TRTTIIterator>(iterator, TRTTIIteratorDeleter<DataType, IsContainer>(&allocator));
	}

	/** @} */
	/** @} */
} // namespace b3d
