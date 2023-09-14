//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Provides a way to access elements of any array type (e.g. TArray, Vector, FrameVector, SmallVector). The view provides direct access to the array data.
	 * The caller must ensure the viewed array is not destroyed or reallocated while the view is active.
	 */
	template <class Type>
	class ArrayView final
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

		ArrayView() = default;
		~ArrayView() = default;

		ArrayView(const ArrayView& other) = default;
		ArrayView(ArrayView&& other) = default;

		ArrayView& operator=(const ArrayView& other) = default;
		ArrayView& operator=(ArrayView&& other) = default;

		ArrayView(ValueType* data, u64 size)
			: mData(data), mSize(size)
		{ }

		ArrayView(std::nullptr_t, u64)
			: ArrayView(nullptr, 0)
		{ }

		template<typename U>
		ArrayView(ArrayView<U>& other)
			: ArrayView(other.data(), other.size())
		{ }

		template<typename U>
		ArrayView(const ArrayView<U>& other)
			: ArrayView(other.data(), other.size())
		{ }

		template<typename U>
		ArrayView(ArrayView<U>&& other)
			: ArrayView(other.data(), other.size())
		{ }

		template<typename U, std::enable_if_t<!std::is_rvalue_reference_v<U&&>, i32> = 0>
		ArrayView(U&& other) : ArrayView(other.data(), other.size())
		{ }

		bool operator==(const ArrayView& other)
		{
			if(this->Size() != other.Size()) return false;
			return std::equal(this->Begin(), this->End(), other.Begin());
		}

		bool operator!=(const ArrayView& other)
		{
			return !(*this == other);
		}

		bool operator<(const ArrayView& other) const
		{
			return std::lexicographical_compare(Begin(), End(), other.Begin(), other.End());
		}

		bool operator>(const ArrayView& other) const
		{
			return other < *this;
		}

		bool operator<=(const ArrayView& other) const
		{
			return !(other < *this);
		}

		bool operator>=(const ArrayView& other) const
		{
			return !(*this < other);
		}

		Type& operator[](u64 index)
		{
			B3D_ASSERT(index < mSize && "Array index out-of-range.");
			return mData[index];
		}

		const Type& operator[](u64 index) const
		{
			B3D_ASSERT(index < mSize && "Array index out-of-range.");
			return mData[index];
		}

		bool IsEmpty() const { return mSize == 0; }

		Iterator Begin() { return mData; }

		Iterator End() { return mData + mSize; }

		ConstIterator Begin() const { return mData; }

		ConstIterator End() const { return mData + mSize; }

		ConstIterator Cbegin() const { return mData; }

		ConstIterator Cend() const { return mData + mSize; }

		ReverseIterator Rbegin() { return ReverseIterator(End()); }

		ReverseIterator Rend() { return ReverseIterator(Begin()); }

		ConstReverseIterator Rbegin() const { return ConstReverseIterator(End()); }

		ConstReverseIterator Rend() const { return ConstReverseIterator(Begin()); }

		ConstReverseIterator Crbegin() const { return ConstReverseIterator(End()); }

		ConstReverseIterator Crend() const { return ConstReverseIterator(Begin()); }

		u64 Size() const { return mSize; }

		Type* Data() { return mData; }

		const Type* Data() const { return mData; }

		Type& Front()
		{
			B3D_ASSERT(!IsEmpty());
			return mData[0];
		}

		Type& Back()
		{
			B3D_ASSERT(!IsEmpty());
			return mData[mSize - 1];
		}

		const Type& Front() const
		{
			B3D_ASSERT(!IsEmpty());
			return mData[0];
		}

		const Type& Back() const
		{
			B3D_ASSERT(!IsEmpty());
			return mData[mSize - 1];
		}

		bool Contains(const Type& element)
		{
			for(u64 i = 0; i < mSize; i++)
			{
				if(mData[i] == element)
					return true;
			}

			return false;
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

		Type* data() { return Data(); } // NOLINT
		const Type* data() const { return Data(); } // NOLINT

		Type& front() { return Front(); } // NOLINT
		const Type& front() const { return Front(); } // NOLINT

		Type& back() { return Back(); } // NOLINT
		const Type& back() const { return Back(); } // NOLINT

	private:
		Type* mData = nullptr;
		u64 mSize = 0;
	};

	/** @} */
} // namespace bs
