//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#ifdef __BORLANDC__
    #define __STD_ALGORITHM
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cmath>

#include <memory>

// STL containers
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <set>
#include <list>
#include <deque>
#include <queue>
#include <bitset>
#include <array>

#include <unordered_map>
#include <unordered_set>

// STL algorithms & functions
#include <algorithm>
#include <functional>
#include <limits>

// C++ Stream stuff
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

extern "C" {

#   include <sys/types.h>
#   include <sys/stat.h>

}

#if BS_PLATFORM == BS_PLATFORM_WIN32
#  undef min
#  undef max
#	if !defined(NOMINMAX) && defined(_MSC_VER)
#		define NOMINMAX // required to stop windows.h messing up std::min
#	endif
#  if defined( __MINGW32__ )
#    include <unistd.h>
#  endif
#endif

#if BS_PLATFORM == BS_PLATFORM_LINUX
extern "C" {

#   include <unistd.h>
#   include <dlfcn.h>

}
#endif

#if BS_PLATFORM == BS_PLATFORM_OSX
extern "C" {
#   include <unistd.h>
#   include <sys/param.h>
#   include <CoreFoundation/CoreFoundation.h>
}
#endif

namespace BansheeEngine
{
	/** @addtogroup Containers
	 *  @{
	 */

	/** Double ended queue. Allows for fast insertion and removal at both its beggining and end. */
	template <typename T, typename A = StdAlloc<T>> 
	using Deque = std::deque<T, A>;

	/** Dynamically sized array that stores element contigously. */
	template <typename T, typename A = StdAlloc<T>> 
	using Vector = std::vector<T, A>;

	/** Container that supports constant time insertion and removal, but without fast random access to elements. */
	template <typename T, typename A = StdAlloc<T>> 
	using List = std::list<T, A>;

	/** First-in, last-out data structure. */
	template <typename T, typename A = StdAlloc<T>> 
	using Stack = std::stack<T, std::deque<T, A>>;

	/** First-in, first-out data structure. */
	template <typename T, typename A = StdAlloc<T>>
	using Queue = std::queue<T, std::deque<T, A>>;

	/** An associative container containing an ordered set of elements. */
	template <typename T, typename P = std::less<T>, typename A = StdAlloc<T>> 
	using Set = std::set<T, P, A>;

	/** An associative container containing an ordered set of key-value pairs. */
	template <typename K, typename V, typename P = std::less<K>, typename A = StdAlloc<std::pair<const K, V>>> 
	using Map = std::map<K, V, P, A>;

	/** An associative container containing an ordered set of key-value pairs where multiple elements can have the same key. */
	template <typename K, typename V, typename P = std::less<K>, typename A = StdAlloc<std::pair<const K, V>>> 
	using MultiMap = std::multimap<K, V, P, A>;

	/** An associative container containing an unordered set of elements. Usually faster than Set for larger data sets. */
	template <typename T, typename H = std::hash<T>, typename C = std::equal_to<T>, typename A = StdAlloc<T>> 
	using UnorderedSet = std::unordered_set<T, H, C, A>;

	/** An associative container containing an ordered set of key-value pairs. Usually faster than Map for larger data sets. */
	template <typename K, typename V, typename H = std::hash<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>>> 
	using UnorderedMap = std::unordered_map<K, V, H, C, A>;

	/** 
	 * An associative container containing an ordered set of key-value pairs where multiple elements can have the same key.
	 * Usually faster than MultiMap for larger data sets.
	 */
	template <typename K, typename V, typename H = std::hash<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>>> 
	using UnorderedMultimap = std::unordered_multimap<K, V, H, C, A>;

	/** @} */

	/** @addtogroup Memory
	 *  @{
	 */

	/** 
	 * Smart pointer that retains shared ownership of an project through a pointer. The object is destroyed automatically 
	 * when the last shared pointer to the object is destroyed.
	 */
	template <typename T>
	using SPtr = std::shared_ptr<T>;

	/** 
	 * Smart pointer that retains shared ownership of an project through a pointer. Reference to the object must be unique.
	 * The object is destroyed automatically when the pointer to the object is destroyed.
	 */
	template <typename T, typename Alloc = GenAlloc>
	using UPtr = std::unique_ptr<T, decltype(&bs_delete<T, Alloc>)>;

	/** Create a new shared pointer using a custom allocator category. */
	template<class Type, class AllocCategory, class... Args> 
	SPtr<Type> bs_shared_ptr_new(Args &&... args)
	{
		return std::allocate_shared<Type>(StdAlloc<Type, AllocCategory>(), std::forward<Args>(args)...);
	}

	/** Create a new shared pointer using the default allocator category. */
	template<class Type, class... Args>
	SPtr<Type> bs_shared_ptr_new(Args &&... args)
	{
		return std::allocate_shared<Type>(StdAlloc<Type, GenAlloc>(), std::forward<Args>(args)...);
	}

	/**
	 * Create a new shared pointer from a previously constructed object.
	 * Pointer specific data will be allocated using the provided allocator category.
	 */
	template<class Type, class MainAlloc = GenAlloc, class PtrDataAlloc = GenAlloc>
	SPtr<Type> bs_shared_ptr(Type* data)
	{
		return SPtr<Type>(data, &bs_delete<Type, MainAlloc>, StdAlloc<Type, PtrDataAlloc>());
	}

	/**
	* Create a new unique pointer from a previously constructed object.
	* Pointer specific data will be allocated using the provided allocator category.
	*/
	template<class Type, class Alloc = GenAlloc>
	UPtr<Type, Alloc> bs_unique_ptr(Type* data)
	{
		return std::unique_ptr<Type, decltype(&bs_delete<Type, Alloc>)>(data, bs_delete<Type, Alloc>);
	}

	/** Create a new unique pointer using a custom allocator category. */
	template<class Type, class Alloc, class... Args>
	UPtr<Type> bs_unique_ptr_new(Args &&... args)
	{
		Type* rawPtr = bs_new<Type, Alloc>(std::forward<Args>(args)...);

		return bs_unique_ptr<Type, Alloc>(rawPtr);
	}

	/** Create a new unique pointer using the default allocator category. */
	template<class Type, class... Args>
	UPtr<Type> bs_unique_ptr_new(Args &&... args)
	{
		Type* rawPtr = bs_new<Type, GenAlloc>(std::forward<Args>(args)...);

		return bs_unique_ptr<Type, GenAlloc>(rawPtr);
	}

	/** @} */
}
