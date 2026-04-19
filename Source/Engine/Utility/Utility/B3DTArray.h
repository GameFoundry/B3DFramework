//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Containers
	 *  @{
	 */

	/** Dynamically sized array, similar to std::vector. */
	template <class Type, class Allocator = DefaultContainerAllocator>
	class TArray final
	{
	public:
		typedef Type ValueType;
		typedef Type* Iterator;
		typedef const Type* ConstIterator;
		typedef std::reverse_iterator<Type*> ReverseIterator;
		typedef std::reverse_iterator<const Type*> ConstReverseIterator;
		typedef ptrdiff_t DifferenceType;

		// For std compatibility
		typedef Type value_type;
		typedef Type* pointer;
		typedef const Type* const_pointer;
		typedef Type& reference;
		typedef const Type& const_reference;
		typedef Type* iterator;
		typedef const Type* const_iterator;
		typedef std::reverse_iterator<Type*> reverse_iterator;
		typedef std::reverse_iterator<const Type*> const_reverse_iterator;
		typedef ptrdiff_t difference_type;

		TArray() = default;

		TArray(const TArray& other)
		{
			if(!other.Empty())
				*this = other;
		}

		TArray(TArray&& other)
		{
			if(!other.Empty())
				*this = std::move(other);
		}

		TArray(u64 size, const Type& value = Type())
		{
			Append(size, value);
		}

		TArray(std::initializer_list<Type> list)
		{
			Append(list);
		}

		template <
			typename IteratorType,
			typename = std::enable_if_t<std::is_convertible_v<typename std::iterator_traits<IteratorType>::iterator_category, std::input_iterator_tag>>>
		TArray(IteratorType start, IteratorType end)
		{
			this->Append(start, end);
		}

		~TArray()
		{
			for(auto& entry : *this)
				entry.~Type();
		}

		TArray& operator=(const TArray& other)
		{
			if(this == &other)
				return *this;

			u64 mySize = Size();
			const u64 otherSize = other.Size();

			// Use assignment copy if we have more elements than the other array, and destroy any excess elements
			if(mySize > otherSize)
			{
				Iterator newEnd;
				if(otherSize > 0)
					newEnd = std::copy(other.Begin(), other.End(), Begin());
				else
					newEnd = Begin();

				for(; newEnd != End(); ++newEnd)
					(*newEnd).~Type();
			}
			// Otherwise we need to partially copy (up to our size), and do uninitialized copy for rest. And an optional
			// grow if our capacity isn't enough (in which case we do uninitialized copy for all).
			else
			{
				if(otherSize > mCapacity)
				{
					Clear();
					mySize = 0;

					ChangeCapacity(otherSize);
				}
				else if(mySize > 0)
					std::copy(other.Begin(), other.Begin() + mySize, Begin());

				std::uninitialized_copy(other.Begin() + mySize, other.End(), Begin() + mySize);
			}

			mSize = otherSize;
			return *this;
		}

		TArray& operator=(TArray&& other)
		{
			if(this == &other)
				return *this;

			u64 mySize = Size();
			const u64 otherSize = other.Size();

			mAllocator.Move(mySize, otherSize, std::move(other.mAllocator));
			mCapacity = std::exchange(other.mCapacity, other.mAllocator.GetMinimumCapacity());
			mSize = std::exchange(other.mSize, 0);

			return *this;
		}

		TArray& operator=(std::initializer_list<Type> list)
		{
			u64 mySize = Size();
			const u64 otherSize = (u64)list.size();

			// Use assignment copy if we have more elements than the list, and destroy any excess elements
			if(mySize > otherSize)
			{
				Iterator newEnd;
				if(otherSize > 0)
					newEnd = std::copy(list.begin(), list.end(), Begin());
				else
					newEnd = Begin();

				for(; newEnd != End(); ++newEnd)
					(*newEnd).~Type();
			}
			// Otherwise we need to partially copy (up to our size), and do uninitialized copy for rest. And an optional
			// grow if our capacity isn't enough (in which case we do uninitialized copy for all).
			else
			{
				if(otherSize > mCapacity)
				{
					Clear();
					mySize = 0;

					ChangeCapacity(otherSize);
				}
				else if(mySize > 0)
					std::copy(list.begin(), list.begin() + mySize, Begin());

				std::uninitialized_copy(list.begin() + mySize, list.end(), Begin() + mySize);
			}

			mSize = otherSize;
			return *this;
		}

		bool operator==(const TArray& other) const
		{
			if(this->Size() != other.Size()) return false;
			return std::equal(this->Begin(), this->End(), other.Begin());
		}

		bool operator!=(const TArray& other) const
		{
			return !(*this == other);
		}

		bool operator<(const TArray& other) const
		{
			return std::lexicographical_compare(Begin(), End(), other.Begin(), other.End());
		}

		bool operator>(const TArray& other) const
		{
			return other < *this;
		}

		bool operator<=(const TArray& other) const
		{
			return !(other < *this);
		}

		bool operator>=(const TArray& other) const
		{
			return !(*this < other);
		}

		Type& operator[](u64 index)
		{
			B3D_ASSERT(index < mSize && "Array index out-of-range.");

			return mAllocator.GetElements()[index];
		}

		const Type& operator[](u64 index) const
		{
			B3D_ASSERT(index < mSize && "Array index out-of-range.");

			return mAllocator.GetElements()[index];
		}

		bool Empty() const { return mSize == 0; }

		Iterator Begin() { return mAllocator.GetElements(); }
		Iterator End() { return mAllocator.GetElements() + mSize; }

		ConstIterator Begin() const { return mAllocator.GetElements(); }
		ConstIterator End() const { return mAllocator.GetElements() + mSize; }

		ConstIterator Cbegin() const { return mAllocator.GetElements(); }
		ConstIterator Cend() const { return mAllocator.GetElements() + mSize; }

		ReverseIterator Rbegin() { return ReverseIterator(End()); }
		ReverseIterator Rend() { return ReverseIterator(Begin()); }

		ConstReverseIterator Rbegin() const { return ConstReverseIterator(End()); }
		ConstReverseIterator Rend() const { return ConstReverseIterator(Begin()); }

		ConstReverseIterator Crbegin() const { return ConstReverseIterator(End()); }
		ConstReverseIterator Crend() const { return ConstReverseIterator(Begin()); }

		u64 Size() const { return mSize; }
		u64 Capacity() const { return mCapacity; }

		Type* Data() { return mAllocator.GetElements(); }
		const Type* Data() const { return mAllocator.GetElements(); }

		Type& Front()
		{
			B3D_ASSERT(!Empty());
			return mAllocator.GetElements()[0];
		}

		Type& Back()
		{
			B3D_ASSERT(!Empty());
			return mAllocator.GetElements()[mSize - 1];
		}

		const Type& Front() const
		{
			B3D_ASSERT(!Empty());
			return mAllocator.GetElements()[0];
		}

		const Type& Back() const
		{
			B3D_ASSERT(!Empty());
			return mAllocator.GetElements()[mSize - 1];
		}

		void Add(const Type& element)
		{
			if(mSize == mCapacity)
				ChangeCapacity(std::max((u64)1u, mCapacity << 1));

			new(&mAllocator.GetElements()[mSize++]) Type(element);
		}

		void Add(Type&& element)
		{
			if(mSize == mCapacity)
				ChangeCapacity(std::max((u64)1u, mCapacity << 1));

			new(&mAllocator.GetElements()[mSize++]) Type(std::move(element));
		}

		template <
			typename IteratorType,
			typename = std::enable_if_t<std::is_convertible_v<typename std::iterator_traits<IteratorType>::iterator_category, std::input_iterator_tag>>>
		void Append(IteratorType start, IteratorType end)
		{
			const u64 count = (u64)std::distance(start, end);

			if((Size() + count) > Capacity())
				this->ChangeCapacity(Size() + count);

			std::uninitialized_copy(start, end, this->End());
			mSize += count;
		}

		void Append(u64 count, const Type& element)
		{
			if((Size() + count) > Capacity())
				this->ChangeCapacity(Size() + count);

			std::uninitialized_fill_n(End(), count, element);
			mSize += count;
		}

		void Append(std::initializer_list<Type> list)
		{
			Append(list.begin(), list.end());
		}

		void Pop()
		{
			B3D_ASSERT(mSize > 0 && "Popping an empty array.");
			mSize--;
			mAllocator.GetElements()[mSize].~Type();
		}

		Iterator Erase(ConstIterator iter)
		{
			B3D_ASSERT(iter >= Begin() && "Iterator to erase is out of bounds.");
			B3D_ASSERT(iter < End() && "Erasing at past-the-end iterator.");

			Iterator toErase = const_cast<Iterator>(iter);
			std::move(toErase + 1, End(), toErase);
			Pop();

			return toErase;
		}

		Iterator Erase(ConstIterator start, ConstIterator end)
		{
			B3D_ASSERT(start >= Begin() && "Iterator to erase is out of bounds.");
			B3D_ASSERT(start <= end && "Trying to erase invalid range.");
			B3D_ASSERT(end <= End() && "Erasing at past-the-end iterator.");

			Iterator mutableStart = const_cast<Iterator>(start);
			Iterator mutableEnd = const_cast<Iterator>(end);

			Iterator current = mutableStart;
			Iterator iter = std::move(mutableEnd, End(), mutableStart);

			while(mutableStart != mutableEnd)
			{
				--mutableEnd;
				mutableEnd->~Type();
			}

			mSize = iter - Begin();
			return current;
		}

		void Remove(u64 index)
		{
			Erase(Begin() + index);
		}

		bool Contains(const Type& element)
		{
			for(u64 elementIndex = 0; elementIndex < mSize; elementIndex++)
			{
				if(mAllocator.GetElements()[elementIndex] == element)
					return true;
			}

			return false;
		}

		void RemoveValue(const Type& element)
		{
			for(u64 elementIndex = 0; elementIndex < mSize; elementIndex++)
			{
				if(mAllocator.GetElements()[elementIndex] == element)
				{
					Remove(elementIndex);
					break;
				}
			}
		}

		void Clear()
		{
			for(u64 elementIndex = 0; elementIndex < mSize; ++elementIndex)
				mAllocator.GetElements()[elementIndex].~Type();

			mSize = 0;
		}

		void Reserve(u64 capacity)
		{
			if(capacity > mCapacity)
				ChangeCapacity(capacity);
		}

		void Resize(u64 size, const Type& value = Type())
		{
			if(size > mCapacity)
				ChangeCapacity(size);

			if(size > mSize)
			{
				for(u64 elementIndex = mSize; elementIndex < size; elementIndex++)
					new(&mAllocator.GetElements()[elementIndex]) Type(value);
			}
			else
			{
				for(u64 elementIndex = size; elementIndex < mSize; elementIndex++)
					mAllocator.GetElements()[elementIndex].~Type();
			}

			mSize = size;
		}

		void Shrink()
		{
			if(mSize != mCapacity)
				ChangeCapacity(mSize);
		}

		bool SwapAndErase(Iterator iter)
		{
			B3D_ASSERT(!Empty());

			auto iterLast = end() - 1;

			bool swapped = false;
			if(iter != iterLast)
			{
				std::swap(*iter, *iterLast);
				swapped = true;
			}

			Pop();
			return swapped;
		}

		template <typename... Args>
		void EmplaceBack(Args&&... args)
		{
			if(mSize == mCapacity)
				ChangeCapacity(std::max((u64)1u, mCapacity << 1));

			new(&mAllocator.GetElements()[mSize++]) Type(std::forward<Args>(args)...);
		}

		template <typename... Args>
		Iterator Emplace(ConstIterator it, Args&&... args)
		{
			Iterator mutableIterator = const_cast<Iterator>(it);
			DifferenceType offset = mutableIterator - Begin();

			if(mSize == mCapacity)
				ChangeCapacity(std::max((u64)1u, mCapacity << 1));

			new(&mAllocator.GetElements()[mSize++]) Type(std::forward<Args>(args)...);
			std::rotate(Begin() + offset, End() - 1, End());

			return Begin() + offset;
		}

		Iterator Insert(ConstIterator it, const Type& element)
		{
			Iterator mutableIterator = const_cast<Iterator>(it);
			DifferenceType offset = mutableIterator - Begin();

			if(mSize == mCapacity)
				ChangeCapacity(std::max((u64)1u, mCapacity << 1));

			new(&mAllocator.GetElements()[mSize++]) Type(element);
			std::rotate(Begin() + offset, End() - 1, End());

			return Begin() + offset;
		}

		Iterator Insert(ConstIterator it, Type&& element)
		{
			Iterator mutableIterator = const_cast<Iterator>(it);
			DifferenceType offset = mutableIterator - Begin();

			if(mSize == mCapacity)
				ChangeCapacity(std::max((u64)1u, mCapacity << 1));

			new(&mAllocator.GetElements()[mSize++]) Type(std::move(element));
			std::rotate(Begin() + offset, End() - 1, End());

			return Begin() + offset;
		}

		Iterator Insert(ConstIterator it, u32 count, const Type& element)
		{
			Iterator mutableIterator = const_cast<Iterator>(it);
			DifferenceType offset = mutableIterator - Begin();

			if(!count)
				return Begin() + offset;

			if(Size() + count > Capacity())
				ChangeCapacity((Size() + count) * 2);

			u32 remainingCount = count;
			while(remainingCount--)
				new(&mAllocator.GetElements()[mSize++]) Type(element);

			std::rotate(Begin() + offset, End() - count, End());
			return Begin() + offset;
		}

		template <typename InputIt>
		typename std::enable_if_t<!std::is_integral_v<InputIt>, void> Insert(ConstIterator it, InputIt first, InputIt last)
		{
			Iterator mutableIt = const_cast<Iterator>(it);
			DifferenceType offset = mutableIt - Begin();
			const u64 elementCountToInsert = (u64)(last - first);

			if(Size() + elementCountToInsert > Capacity())
				ChangeCapacity((Size() + elementCountToInsert) * 2);

			while(first != last)
				new(&mAllocator.GetElements()[mSize++]) Type(*first++);

			std::rotate(Begin() + offset, End() - elementCountToInsert, End());
		}

		Iterator Insert(ConstIterator it, std::initializer_list<Type> list)
		{
			Iterator mutableIt = const_cast<Iterator>(it);
			DifferenceType offset = mutableIt - Begin();
			const u64 elementCountToInsert = (u64)list.size();

			if(elementCountToInsert == 0)
				return Begin() + offset;

			if(Size() + elementCountToInsert > Capacity())
				ChangeCapacity((Size() + elementCountToInsert) * 2);

			for(auto& entry : list)
				new(&mAllocator.GetElements()[mSize++]) Type(entry);

			std::rotate(Begin() + offset, end() - elementCountToInsert, end());
			return Begin() + offset;
		}

		// STD compatible API
		Iterator begin() { return Begin(); } // NOLINT
		Iterator end() { return End(); } // NOLINT

		ConstIterator begin() const { return Begin(); } // NOLINT
		ConstIterator end() const { return End(); } // NOLINT

		ConstIterator cbegin() const { return Cbegin(); } // NOLINT
		ConstIterator cend() const { return Cend(); } // NOLINT

		ReverseIterator rbegin() { return Rbegin(); } // NOLINT
		ReverseIterator rend() { return Rend(); } // NOLINT

		ConstReverseIterator rbegin() const { return Rbegin(); } // NOLINT
		ConstReverseIterator rend() const { return Rend(); } // NOLINT

		ConstReverseIterator crbegin() const { return Crbegin(); } // NOLINT
		ConstReverseIterator crend() const { return Crend(); } // NOLINT

		u64 size() const { return Size(); } // NOLINT
		u64 capacity() const { return Capacity(); } // NOLINT

		Type* data() { return Data(); } // NOLINT
		const Type* data() const { return Data(); } // NOLINT

		Type& front() { return Front(); } // NOLINT
		const Type& front() const { return Front(); } // NOLINT

		Type& back() { return Back(); } // NOLINT
		const Type& back() const { return Back(); } // NOLINT

		Iterator insert(ConstIterator it, const Type& element) { return Insert(it, element); } // NOLINT

		Iterator erase(ConstIterator iter) { return Erase(iter); } // NOLINT
		Iterator erase(ConstIterator start, ConstIterator end) { return Erase(start, end); } // NOLINT

		void clear() { Clear(); } // NOLINT
		void reserve(u64 capacity) { Reserve(capacity); } // NOLINT
		void resize(u64 size, const Type& value = Type()) { Resize(size, value); } // NOLINT

	private:
		void ChangeCapacity(u64 capacity)
		{
			mAllocator.Resize(mSize, capacity);
			mCapacity = std::max(capacity, mAllocator.GetMinimumCapacity());
		}

		typename Allocator::template ForElementType<Type> mAllocator;
		u64 mSize = 0;
		u64 mCapacity = mAllocator.GetMinimumCapacity();
	};

	/**
	 * Dynamically sized container that statically allocates enough room for @p N elements of type @p Type. If the element
	 * count exceeds the statically allocated buffer size the vector falls back to general purpose dynamic allocator.
	 */
	template<class T, u32 N>
	using TInlineArray = TArray<T, InlineContainerAllocator<N>>;

	/** @} */
} // namespace b3d
