//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <BsEnumClassHash.h>

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Generates a new hash for the provided type using the default standard hasher and combines it with a previous hash. */
	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v)
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
	void bs_zero_out(T * arr, size_t count)
	{
		assert(arr != nullptr);
		assert(count != 0);
		std::memset(arr, 0, sizeof(T) * count);
	}

	/** @} */
}
