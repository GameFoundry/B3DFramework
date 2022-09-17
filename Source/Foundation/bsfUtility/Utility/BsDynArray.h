//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Dynamically sized array, similar to std::vector. */
	template <class Type>
	class DynArray final
	{
	public:
		typedef Type ValueType;
		typedef Type* Iterator;
		typedef const Type* ConstIterator;
		typedef std::reverse_iterator<Type*> ReverseIterator;
		typedef std::reverse_iterator<const Type*> ConstReverseIterator;
		typedef ptrdiff_t DifferenceType;

		DynArray() = default;
		DynArray(UINT32 size, const ValueType& value = ValueType())
		{
			append(size, value);
		}

		DynArray(Iterator first, Iterator last)
		{
			append(first, last);
		}

		DynArray(std::initializer_list<ValueType> list)
		{
			append(list);
		}

		DynArray(const DynArray<ValueType>& other)
		{
			if (!other.Empty())
				*this = other;
		}

		DynArray(DynArray<ValueType>&& other)
		{
			if (!other.Empty())
				*this = std::move(other);
		}

		~DynArray()
		{
			for (auto& entry : *this)
				entry.~Type();

			bs_free(mElements);
		}

		DynArray<ValueType>& operator= (const DynArray<ValueType>& other)
		{
			if (this == &other)
				return *this;

			UINT32 mySize = Size();
			const UINT32 otherSize = other.Size();

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
					Clear();
					mySize = 0;

					Realloc(otherSize);
				}
				else if (mySize > 0)
					std::copy(other.begin(), other.begin() + mySize, begin());

				std::uninitialized_copy(other.begin() + mySize, other.end(), begin() + mySize);
			}

			mSize = otherSize;
			return *this;
		}

		DynArray<ValueType>& operator= (DynArray<ValueType>&& other)
		{
			if(this == &other)
				return *this;

			// Just steal the buffer
			for (auto& entry : *this)
				entry.~Type();

			bs_free(mElements);

			mElements = std::exchange(other.mElements, nullptr);
			mSize = std::exchange(other.mSize, 0);
			mCapacity = std::exchange(other.mCapacity, 0);

			return *this;
		}

		DynArray<ValueType>& operator= (std::initializer_list<ValueType> list)
		{
			UINT32 mySize = Size();
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
					Clear();
					mySize = 0;

					Realloc(otherSize);
				}
				else if (mySize > 0)
					std::copy(list.begin(), list.begin() + mySize, begin());

				std::uninitialized_copy(list.begin() + mySize, list.end(), begin() + mySize);
			}

			mSize = otherSize;
			return *this;
		}

		bool operator== (const DynArray<ValueType>& other) const
		{
			if (this->Size() != other.Size()) return false;
			return std::equal(this->begin(), this->end(), other.begin());
		}

		bool operator!= (const DynArray<ValueType>& other) const
		{
			return !(*this == other);
		}

		bool operator< (const DynArray<ValueType>& other) const
		{
			return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
		}

		bool operator> (const DynArray<ValueType>& other) const
		{
			return other < *this;
		}

		bool operator<= (const DynArray<ValueType>& other) const
		{
			return !(other < *this);
		}

		bool operator>= (const DynArray<ValueType>& other) const
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

		bool Empty() const { return mSize == 0; }

		Iterator Begin() { return mElements; }
		Iterator End() { return mElements + mSize; }

		ConstIterator Begin() const { return mElements; }
		ConstIterator End() const { return mElements + mSize; }

		ConstIterator Cbegin() const { return mElements; }
		ConstIterator Cend() const { return mElements + mSize; }

		ReverseIterator Rbegin() { return ReverseIterator(end()); }
		ReverseIterator Rend() { return ReverseIterator(begin()); }

		ConstReverseIterator Rbegin() const { return ReverseIterator(end()); }
		ConstReverseIterator Rend() const { return ReverseIterator(begin()); }

		ConstReverseIterator Crbegin() const { return ReverseIterator(end()); }
		ConstReverseIterator Crend() const { return ReverseIterator(begin()); }

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

		UINT32 Size() const { return mSize; }
		UINT32 Capacity() const { return mCapacity; }

		Type* Data() { return mElements; }
		const Type* Data() const { return mElements; }

		Type& Front()
		{
			assert(!Empty());
			return *mElements[0];
		}

		Type& Back()
		{
			assert(!Empty());
			return *mElements[mSize - 1];
		}

		const Type& Front() const
		{
			assert(!Empty());
			return mElements[0];
		}

		const Type& Back() const
		{
			assert(!Empty());
			return mElements[mSize - 1];
		}

		void Add(const Type& element)
		{
			if (Size() == Capacity())
				Realloc(std::max(1U, Capacity() * 2));

			new (&mElements[mSize++]) Type(element);
		}

		void Add(Type&& element)
		{
			if (Size() == Capacity())
				Realloc(std::max(1U, Capacity() * 2));

			new (&mElements[mSize++]) Type(std::move(element));
		}

		void Pop()
		{
			assert(mSize > 0 && "Popping an empty array.");
			--mSize;
			mElements[mSize].~Type();
		}

		void Remove(UINT32 index)
		{
			erase(begin() + index);
		}

		bool Contains(const Type& element)
		{
			for (UINT32 i = 0; i < mSize; i++)
			{
				if (mElements[i] == element)
					return true;
			}

			return false;
		}

		void RemoveValue(const Type& element)
		{
			for (UINT32 i = 0; i < mSize; i++)
			{
				if (mElements[i] == element)
				{
					Remove(i);
					break;
				}
			}
		}

		void Clear()
		{
			for (UINT32 i = 0; i < mSize; ++i)
				mElements[i].~Type();

			mSize = 0;
		}

		void Resize(UINT32 size, const Type& value = Type())
		{
			if (size > Capacity())
				Realloc(size);

			if (size > mSize)
			{
				for (UINT32 i = mSize; i < size; i++)
					new (&mElements[i]) Type(value);
			}
			else
			{
				for (UINT32 i = size; i < mSize; i++)
					mElements[i].~Type();
			}

			mSize = size;
		}

		void Reserve(UINT32 size)
		{
			if (size > Capacity())
				Realloc(size);
		}

		void Shrink()
		{
			Realloc(mSize);
		}

		void Append(ConstIterator start, ConstIterator end)
		{
			const UINT32 count = (UINT32)std::distance(start, end);

			if ((Size() + count) > Capacity())
				Realloc(Size() + count);

			std::uninitialized_copy(start, end, this->end());
			mSize += count;
		}

		void Append(UINT32 count, const Type& element)
		{
			if ((Size() + count) > Capacity())
				Realloc(Size() + count);

			std::uninitialized_fill_n(end(), count, element);
			mSize += count;
		}

		void Append(std::initializer_list<Type> list)
		{
			append(list.begin(), list.end());
		}

		void Swap(DynArray<ValueType>& other)
		{
			const UINT32 tmpSize = Size();
			const UINT32 tmpCapacity = Capacity();
			Type* tmp = Data();

			mSize = other.Size();
			mCapacity = other.Capacity();
			mElements = other.Data();

			other.mSize = tmpSize;
			other.mCapacity = tmpCapacity;
			other.mElements = tmp;
		}

		bool SwapAndErase(Iterator iter)
		{
			assert(!Empty());

			auto iterLast = end() - 1;

			bool swapped = false;
			if (iter != iterLast)
			{
				std::swap(*iter, *iterLast);
				swapped = true;
			}

			Pop();
			return swapped;
		}

		template <typename ...Args>
		void EmplaceBack(Args&& ...args)
		{
			if (Size() == Capacity())
				Realloc(std::max(1U, Capacity() * 2));

			new (&mElements[mSize++]) Type(std::forward<Args>(args) ...);
		}

		template <typename ...Args>
		Iterator Emplace(ConstIterator it, Args&&... args)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();

			if (Size() == Capacity())
				Realloc(std::max(1U, Capacity() * 2));

			new (&mElements[mSize++]) Type(std::forward<Args>(args) ...);
			std::rotate(begin() + offset, end() - 1, end());

			return begin() + offset;
		}

		Iterator Insert(ConstIterator it, const ValueType& element)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();

			if (Size() == Capacity())
				Realloc(std::max(1U, Capacity() * 2));

			new (&mElements[mSize++]) Type(element);
			std::rotate(begin() + offset, end() - 1, end());

			return begin() + offset;
		}

		Iterator Insert(ConstIterator it, ValueType&& element)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();

			if (Size() == Capacity())
				Realloc(std::max(1U, Capacity() * 2));

			new (&mElements[mSize++]) Type(std::move(element));
			std::rotate(begin() + offset, end() - 1, end());

			return begin() + offset;
		}

		Iterator Insert(ConstIterator it, UINT32 n, const ValueType& element)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();
			Iterator iter = &mElements[offset];

			if (!n)
				return iter;

			if (Size() + n > Capacity())
				Realloc((Size() + n) * 2);

			UINT32 c = n;
			while (c--)
				new (&mElements[mSize++]) Type(element);

			std::rotate(begin() + offset, end() - n, end());

			return begin() + offset;
		}

		template <typename InputIt>
		typename std::enable_if<!std::is_integral<InputIt>::value, void>::type Insert(ConstIterator it,
			InputIt first, InputIt last)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();
			UINT32 n = (UINT32)(last - first);

			if (Size() + n > Capacity())
				Realloc((Size() + n) * 2);

			while (first != last)
				new (&mElements[mSize++]) Type(*first++);

			std::rotate(begin() + offset, end() - n, end());
		}

		Iterator Insert(ConstIterator it, std::initializer_list<ValueType> list)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();
			Iterator iter = &mElements[offset];
			UINT32 n = (UINT32)list.size();

			if (!n)
				return iter;

			if (Size() + n > Capacity())
				Realloc((Size() + n) * 2);

			for (auto& entry : list)
				new (&mElements[mSize++]) Type(entry);

			std::rotate(begin() + offset, end() - n, end());

			return iter;
		}

		Iterator Erase(ConstIterator first, ConstIterator last)
		{
			assert(first >= begin() && "Iterator to insert is out of bounds.");
			assert(last < end() && "Inserting at past-the-end iterator.");

			Iterator iter = const_cast<Iterator>(first);

			if (first == last)
				return iter;

			Iterator iterLast = const_cast<Iterator>(last);
			std::move(iterLast, end(), iter);

			for (Iterator it = iter; it < iterLast; ++it)
				Pop();

			return iter;
		}

		Iterator Erase(ConstIterator it)
		{
			assert(it >= begin() && "Iterator to erase is out of bounds.");
			assert(it < end() && "Erasing at past-the-end iterator.");

			Iterator toErase = const_cast<Iterator>(it);
			std::move(toErase + 1, end(), toErase);
			Pop();

			return toErase;
		}

	private:
		void Realloc(UINT32 capacity)
		{
			Type* buffer = bs_allocN<Type>(capacity);

			if (mElements)
			{
				std::uninitialized_copy(
					std::make_move_iterator(begin()), 
					std::make_move_iterator(end()),
					buffer);

				for(auto& entry : *this)
					entry.~Type();

				bs_free(mElements);
			}

			mElements = buffer;
			mCapacity = capacity;
		}

		Type* mElements = nullptr;
		UINT32 mSize = 0;
		UINT32 mCapacity = 0;
	};

	/** @} */
}
