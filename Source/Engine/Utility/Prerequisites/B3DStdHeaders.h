//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#ifdef __BORLANDC__
#	define __STD_ALGORITHM
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

#include <optional>

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

// Time
#include <chrono>

// C limits
#include <float.h>

extern "C" {

#include <sys/types.h>
#include <sys/stat.h>
}

#if B3D_PLATFORM_WIN32
#	undef min
#	undef max
#	if !defined(NOMINMAX) && defined(_MSC_VER)
// NOLINTBEGIN
#		define NOMINMAX // required to stop windows.h messing up std::min
// NOLINTEND
#	endif
#	if defined(__MINGW32__)
#		include <unistd.h>
#	endif
#endif

#if B3D_PLATFORM_LINUX
extern "C" {

#	include <unistd.h>
#	include <dlfcn.h>
}
#endif

#if B3D_PLATFORM_MACOS
extern "C" {

#	include <unistd.h>
#	include <sys/param.h>
#	include <CoreFoundation/CoreFoundation.h>
}
#endif

namespace b3d
{
	/**
	 * Hash for enum types, to be used instead of std::hash<T> when T is an enum.
	 *
	 * Until C++14, std::hash<T> is not defined if T is a enum (see
	 * http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148).  But
	 * even with C++14, as of october 2016, std::hash for enums is not widely
	 * implemented by compilers, so here when T is a enum, we use EnumClassHash
	 * instead of std::hash. (For instance, in b3d::hash_combine(), or
	 * b3d::UnorderedMap.)
	 */
	struct EnumClassHash
	{
		template <typename T>
		constexpr std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
	};

	template <typename F>
	using Function = std::function<F>;

	/** @addtogroup Memory
	 *  @{
	 */

	/**
	 * Smart pointer that retains shared ownership of an project through a pointer. The object is destroyed automatically
	 * when the last shared pointer to the object is destroyed.
	 */
	template <typename T>
	using SPtr = std::shared_ptr<T>;

	/** Holds a reference to an object whose lifetime is managed by a SPtr, but doesn't increment the reference count. */
	template <typename T>
	using WeakSPtr = std::weak_ptr<T>;

	/** Contains an object of the specified type, or null. */
	template <typename T>
	using TOptional = std::optional<T>;

	/**
	 * Smart pointer that retains shared ownership of an project through a pointer. Reference to the object must be unique.
	 * The object is destroyed automatically when the pointer to the object is destroyed.
	 */
	template <typename T, typename AllocatorTag = DefaultAllocatorTag, typename Delete = Deleter<T, AllocatorTag>>
	using UPtr = std::unique_ptr<T, Delete>;

	/** @} */

	/** @addtogroup Metaprogramming
	 *  @{
	 */

	/**
	 * Checks if the class @p T has a SharedDeleter static method that accepts a non-const pointer to T. 	 */
	template <typename T, typename = void>
	struct B3DHasSharedDeleter : std::false_type {};

	template <typename T>
	struct B3DHasSharedDeleter<T, std::enable_if_t<std::is_same_v<decltype(T::template SharedDeleter<T, DefaultAllocatorTag>(std::declval<T*>())), void>>>
		: std::true_type {};

	// Checks is the provided type a shared pointer
	template <typename T>
	struct B3DIsSharedPointer : std::false_type {};

	template <typename T>
	struct B3DIsSharedPointer<SPtr<T>> : std::true_type {};

	// Checks is the provided type a weak shared pointer
	template <typename T>
	struct B3DIsWeakSharedPointer : std::false_type {};

	template <typename T>
	struct B3DIsWeakSharedPointer<WeakSPtr<T>> : std::true_type {};

	/** Checks does the provided type define iterator type. */
	template <typename T, typename = void>
	struct B3DHasIterator : std::false_type { };

	template <typename T>
	struct B3DHasIterator<T, std::void_t<typename T::iterator>> : std::true_type { };

	/** Checks does the provided type define const_iterator type. */
	template <typename T, typename = void>
	struct B3DHasConstIterator : std::false_type { };

	template <typename T>
	struct B3DHasConstIterator<T, std::void_t<typename T::const_iterator>> : std::true_type { };

	// Returns the underlying type if the provided type is a shared pointer, or itself otherwise
	template <typename T>
	struct B3DDecaySharedPointer
	{
		using value = T;
	};

	template <typename T>
	struct B3DDecaySharedPointer<SPtr<T>>
	{
		using value = typename SPtr<T>::element_type;
	};

	/** Checks is the provided type std::pair<K, V>. */
	template<typename T>
	struct B3DIsStdPair : std::false_type { };

	template<typename KeyType, typename ValueType>
	struct B3DIsStdPair<std::pair<KeyType, ValueType>> : std::true_type { };

	/** @} */

	/** @addtogroup Memory */

	// Checks if a specific template specialization exists
	template <class T, std::size_t = sizeof(T)>
	std::true_type B3DIsCompleteImplementation(T*);
	std::false_type B3DIsCompleteImplementation(...);

	template <class T>
	using B3DIsComplete = decltype(B3DIsCompleteImplementation(std::declval<T*>()));

	/**
	 * Create a new shared pointer using a custom allocator category.
	 * If class provides a `static SharedDeleter(Type*)` method it will be used as a shared pointer deleter, instead of the default.
	 */
	template <typename Type, typename AllocatorTag = DefaultAllocatorTag, typename... Args>
	SPtr<Type> B3DMakeShared(Args&&... args)
	{
		if constexpr(B3DHasSharedDeleter<Type>::value)
			return SPtr<Type>(B3DNew<Type, AllocatorTag>(std::forward<Args>(args)...), &Type::template SharedDeleter<Type, AllocatorTag>, StdAlloc<Type, AllocatorTag>());
		else
			return std::allocate_shared<Type>(StdAlloc<Type, AllocatorTag>(), std::forward<Args>(args)...);
	}

	/**
	 * Create a new shared pointer from a previously constructed object.
	 * Pointer specific data will be allocated using the provided allocator category.
	 * If class provides a `static SharedDeleter(Type*)` method it will be used as a shared pointer deleter, instead of the default. 
	 */
	template <typename Type, typename MainAllocatorTag = DefaultAllocatorTag, typename PointerDataAllocatorTag = DefaultAllocatorTag, typename Delete = Deleter<Type, MainAllocatorTag>>
	SPtr<Type> B3DMakeSharedFromExisting(Type* data, Delete del = Delete())
	{
		if constexpr(B3DHasSharedDeleter<Type>::value)
			return SPtr<Type>(data, &Type::template SharedDeleter<Type, MainAllocatorTag>, StdAlloc<Type, PointerDataAllocatorTag>());
		else
			return SPtr<Type>(data, std::move(del), StdAlloc<Type, PointerDataAllocatorTag>());
	}

	/**
	 * Create a new unique pointer from a previously constructed object.
	 * Pointer specific data will be allocated using the provided allocator category.
	 */
	template <typename Type, typename AllocatorTag = DefaultAllocatorTag, typename Delete = Deleter<Type, AllocatorTag>>
	UPtr<Type, AllocatorTag, Delete> B3DMakeUniqueFromExisting(Type* data, Delete del = Delete())
	{
		return std::unique_ptr<Type, Delete>(data, std::move(del));
	}

	/** Create a new unique pointer using a custom allocator category. */
	template <typename Type, typename AllocatorTag = DefaultAllocatorTag, typename Delete = Deleter<Type, AllocatorTag>, typename... Args>
	UPtr<Type, AllocatorTag, Delete> B3DMakeUnique(Args&&... args)
	{
		Type* rawPtr = B3DNew<Type, AllocatorTag>(std::forward<Args>(args)...);

		return B3DMakeUniqueFromExisting<Type, AllocatorTag, Delete>(rawPtr);
	}

	/** Returns true if the weak pointer is not assigned. Will return false if the pointer was assigned but has since expired. */
	template<typename Type>
	bool B3DIsWeakUnassigned(const WeakSPtr<Type>& pointer)
	{
		return !pointer.owner_before(WeakSPtr<Type>()) && !WeakSPtr<Type>().owner_before(pointer);
	}

	/**
	 * "Smart" pointer that is not smart. Does nothing but hold a pointer value. No memory management is performed at all.
	 * This class exists to make storing pointers in containers easier to manage, such as with non-member comparison
	 * operators.
	 */
	template <typename T>
	struct NativePtr
	{
		constexpr NativePtr(T* p)
			: mPtr(p) {}

		constexpr T& operator*() const { return *mPtr; }

		constexpr T* operator->() const { return mPtr; }

		constexpr T* Get() const { return mPtr; }

	private:
		T* mPtr = nullptr;
	};

	template <typename T>
	using NPtr = NativePtr<T>;

	template <typename L_T, typename R_T>
	constexpr bool operator<(const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		return lhs.get() < rhs.get();
	}

	template <typename L_T, typename R_T>
	constexpr bool operator>(const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		return lhs.get() > rhs.get();
	}

	template <typename L_T, typename R_T>
	constexpr bool operator<=(const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		return lhs.get() <= rhs.get();
	}

	template <typename L_T, typename R_T>
	constexpr bool operator>=(const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		return lhs.get() >= rhs.get();
	}

	template <typename L_T, typename R_T>
	constexpr bool operator==(const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template <typename L_T, typename R_T>
	constexpr bool operator!=(const NPtr<L_T>& lhs, const NPtr<R_T>& rhs)
	{
		return lhs.get() != rhs.get();
	}

	/** @} */

	/** @addtogroup Containers
	 *  @{
	 */

	/** Hasher that handles custom enums automatically. */
	template <typename Key>
	using HashType = typename std::conditional<std::is_enum<Key>::value, EnumClassHash, std::hash<Key>>::type;

	/** Double ended queue. Allows for fast insertion and removal at both its beginning and end. */
	template <typename T, typename A = StdAlloc<T>>
	using Deque = std::deque<T, A>;

	/** Dynamically sized array that stores elements contiguously. */
	template <typename T, typename A = StdAlloc<T>>
	using Vector = std::vector<T, A>;

	/** Constant array that stores elements contiguously. */
	template <typename ElementType, size_t Size>
	using Array = std::array<ElementType, Size>;

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

	/** @addtogroup Containers-Internal
	 *  @{
	 */

	/** Helper that provides hash and equality types for UnorderedSet and UnorderedMap containing shared pointer as a key. Key class must provide a `u64 GenerateHash()` method and an equality operator. */
	template<class T>
	struct TSharedUnorderedTypeHelper
	{
		struct Hash
		{
			u64 operator()(const SPtr<T>& value) const { return value ? value->GenerateHash() : 0; }
		};

		struct Equals
		{
			u64 operator()(const SPtr<T>& lhs, const SPtr<T>& rhs) const
			{
				if(lhs == nullptr && rhs == nullptr)
					return true;

				if(lhs == nullptr || rhs == nullptr)
					return false;

				return *lhs == *rhs;
			}
		};
	};

	/** Helper that provides hash type for UnorderedSet and UnorderedMap. Key class must provide a `u64 GenerateHash()` method and an equality operator. */
	template<class T>
	struct TUnorderedTypeHelper
	{
		struct Hash
		{
			u64 operator()(const T& value) const { return value.GenerateHash(); }
		};
	};

	/** @} */

	/** @addtogroup Containers
	 *  @{
	 */

	/** Unordered set containing @p SPtr<T> as the key. @p T must provide `u64 GenerateHash()` method and an equality operator. */
	template<class T>
	using TSharedUnorderedSet = UnorderedSet<SPtr<T>, typename TSharedUnorderedTypeHelper<T>::Hash, typename TSharedUnorderedTypeHelper<T>::Equals>;

	/** Unordered map containing @p SPtr<K> as the key and @p V as value. @p K must provide `u64 GenerateHash()` method and an equality operator. */
	template<class K, class V>
	using TSharedUnorderedMap = UnorderedMap<SPtr<K>, V, typename TSharedUnorderedTypeHelper<K>::Hash, typename TSharedUnorderedTypeHelper<K>::Equals>;

	/** Unordered set containing @p T as the key. @p T must provide `u64 GenerateHash()` method and an equality operator. */
	template<class T>
	using TUnorderedSet = UnorderedSet<T, typename TUnorderedTypeHelper<T>::Hash>;

	/** Unordered map containing @p K as the key and @p V as value. @p K must provide `u64 GenerateHash()` method and an equality operator. */
	template<class K, class V>
	using TUnorderedMap = UnorderedMap<K, V, typename TUnorderedTypeHelper<K>::Hash>;

	/** @} */

	/** @addtogroup Utility
	 *  @{
	 */

	using namespace std::chrono_literals;

	using Nanoseconds = std::chrono::nanoseconds;
	using Microseconds = std::chrono::microseconds;
	using Milliseconds = std::chrono::milliseconds;
	using Seconds = std::chrono::seconds;
	using Minutes = std::chrono::minutes;
	using Hours = std::chrono::hours;

	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

	/** @} */

} // namespace b3d
