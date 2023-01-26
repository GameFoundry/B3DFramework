//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Dynamically sized container that statically allocates enough room for @p N elements of type @p Type. If the element
	 * count exceeds the statically allocated buffer size the vector falls back to general purpose dynamic allocator.
	 */
	template <class Type, u32 N>
	class SmallVector final
	{
	public:
		typedef Type ValueType;
		typedef Type* Iterator;
		typedef const Type* ConstIterator;
		typedef std::reverse_iterator<Type*> ReverseIterator;
		typedef std::reverse_iterator<const Type*> ConstReverseIterator;

		// For std compatibility
		typedef Type value_type;
		typedef Type* iterator;
		typedef const Type* const_iterator;
		typedef std::reverse_iterator<Type*> reverse_iterator;
		typedef std::reverse_iterator<const Type*> const_reverse_iterator;

		SmallVector() = default;

		SmallVector(const SmallVector<ValueType, N>& other)
		{
			if(!other.Empty())
				*this = other;
		}

		SmallVector(SmallVector<ValueType, N>&& other)
		{
			if(!other.Empty())
				*this = std::move(other);
		}

		SmallVector(u32 size, const Type& value = Type())
		{
			Append(size, value);
		}

		SmallVector(std::initializer_list<Type> list)
		{
			Append(list);
		}

		~SmallVector()
		{
			for(auto& entry : *this)
				entry.~Type();

			if(!IsSmall())
				B3DFree(mElements);
		}

		SmallVector<ValueType, N>& operator=(const SmallVector<ValueType, N>& other)
		{
			if(this == &other)
				return *this;

			u32 mySize = Size();
			const u32 otherSize = other.Size();

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

					Grow(otherSize);
				}
				else if(mySize > 0)
					std::copy(other.Begin(), other.Begin() + mySize, Begin());

				std::uninitialized_copy(other.Begin() + mySize, other.End(), Begin() + mySize);
			}

			mSize = otherSize;
			return *this;
		}

		SmallVector<ValueType, N>& operator=(SmallVector<ValueType, N>&& other)
		{
			if(this == &other)
				return *this;

			// If the other buffer isn't small, we can just steal its buffer
			if(!other.IsSmall())
			{
				for(auto& entry : *this)
					entry.~Type();

				if(!IsSmall())
					B3DFree(mElements);

				mElements = other.mElements;
				other.mElements = (Type*)other.mStorage;
				mSize = std::exchange(other.mSize, 0);
				mCapacity = std::exchange(other.mCapacity, N);
			}
			// Otherwise we do essentially the same thing as in non-move assigment, except for also clearing the other
			// vector
			else
			{
				u32 mySize = Size();
				const u32 otherSize = other.Size();

				// Use assignment copy if we have more elements than the other array, and destroy any excess elements
				if(mySize > otherSize)
				{
					Iterator newEnd;
					if(otherSize > 0)
						newEnd = std::move(other.Begin(), other.End(), Begin());
					else
						newEnd = Begin();

					for(; newEnd != End(); ++newEnd)
						(*newEnd).~Type();
				}
				else
				{
					if(otherSize > mCapacity)
					{
						Clear();
						mySize = 0;

						Grow(otherSize);
					}
					else if(mySize > 0)
						std::move(other.Begin(), other.Begin() + mySize, Begin());

					std::uninitialized_copy(
						std::make_move_iterator(other.Begin() + mySize),
						std::make_move_iterator(other.End()),
						Begin() + mySize);
				}

				mSize = otherSize;
				other.Clear();
			}

			return *this;
		}

		SmallVector<ValueType, N>& operator=(std::initializer_list<Type> list)
		{
			u32 mySize = Size();
			const u32 otherSize = (u32)list.size();

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

					Grow(otherSize);
				}
				else if(mySize > 0)
					std::copy(list.begin(), list.begin() + mySize, Begin());

				std::uninitialized_copy(list.begin() + mySize, list.end(), Begin() + mySize);
			}

			mSize = otherSize;
			return *this;
			;
		}

		bool operator==(const SmallVector<ValueType, N>& other)
		{
			if(this->Size() != other.Size()) return false;
			return std::equal(this->Begin(), this->End(), other.Begin());
		}

		bool operator!=(const SmallVector<ValueType, N>& other)
		{
			return !(*this == other);
		}

		bool operator<(const SmallVector<ValueType, N>& other) const
		{
			return std::lexicographical_compare(Begin(), End(), other.Begin(), other.End());
		}

		bool operator>(const SmallVector<ValueType, N>& other) const
		{
			return other < *this;
		}

		bool operator<=(const SmallVector<ValueType, N>& other) const
		{
			return !(other < *this);
		}

		bool operator>=(const SmallVector<ValueType, N>& other) const
		{
			return !(*this < other);
		}

		Type& operator[](u32 index)
		{
			B3D_ASSERT(index < mSize && "Array index out-of-range.");

			return mElements[index];
		}

		const Type& operator[](u32 index) const
		{
			B3D_ASSERT(index < mSize && "Array index out-of-range.");

			return mElements[index];
		}

		bool Empty() const { return mSize == 0; }

		Iterator Begin() { return mElements; }

		Iterator End() { return mElements + mSize; }

		ConstIterator Begin() const { return mElements; }

		ConstIterator End() const { return mElements + mSize; }

		ConstIterator Cbegin() const { return mElements; }

		ConstIterator Cend() const { return mElements + mSize; }

		ReverseIterator Rbegin() { return ReverseIterator(End()); }

		ReverseIterator Rend() { return ReverseIterator(Begin()); }

		ConstReverseIterator Rbegin() const { return ConstReverseIterator(End()); }

		ConstReverseIterator Rend() const { return ConstReverseIterator(Begin()); }

		ConstReverseIterator Crbegin() const { return ConstReverseIterator(End()); }

		ConstReverseIterator Crend() const { return ConstReverseIterator(Begin()); }

		u32 Size() const { return mSize; }

		u32 Capacity() const { return mCapacity; }

		Type* Data() { return mElements; }

		const Type* Data() const { return mElements; }

		Type& Front()
		{
			B3D_ASSERT(!Empty());
			return mElements[0];
		}

		Type& Back()
		{
			B3D_ASSERT(!Empty());
			return mElements[mSize - 1];
		}

		const Type& Front() const
		{
			B3D_ASSERT(!Empty());
			return mElements[0];
		}

		const Type& Back() const
		{
			B3D_ASSERT(!Empty());
			return mElements[mSize - 1];
		}

		void Add(const Type& element)
		{
			if(mSize == mCapacity)
				Grow(mCapacity << 1);

			new(&mElements[mSize++]) Type(element);
		}

		void Add(Type&& element)
		{
			if(mSize == mCapacity)
				Grow(mCapacity << 1);

			new(&mElements[mSize++]) Type(std::move(element));
		}

		void Append(ConstIterator start, ConstIterator end)
		{
			const u32 count = (u32)std::distance(start, end);

			if((Size() + count) > Capacity())
				this->Grow(Size() + count);

			std::uninitialized_copy(start, end, this->End());
			mSize += count;
		}

		void Append(u32 count, const Type& element)
		{
			if((Size() + count) > Capacity())
				this->Grow(Size() + count);

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
			mElements[mSize].~Type();
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

		void Remove(u32 index)
		{
			Erase(Begin() + index);
		}

		bool Contains(const Type& element)
		{
			for(u32 i = 0; i < mSize; i++)
			{
				if(mElements[i] == element)
					return true;
			}

			return false;
		}

		void RemoveValue(const Type& element)
		{
			for(u32 i = 0; i < mSize; i++)
			{
				if(mElements[i] == element)
				{
					Remove(i);
					break;
				}
			}
		}

		void Clear()
		{
			for(u32 i = 0; i < mSize; ++i)
				mElements[i].~Type();

			mSize = 0;
		}

		void Reserve(u32 capacity)
		{
			if(capacity > mCapacity)
				Grow(capacity);
		}

		void Resize(u32 size, const Type& value = Type())
		{
			if(size > mCapacity)
				Grow(size);

			if(size > mSize)
			{
				for(u32 i = mSize; i < size; i++)
					new(&mElements[i]) Type(value);
			}
			else
			{
				for(u32 i = size; i < mSize; i++)
					mElements[i].~Type();
			}

			mSize = size;
		}

		// STD compatible API
		Iterator begin() { return Begin(); } // NOLINT

		Iterator end() { return End(); } // NOLINT

		ConstIterator begin() const { return Begin(); } // NOLINT

		ConstIterator end() const { return End(); } // NOLINT

		ConstIterator cbegin() const { return Cbegin(); } // NOLINT

		ConstIterator cend() const { return cend(); } // NOLINT

		ReverseIterator rbegin() { return Rbegin(); } // NOLINT

		ReverseIterator rend() { return Rend(); } // NOLINT

		ConstReverseIterator rbegin() const { return Rbegin(); } // NOLINT

		ConstReverseIterator rend() const { return Rend(); } // NOLINT

		ConstReverseIterator crbegin() const { return Crbegin(); } // NOLINT

		ConstReverseIterator crend() const { return Crend(); } // NOLINT

		u32 size() const { return Size(); } // NOLINT

		u32 capacity() const { return Capacity(); } // NOLINT

		Type* data() { return Data(); } // NOLINT

		const Type* data() const { return Data(); } // NOLINT

		Type& front() { return Front(); } // NOLINT

		const Type& front() const { return Front(); } // NOLINT

		Type& back() { return Back(); } // NOLINT

		const Type& back() const { return Back(); } // NOLINT

		Iterator erase(ConstIterator iter) { return Erase(iter); } // NOLINT

		void clear() { Clear(); } // NOLINT

		void reserve(u32 capacity) { Reserve(capacity); } // NOLINT

		void resize(u32 size, const Type& value = Type()) { Resize(size, value); } // NOLINT

	private:
		/** Returns true if the vector is still using its static memory and hasn't made any dynamic allocations. */
		bool IsSmall() const { return mElements == (Type*)mStorage; }

		void Grow(u32 capacity)
		{
			B3D_ASSERT(capacity > N);

			// Allocate memory with the new capacity (caller guarantees never to call this with capacity <= N, so we don't
			// need to worry about the static buffer)
			Type* buffer = B3DAllocateMultiple<Type>(capacity);

			// Move any existing elements
			std::uninitialized_copy(
				std::make_move_iterator(Begin()),
				std::make_move_iterator(End()),
				buffer);

			// Destoy existing elements in old memory
			for(auto& entry : *this)
				entry.~Type();

			// If the current buffer is dynamically allocated, free it
			if(!IsSmall())
				B3DFree(mElements);

			mElements = buffer;
			mCapacity = capacity;
		}

		std::aligned_storage_t<sizeof(Type), alignof(Type)> mStorage[N];
		Type* mElements = (Type*)mStorage;

		u32 mSize = 0;
		u32 mCapacity = N;
	};

	/** @} */
} // namespace bs
