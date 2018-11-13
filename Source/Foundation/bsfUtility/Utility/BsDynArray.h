#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
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
			if (!other.empty())
				*this = other;
		}

		DynArray(DynArray<ValueType>&& other)
		{
			if (!other.empty())
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

					realloc(otherSize);
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

					realloc(otherSize);
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
			if (this->size() != other.size()) return false;
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

		bool empty() const { return mSize == 0; }

		Iterator begin() { return mElements; }
		Iterator end() { return mElements + mSize; }

		ConstIterator begin() const { return mElements; }
		ConstIterator end() const { return mElements + mSize; }

		ConstIterator cbegin() const { return mElements; }
		ConstIterator cend() const { return mElements + mSize; }

		ReverseIterator rbegin() { return ReverseIterator(end()); }
		ReverseIterator rend() { return ReverseIterator(begin()); }

		ConstReverseIterator rbegin() const { return ReverseIterator(end()); }
		ConstReverseIterator rend() const { return ReverseIterator(begin()); }

		ConstReverseIterator crbegin() const { return ReverseIterator(end()); }
		ConstReverseIterator crend() const { return ReverseIterator(begin()); }

		UINT32 size() const { return mSize; }
		UINT32 capacity() const { return mCapacity; }

		Type* data() { return mElements; }
		const Type* data() const { return mElements; }

		Type& front()
		{
			assert(!empty());
			return *mElements[0];
		}

		Type& back()
		{
			assert(!empty());
			return *mElements[mSize - 1];
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
			if (size() == capacity())
				realloc(std::max(1U, capacity() * 2));

			new (&mElements[mSize++]) Type(element);
		}

		void add(Type&& element)
		{
			if (size() == capacity())
				realloc(std::max(1U, capacity() * 2));

			new (&mElements[mSize++]) Type(std::move(element));
		}

		void pop()
		{
			assert(mSize > 0 && "Popping an empty array.");
			--mSize;
			mElements[mSize].~Type();
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

		void resize(UINT32 size, const Type& value = Type())
		{
			if (size > capacity())
				realloc(size);

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

		void reserve(UINT32 size)
		{
			if (size > capacity())
				realloc(size);
		}

		void shrink()
		{
			realloc(mSize);
		}

		void append(ConstIterator start, ConstIterator end)
		{
			const UINT32 count = (UINT32)std::distance(start, end);

			if ((size() + count) > capacity())
				realloc(size() + count);

			std::uninitialized_copy(start, end, this->end());
			mSize += count;
		}

		void append(UINT32 count, const Type& element)
		{
			if ((size() + count) > capacity())
				realloc(size() + count);

			std::uninitialized_fill_n(end(), count, element);
			mSize += count;
		}

		void append(std::initializer_list<Type> list)
		{
			append(list.begin(), list.end());
		}

		void swap(DynArray<ValueType>& other)
		{
			const UINT32 tmpSize = size();
			const UINT32 tmpCapacity = capacity();
			Type* tmp = data();

			mSize = other.size();
			mCapacity = other.capacity();
			mElements = other.data();

			other.mSize = tmpSize;
			other.mCapacity = tmpCapacity;
			other.mElements = tmp;
		}

		template <typename ...Args>
		void emplaceBack(Args&& ...args)
		{
			if (size() == capacity())
				realloc(std::max(1U, capacity() * 2));

			new (&mElements[mSize++]) Type(std::forward<Args>(args) ...);
		}

		template <typename ...Args>
		Iterator emplace(ConstIterator it, Args&&... args)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();

			if (size() == capacity())
				realloc(std::max(1U, capacity() * 2));

			new (&mElements[mSize++]) Type(std::forward<Args>(args) ...);
			std::rotate(begin() + offset, end() - 1, end());

			return begin() + offset;
		}

		Iterator insert(ConstIterator it, const ValueType& element)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();

			if (size() == capacity())
				realloc(std::max(1U, capacity() * 2));

			new (&mElements[mSize++]) Type(element);
			std::rotate(begin() + offset, end() - 1, end());

			return begin() + offset;
		}

		Iterator insert(ConstIterator it, ValueType&& element)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();

			if (size() == capacity())
				realloc(std::max(1U, capacity() * 2));

			new (&mElements[mSize++]) Type(std::move(element));
			std::rotate(begin() + offset, end() - 1, end());

			return begin() + offset;
		}

		Iterator insert(ConstIterator it, UINT32 n, const ValueType& element)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();
			Iterator iter = &mElements[offset];

			if (!n)
				return iter;

			if (size() + n > capacity())
				realloc((size() + n) * 2);

			UINT32 c = n;
			while (c--)
				new (&mElements[mSize++]) Type(element);

			std::rotate(begin() + offset, end() - n, end());

			return begin() + offset;
		}

		template <typename InputIt>
		typename std::enable_if<!std::is_integral<InputIt>::value, void>::type insert(ConstIterator it,
			InputIt first, InputIt last)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();
			UINT32 n = (UINT32)(last - first);

			if (size() + n > capacity())
				realloc((size() + n) * 2);

			while (first != last)
				new (&mElements[mSize++]) Type(*first++);

			std::rotate(begin() + offset, end() - n, end());
		}

		Iterator insert(ConstIterator it, std::initializer_list<ValueType> list)
		{
			Iterator iterc = const_cast<Iterator>(it);
			DifferenceType offset = iterc - begin();
			Iterator iter = &mElements[offset];
			UINT32 n = (UINT32)list.size();

			if (!n)
				return iter;

			if (size() + n > capacity())
				realloc((size() + n) * 2);

			for (auto& entry : list)
				new (&mElements[mSize++]) Type(entry);

			std::rotate(begin() + offset, end() - n, end());

			return iter;
		}

		Iterator erase(ConstIterator first, ConstIterator last)
		{
			assert(first >= begin() && "Iterator to insert is out of bounds.");
			assert(last < end() && "Inserting at past-the-end iterator.");

			Iterator iter = const_cast<Iterator>(first);

			if (first == last)
				return iter;

			Iterator iterLast = const_cast<Iterator>(last);
			std::move(iterLast, end(), iter);

			for (Iterator it = iter; it < iterLast; ++it)
				pop();

			return iter;
		}

		Iterator erase(ConstIterator it)
		{
			assert(it >= begin() && "Iterator to erase is out of bounds.");
			assert(it < end() && "Erasing at past-the-end iterator.");

			Iterator toErase = const_cast<Iterator>(it);
			std::move(toErase + 1, end(), toErase);
			pop();

			return toErase;
		}

	private:
		void realloc(UINT32 capacity)
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
}
