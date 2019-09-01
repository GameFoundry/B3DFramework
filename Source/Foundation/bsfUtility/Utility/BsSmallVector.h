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
	template <class Type, UINT32 N>
	class SmallVector final
	{
	public:
		typedef Type ValueType;
		typedef Type* Iterator;
		typedef const Type* ConstIterator;
		typedef std::reverse_iterator<Type*> ReverseIterator;
		typedef std::reverse_iterator<const Type*> ConstReverseIterator;

		SmallVector() = default;
		SmallVector(const SmallVector<ValueType, N>& other)
		{
			if(!other.empty())
				*this = other;
		}

		SmallVector(SmallVector<ValueType, N>&& other)
		{
			if(!other.empty())
				*this = std::move(other);
		}

		SmallVector(UINT32 size, const Type& value = Type())
		{
			append(size, value);
		}

		SmallVector(std::initializer_list<Type> list)
		{
			append(list);
		}

		~SmallVector()
		{
			for (auto& entry : *this)
				entry.~Type();

			if(!isSmall())
				bs_free(mElements);
		}

		SmallVector<ValueType, N>& operator= (const SmallVector<ValueType, N>& other)
		{
			if(this == &other)
				return *this;

			UINT32 mySize = size();
			const UINT32 otherSize = other.size();

			// Use assignment copy if we have more elements than the other array, and destroy any excess elements
			if(mySize > otherSize)
			{
				Iterator newEnd;
				if(otherSize > 0)
					newEnd = std::copy(other.begin(), other.end(), begin());
				else
					newEnd = begin();

				for(;newEnd != end(); ++newEnd)
					(*newEnd).~Type();

			}
			// Otherwise we need to partially copy (up to our size), and do uninitialized copy for rest. And an optional
			// grow if our capacity isn't enough (in which case we do uninitialized copy for all).
			else
			{
				if (otherSize > mCapacity)
				{
					clear();
					mySize = 0;

					grow(otherSize);
				}
				else if (mySize > 0)
					std::copy(other.begin(), other.begin() + mySize, begin());

				std::uninitialized_copy(other.begin() + mySize, other.end(), begin() + mySize);
			}

			mSize = otherSize;
			return *this;
		}

		SmallVector<ValueType, N>& operator= (SmallVector<ValueType, N>&& other)
		{
			if(this == &other)
				return *this;

			// If the other buffer isn't small, we can just steal its buffer
			if(!other.isSmall())
			{
				for(auto& entry : *this)
					entry.~Type();

				if(!isSmall())
					bs_free(mElements);

				mElements = other.mElements;
				other.mElements = (Type*)other.mStorage;
				mSize = std::exchange(other.mSize, 0);
				mCapacity = std::exchange(other.mCapacity, N);
			}
			// Otherwise we do essentially the same thing as in non-move assigment, except for also clearing the other
			// vector
			else
			{
				UINT32 mySize = size();
				const UINT32 otherSize = other.size();

				// Use assignment copy if we have more elements than the other array, and destroy any excess elements
				if(mySize > otherSize)
				{
					Iterator newEnd;
					if(otherSize > 0)
						newEnd = std::move(other.begin(), other.end(), begin());
					else
						newEnd = begin();

					for(;newEnd != end(); ++newEnd)
						(*newEnd).~Type();
				}	
				else
				{
					if (otherSize > mCapacity)
					{
						clear();
						mySize = 0;

						grow(otherSize);
					}
					else if (mySize > 0)
						std::move(other.begin(), other.begin() + mySize, begin());

					std::uninitialized_copy(
						std::make_move_iterator(other.begin() + mySize),
						std::make_move_iterator(other.end()), 
						begin() + mySize);
				}

				mSize = otherSize;
				other.clear();
			}

			return *this;
		}

		SmallVector<ValueType, N>& operator=(std::initializer_list<Type> list)
		{
			UINT32 mySize = size();
			const UINT32 otherSize = (UINT32)list.size();

			// Use assignment copy if we have more elements than the list, and destroy any excess elements
			if(mySize > otherSize)
			{
				Iterator newEnd;
				if(otherSize > 0)
					newEnd = std::copy(list.begin(), list.end(), begin());
				else
					newEnd = begin();

				for(;newEnd != end(); ++newEnd)
					(*newEnd).~Type();

			}
			// Otherwise we need to partially copy (up to our size), and do uninitialized copy for rest. And an optional
			// grow if our capacity isn't enough (in which case we do uninitialized copy for all).
			else
			{
				if (otherSize > mCapacity)
				{
					clear();
					mySize = 0;

					grow(otherSize);
				}
				else if (mySize > 0)
					std::copy(list.begin(), list.begin() + mySize, begin());

				std::uninitialized_copy(list.begin() + mySize, list.end(), begin() + mySize);
			}

			mSize = otherSize;
			return *this;;
		}

		bool operator== (const SmallVector<ValueType, N>& other)
		{
			if (this->size() != other.size()) return false;
			return std::equal(this->begin(), this->end(), other.begin());
		}

		bool operator!= (const SmallVector<ValueType, N>& other)
		{
			return !(*this == other);
		}

		bool operator< (const SmallVector<ValueType, N>& other) const
		{
			return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
		}

		bool operator> (const SmallVector<ValueType, N>& other) const
		{
			return other < *this;
		}

		bool operator<= (const SmallVector<ValueType, N>& other) const
		{
			return !(other < *this);
		}

		bool operator>= (const SmallVector<ValueType, N>& other) const
		{
			return !(*this < other);
		}

		Type& operator[] (UINT32 index)
		{
			assert(index < mSize && "Array index out-of-range.");

			return mElements[index];
		}

		const Type& operator[] (UINT32 index) const
		{
			assert(index < mSize && "Array index out-of-range.");

			return mElements[index];
		}

		bool empty() const { return mSize == 0; }

		Iterator begin() { return mElements; }
		Iterator end() { return mElements + mSize; }

		ConstIterator begin() const { return mElements; }
		ConstIterator end() const { return mElements + mSize; }

		ConstIterator cbegin() const { return mElements; }
		ConstIterator cend() const { return mElements + mSize; }

		ReverseIterator rbegin() { return ReverseIterator(end()); }
		ReverseIterator rend() { return ReverseIterator(begin()); }

		ConstReverseIterator rbegin() const { return ConstReverseIterator(end()); }
		ConstReverseIterator rend() const { return ConstReverseIterator(begin()); }

		ConstReverseIterator crbegin() const { return ConstReverseIterator(end()); }
		ConstReverseIterator crend() const { return ConstReverseIterator(begin()); }

		UINT32 size() const { return mSize; }
		UINT32 capacity() const { return mCapacity; }

		Type* data() { return mElements; }
		const Type* data() const { return mElements; }

		Type& front()
		{
			assert(!empty());
			return mElements[0];
		}

		Type& back()
		{
			assert(!empty());
			return mElements[mSize - 1];
		}

		const Type& front() const
		{
			assert(!empty());
			return mElements[0];
		}

		const Type& back() const
		{
			assert(!empty());
			return mElements[mSize - 1];
		}

		void add(const Type& element)
		{
			if (mSize == mCapacity)
				grow(mCapacity << 1);

			new (&mElements[mSize++]) Type(element);
		}

		void add(Type&& element)
		{
			if (mSize == mCapacity)
				grow(mCapacity << 1);

			new (&mElements[mSize++]) Type(std::move(element));
		}

		void append(ConstIterator start, ConstIterator end) 
		{
			const UINT32 count = (UINT32)std::distance(start, end);

			if ((size() + count) > capacity())
				this->grow(size() + count);

			std::uninitialized_copy(start, end, this->end());
			mSize += count;
		}

		void append(UINT32 count, const Type& element) 
		{
			if ((size() + count) > capacity())
				this->grow(size() + count);

			std::uninitialized_fill_n(end(), count, element);
			mSize += count;
		}

		void append(std::initializer_list<Type> list) 
		{
			append(list.begin(), list.end());
		}

		void pop()
		{
			assert(mSize > 0 && "Popping an empty array.");
			mSize--;
			mElements[mSize].~Type();
		}

		Iterator erase(ConstIterator iter)
		{
			assert(iter >= begin() && "Iterator to erase is out of bounds.");
			assert(iter < end() && "Erasing at past-the-end iterator.");

			Iterator toErase = const_cast<Iterator>(iter);
			std::move(toErase + 1, end(), toErase);
			pop();

			return toErase;
		}

		void remove(UINT32 index)
		{
			erase(begin() + index);
		}

		bool contains(const Type& element)
		{
			for (UINT32 i = 0; i < mSize; i++)
			{
				if (mElements[i] == element)
					return true;
			}

			return false;
		}

		void removeValue(const Type& element)
		{
			for (UINT32 i = 0; i < mSize; i++)
			{
				if (mElements[i] == element)
				{
					remove(i);
					break;
				}
			}
		}

		void clear()
		{
			for (UINT32 i = 0; i < mSize; ++i)
				mElements[i].~Type();

			mSize = 0;
		}

		void reserve(UINT32 capacity)
		{
			if (capacity > mCapacity)
				grow(capacity);
		}

		void resize(UINT32 size, const Type& value = Type())
		{
			if(size > mCapacity)
				grow(size);

			if(size > mSize)
			{
				for(UINT32 i = mSize; i < size; i++)
					new (&mElements[i]) Type(value);
			}
			else
			{
				for(UINT32 i = size; i < mSize; i++)
					mElements[i].~Type();
			}

			mSize = size;
		}

	private:
		/** Returns true if the vector is still using its static memory and hasn't made any dynamic allocations. */
		bool isSmall() const { return mElements == (Type*)mStorage; }

		void grow(UINT32 capacity)
		{
			assert(capacity > N);

			// Allocate memory with the new capacity (caller guarantees never to call this with capacity <= N, so we don't
			// need to worry about the static buffer)
			Type* buffer = bs_allocN<Type>(capacity);

			// Move any existing elements
			std::uninitialized_copy(
				std::make_move_iterator(begin()), 
				std::make_move_iterator(end()),
				buffer);

			// Destoy existing elements in old memory
			for (auto& entry : *this)
				entry.~Type();

			// If the current buffer is dynamically allocated, free it
			if(!isSmall())
				bs_free(mElements);

			mElements = buffer;
			mCapacity = capacity;
		}

		std::aligned_storage_t<sizeof(Type), alignof(Type)> mStorage[N];
		Type* mElements = (Type*)mStorage;

		UINT32 mSize = 0;
		UINT32 mCapacity = N;
	};

	/** @} */
}
