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
	void hash_combine(std::size_t& seed, const T& v)
	{
		using HashType = typename std::conditional<std::is_enum<T>::value, EnumClassHash, std::hash<T>>::type;

		HashType hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}

	/** Generates an MD5 hash string for the provided source string. */
	String BS_UTILITY_EXPORT md5(const WString& source);

	/**	Generates an MD5 hash string for the provided source string. */
	String BS_UTILITY_EXPORT md5(const String& source);

	/** Sets contents of a struct to zero. */
	template<class T>
	void bs_zero_out(T& s)
	{
		std::memset(&s, 0, sizeof(T));
	}

	/** Sets contents of a static array to zero. */
	template<class T, size_t N>
	void bs_zero_out(T(&arr)[N])
	{
		std::memset(arr, 0, sizeof(T) * N);
	}

	/** Sets contents of a block of memory to zero. */
	template<class T>
	void bs_zero_out(T* arr, size_t count)
	{
		assert(arr != nullptr);
		std::memset(arr, 0, sizeof(T) * count);
	}

	/** Copies the contents of one array to another. Automatically accounts for array element size. */
	template<class T, size_t N>
	void bs_copy(T(&dst)[N], T(&src)[N], size_t count)
	{
		std::memcpy(dst, src, sizeof(T) * count);
	}

	/** Copies the contents of one array to another. Automatically accounts for array element size. */
	template<class T>
	void bs_copy(T* dst, T* src, size_t count)
	{
		std::memcpy(dst, src, sizeof(T) * count);
	}

	/** Returns the size of the provided static array. */
	template <class T, std::size_t N>
	constexpr size_t bs_size(const T (&array)[N])
	{
		return N;
	}

	/** 
	 * Erases the provided element from the container, but first swaps the element so its located at the end of the
	 * container, making the erase operation cheaper at the cost of an extra move operation. Doesn't preserve ordering
	 * within the element. Returns true if a swap occurred, or false if the element was already at the end of the container.
	 */
	template <class T, class A = StdAlloc<T>>
	bool bs_swap_and_erase(Vector<T, A>& container, const typename Vector<T, A>::iterator iter)
	{
		assert(!container.empty());

		auto iterLast = container.end() - 1;

		bool swapped = false;
		if(iter != iterLast)
		{
			std::swap(*iter, *iterLast);
			swapped = true;
		}

		container.pop_back();
		return swapped;
	}

	/** @} */
}
