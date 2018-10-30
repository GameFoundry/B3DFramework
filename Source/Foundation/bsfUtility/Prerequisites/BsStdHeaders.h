//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
#include <forward_list>
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
#include <iterator>

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

namespace bs
{
	/**
	 * Hash for enum types, to be used instead of std::hash<T> when T is an enum.
	 *
	 * Until C++14, std::hash<T> is not defined if T is a enum (see
	 * http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148).  But
	 * even with C++14, as of october 2016, std::hash for enums is not widely
	 * implemented by compilers, so here when T is a enum, we use EnumClassHash
	 * instead of std::hash. (For instance, in bs::hash_combine(), or
	 * bs::UnorderedMap.)
	 */
	struct EnumClassHash
	{
		template <typename T>
		constexpr std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
	};

	/** @addtogroup Containers
	 *  @{
	 */

	/** Hasher that handles custom enums automatically. */
	template <typename Key>
	using HashType = typename std::conditional<std::is_enum<Key>::value, EnumClassHash, std::hash<Key>>::type;

	/** Double ended queue. Allows for fast insertion and removal at both its beggining and end. */
	template <typename T, typename A = StdAlloc<T>>
	using Deque = std::deque<T, A>;

	/** Dynamically sized array that stores element contigously. */
	template <typename T, typename A = StdAlloc<T>>
	using Vector = std::vector<T, A>;

	/** 
	 * Container that supports constant time insertion and removal for elements with known locations, but without fast
	 * random access to elements. Internally implemented as a doubly linked list. Use ForwardList if you do not need 
	 * reverse iteration.
	 */
	template <typename T, typename A = StdAlloc<T>>
	using List = std::list<T, A>;

	/** 
	 * Container that supports constant time insertion and removal for elements with known locations, but without fast
	 * random access to elements. Internally implemented as a singly linked list that doesn't support reverse iteration.
	 */
	template <typename T, typename A = StdAlloc<T>>
	using ForwardList = std::forward_list<T, A>;

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

	/** An associative container containing an ordered set of elements where multiple elements can have the same key. */
	template <typename T, typename P = std::less<T>, typename A = StdAlloc<T>>
	using MultiSet = std::multiset<T, P, A>;

	/** An associative container containing an ordered set of key-value pairs where multiple elements can have the same key. */
	template <typename K, typename V, typename P = std::less<K>, typename A = StdAlloc<std::pair<const K, V>>>
	using MultiMap = std::multimap<K, V, P, A>;

	/** An associative container containing an unordered set of elements. Usually faster than Set for larger data sets. */
	template <typename T, typename H = HashType<T>, typename C = std::equal_to<T>, typename A = StdAlloc<T>>
	using UnorderedSet = std::unordered_set<T, H, C, A>;

	/** An associative container containing an ordered set of key-value pairs. Usually faster than Map for larger data sets. */
	template <typename K, typename V, typename H = HashType<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>>>
	using UnorderedMap = std::unordered_map<K, V, H, C, A>;

	/**
	 * An associative container containing an ordered set of key-value pairs where multiple elements can have the same key.
	 * Usually faster than MultiMap for larger data sets.
	 */
	template <typename K, typename V, typename H = HashType<K>, typename C = std::equal_to<K>, typename A = StdAlloc<std::pair<const K, V>>>
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
	template <typename T, typename Alloc = GenAlloc, typename Delete = decltype(&bs_delete<T, Alloc>)>
	using UPtr = std::unique_ptr<T, Delete>;

	/** Create a new shared pointer using a custom allocator category. */
	template<typename Type, typename AllocCategory = GenAlloc, typename... Args>
	SPtr<Type> bs_shared_ptr_new(Args &&... args)
	{
		return std::allocate_shared<Type>(StdAlloc<Type, AllocCategory>(), std::forward<Args>(args)...);
	}

	/**
	 * Create a new shared pointer from a previously constructed object.
	 * Pointer specific data will be allocated using the provided allocator category.
	 */
	template<typename Type, typename MainAlloc = GenAlloc, typename PtrDataAlloc = GenAlloc, typename Deleter = decltype(&bs_delete<Type, MainAlloc>)>
	SPtr<Type> bs_shared_ptr(Type* data, Deleter del = &bs_delete<Type, MainAlloc>)
	{
		return SPtr<Type>(data, std::move(del), StdAlloc<Type, PtrDataAlloc>());
	}

	/**
	 * Create a new unique pointer from a previously constructed object.
	 * Pointer specific data will be allocated using the provided allocator category.
	 */
	template<typename Type, typename Alloc = GenAlloc, typename Deleter = decltype(&bs_delete<Type, Alloc>)>
	UPtr<Type, Alloc, Deleter> bs_unique_ptr(Type* data, Deleter del = &bs_delete<Type, Alloc>)
	{
		return std::unique_ptr<Type, Deleter>(data, std::move(del));
	}

	/** Create a new unique pointer using a custom allocator category. */
	template<typename Type, typename Alloc = GenAlloc, typename Deleter = decltype(&bs_delete<Type, Alloc>), typename... Args>
	UPtr<Type, Alloc, Deleter> bs_unique_ptr_new(Args &&... args)
	{
		Type* rawPtr = bs_new<Type, Alloc>(std::forward<Args>(args)...);

		return bs_unique_ptr<Type, Alloc, Deleter>(rawPtr);
	}

	/**
	 * "Smart" pointer that is not smart. Does nothing but hold a pointer value. No memory management is performed at all.
	 * This class exists to make storing pointers in containers easier to manage, such as with non-member comparison
	 * operators.
	 */
	template<typename T>
	struct NativePtr
	{
		constexpr NativePtr(T* p) : mPtr(p) {}
		constexpr T& operator*() const { return *mPtr; }
		constexpr T* operator->() const { return mPtr; }
		constexpr T* get() const { return mPtr; }

	private:
		T* mPtr = nullptr;
	};

	template<typename T>
	using NPtr = NativePtr<T>;

	template<typename L_T, typename R_T>
	constexpr bool operator< (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		 return lhs.get() < rhs.get();
	}

	template<typename L_T, typename R_T>
	constexpr bool operator> (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		 return lhs.get() > rhs.get();
	}

	template<typename L_T, typename R_T>
	constexpr bool operator<= (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		 return lhs.get() <= rhs.get();
	}

	template<typename L_T, typename R_T>
	constexpr bool operator>= (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		 return lhs.get() >= rhs.get();
	}

	template<typename L_T, typename R_T>
	constexpr bool operator== (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		 return lhs.get() == rhs.get();
	}

	template<typename L_T, typename R_T>
	constexpr bool operator!= (const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		 return lhs.get() != rhs.get();
	}

	/** @} */
}
