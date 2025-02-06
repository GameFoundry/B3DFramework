//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Generates a new hash for the provided type using the default standard hasher and combines it with a previous hash. */
	template <class T>
	void B3DCombineHash(std::size_t& seed, const T& v)
	{
		using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

		HashType hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	/** Generates a hash for the provided type. Type must have a std::hash specialization. */
	template <class T>
	size_t B3DHash(const T& v)
	{
		using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

		HashType hasher;
		return hasher(v);
	}

	/** Generates an MD5 hash string for the provided source string. */
	String B3D_UTILITY_EXPORT Md5(const WString& source);

	/**	Generates an MD5 hash string for the provided source string. */
	String B3D_UTILITY_EXPORT Md5(const String& source);

	/** Sets contents of a struct to zero. */
	template <class T>
	void B3DZeroOut(T& s)
	{
		std::memset(&s, 0, sizeof(T));
	}

	/** Sets contents of a static array to zero. */
	template <class T, size_t N>
	void B3DZeroOut(T (&arr)[N])
	{
		std::memset(arr, 0, sizeof(T) * N);
	}

	/** Sets contents of a block of memory to zero. */
	template <class T>
	void B3DZeroOut(T* arr, size_t count)
	{
		B3D_ASSERT(arr != nullptr);
		std::memset(arr, 0, sizeof(T) * count);
	}

	/** Copies the contents of one array to another. Automatically accounts for array element size. */
	template <class T, size_t N>
	void B3DCopy(T (&dst)[N], T (&src)[N], size_t count)
	{
		std::memcpy(dst, src, sizeof(T) * count);
	}

	/** Copies the contents of one array to another. Automatically accounts for array element size. */
	template <class T>
	void B3DCopy(T* dst, T* src, size_t count)
	{
		std::memcpy(dst, src, sizeof(T) * count);
	}

	/** Returns the size of the provided static array. */
	template <class T, std::size_t N>
	constexpr size_t B3DSize(const T (&array)[N])
	{
		return N;
	}

	/**
	 * Erases the provided element from the container, but first swaps the element so its located at the end of the
	 * container, making the erase operation cheaper at the cost of an extra move operation. Doesn't preserve ordering
	 * within the element. Returns true if a swap occurred, or false if the element was already at the end of the container.
	 */
	template <class T, class A = StdAlloc<T>>
	bool B3DSwapAndErase(Vector<T, A>& container, const typename Vector<T, A>::iterator iter)
	{
		B3D_ASSERT(!container.empty());

		auto iterLast = container.end() - 1;

		bool swapped = false;
		if(iter != iterLast)
		{
			std::iter_swap(iter, iterLast);
			swapped = true;
		}

		container.pop_back();
		return swapped;
	}

	/** @copydoc B3DSwapAndErase(Vector<T, A>&, const typename Vector<T, A>::iterator) */
	template <class T, class A = StdAlloc<T>>
	bool B3DSwapAndErase(Vector<T, A>& container, u32 index)
	{
		B3D_ASSERT(!container.empty());

		auto it = container.begin() + index;
		auto iterLast = container.end() - 1;

		bool swapped = false;
		if(it != iterLast)
		{
			std::iter_swap(it, iterLast);
			swapped = true;
		}

		container.pop_back();
		return swapped;
	}

	/** Encapsulates width/height in a single structure. */
	template<class T>
	struct TSize2
	{
		T Width, Height;
		
		constexpr TSize2() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TSize2(BS_ZERO)
			: Width((T)0), Height((T)0)
		{}

		constexpr TSize2(T width, T height)
			: Width(width), Height(height)
		{}

		TSize2 operator+(const TSize2& rhs) const
		{
			return TSize2(Width + rhs.Width, Height + rhs.Height);
		}

		TSize2 operator-(const TSize2& rhs) const
		{
			return TSize2(Width - rhs.Width, Height - rhs.Height);
		}

		TSize2 operator*(const T rhs) const
		{
			return TSize2(Width * rhs, Height * rhs);
		}

		TSize2 operator*(const TSize2& rhs) const
		{
			return TSize2(Width * rhs.Width, Height * rhs.Height);
		}

		TSize2 operator/(const T rhs) const
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverseRHS = (T)1.0 / rhs;
			return TSize2(Width * inverseRHS, Height * inverseRHS);
		}

		TSize2 operator/(const TSize2& rhs) const
		{
			return TSize2(Width / rhs.Width, Height / rhs.Height);
		}

		const TSize2& operator+() const
		{
			return *this;
		}

		template<typename U = T, typename = std::enable_if_t<std::is_signed_v<U>, i32>>
		TSize2 operator-() const
		{
			return TSize2(-Width, -Height);
		}

		friend TSize2 operator*(T lhs, const TSize2& rhs)
		{
			return TSize2(lhs * rhs.Width, lhs * rhs.Height);
		}

		friend TSize2 operator/(T lhs, const TSize2& rhs)
		{
			return TSize2(lhs / rhs.Width, lhs / rhs.Height);
		}

		friend TSize2 operator+(TSize2& lhs, T rhs)
		{
			return TSize2(lhs.Width + rhs, lhs.Height + rhs);
		}

		friend TSize2 operator+(T lhs, const TSize2& rhs)
		{
			return TSize2(lhs + rhs.Width, lhs + rhs.Height);
		}

		friend TSize2 operator-(const TSize2& lhs, T rhs)
		{
			return TSize2(lhs.Width - rhs, lhs.Height - rhs);
		}

		friend TSize2 operator-(const T lhs, const TSize2& rhs)
		{
			return TSize2(lhs - rhs.Width, lhs - rhs.Height);
		}

		TSize2& operator+=(const TSize2& rhs)
		{
			Width += rhs.Width;
			Height += rhs.Height;

			return *this;
		}

		TSize2& operator+=(T rhs)
		{
			Width += rhs;
			Height += rhs;

			return *this;
		}

		TSize2& operator-=(const TSize2& rhs)
		{
			Width -= rhs.Width;
			Height -= rhs.Height;

			return *this;
		}

		TSize2& operator-=(T rhs)
		{
			Width -= rhs;
			Height -= rhs;

			return *this;
		}

		TSize2& operator*=(T rhs)
		{
			Width *= rhs;
			Height *= rhs;

			return *this;
		}

		TSize2& operator*=(const TSize2& rhs)
		{
			Width *= rhs.Width;
			Height *= rhs.Height;

			return *this;
		}

		TSize2& operator/=(T rhs)
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverseRHS = (T)1.0 / rhs;
			Width *= inverseRHS;
			Height *= inverseRHS;

			return *this;
		}

		TSize2& operator/=(const TSize2& rhs)
		{
			Width /= rhs.Width;
			Height /= rhs.Height;

			return *this;
		}

		bool operator==(const TSize2& rhs) const
		{
			return (Width == rhs.Width && Height == rhs.Height);
		}

		bool operator!=(const TSize2& rhs) const
		{
			return !operator==(rhs);
		}

		TSize2<float> ToFloat() const
		{
			return TSize2<float>((float)Width, (float)Height);
		}

		static TSize2<T> FromFloat(const TSize2<float>& other)
		{
			return TSize2<T>((T)other.Width, (T)other.Height);
		}

		static const TSize2 kZero;
	};

	template<> const TSize2<u32> TSize2<u32>::kZero{BsZero};
	template<> const TSize2<float> TSize2<float>::kZero{BsZero};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Size2)) TSize2<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Size2UI)) TSize2<u32>;

	/** Encapsulates width/height/depth in a single structure. */
	template<class T>
	struct TSize3
	{
		T Width, Height, Depth;
		
		constexpr TSize3() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TSize3(BS_ZERO)
			: Width((T)0), Height((T)0), Depth((T)0)
		{}

		constexpr TSize3(T width, T height, T depth)
			: Width(width), Height(height), Depth(depth)
		{}

		static const TSize3 kZero;
	};

	template<> const TSize3<u32> TSize3<u32>::kZero{BsZero};
	template<> const TSize3<float> TSize3<float>::kZero{BsZero};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Size3)) TSize3<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Size3UI)) TSize3<u32>;

	using Size2UI = TSize2<u32>;
	using Size3UI = TSize3<u32>;

	using Size2 = TSize2<float>;
	using Size3 = TSize3<float>;

	/**
	 * Represents a range of memory containing sequential elements. Elements may be optionally separated by a stride.
	 * Provides utility methods for easier reads and writes.
	 */
	struct DataRange
	{
		/**
		 * Constructs the data range.
		 *
		 * @param	data			Beginning of raw memory the data range is viewing.
		 * @param	elementCount	Number of elements in the range.
		 * @param	strideInBytes	Stride between elements. If 0, the stride will be automatically deduced from provided data type during access.
		 */
		DataRange(void* const data = nullptr, u64 elementCount = 0, u64 strideInBytes = 0)
			: Data((u8*)data), ElementCount(elementCount), ExplicitStrideInBytes(strideInBytes)
		{
		}

		template <typename T>
		const T& At(u32 index) const
		{
			B3D_ASSERT(index < ElementCount);

			const u64 strideInBytes = ExplicitStrideInBytes == 0 ? sizeof(T) : ExplicitStrideInBytes;
			const u64 offset = strideInBytes * index;

			return *(const T*)(Data + offset);
		}

		template <typename T>
		T& At(u32 index)
		{
			B3D_ASSERT(index < ElementCount);

			const u64 strideInBytes = ExplicitStrideInBytes == 0 ? sizeof(T) : ExplicitStrideInBytes;
			const u64 offset = strideInBytes * index;

			return *(T*)(Data + offset);
		}

		template <typename T>
		void Set(u32 index, const T& value)
		{
			T& destination = At<T>(index);
			destination = value;
		}

		u8* Data;
		size_t ElementCount;
		size_t ExplicitStrideInBytes;
	};

	/** Represents a range between two values. */
	template<typename T>
	struct TRange
	{
		constexpr TRange() = default;
		constexpr TRange(T center, T extent)
			: Center(center), Extent(extent)
		{ }

		T Center = (T)0.0;
		T Extent = (T)0.0;
	};

	using Range = TRange<float>;
	using RangeF = TRange<float>;
	using RangeD = TRange<double>;

	/** @} */
} // namespace bs

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TSize2<T>. */
template<class T>
struct hash<bs::TSize2<T>>
{
	size_t operator()(const bs::TSize2<T>& value) const
	{
		size_t hash = 0;
		bs::B3DCombineHash(hash, value.Width);
		bs::B3DCombineHash(hash, value.Height);

		return hash;
	}
};

/** Hash value generator for TSize3<T>. */
template<class T>
struct hash<bs::TSize3<T>>
{
	size_t operator()(const bs::TSize3<T>& value) const
	{
		size_t hash = 0;
		bs::B3DCombineHash(hash, value.Width);
		bs::B3DCombineHash(hash, value.Height);
		bs::B3DCombineHash(hash, value.Depth);

		return hash;
	}
};
} // namespace std

/** @endcond */
