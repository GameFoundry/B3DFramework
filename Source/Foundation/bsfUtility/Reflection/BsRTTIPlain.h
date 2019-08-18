//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Allocators/BsMemoryAllocator.h"
#include "Prerequisites/BsFwdDeclUtil.h"      // For TIDs
#include "Prerequisites/BsTypes.h"            // For UINT32

#include <limits>
#include <type_traits>          // For std::is_pod
#include <utility>              // For std::pair
#include <vector>

namespace bs
{
	/** @addtogroup Utility
	 *  @{
	 */

	/** @addtogroup RTTI
	 *  @{
	 */

	/**
	 * Template that you may specialize with a class if you want to provide simple serialization for it.
	 *
	 * Any type that uses the "plain" field in the RTTI system must specialize this class.
	 *
	 * @note
	 * Normally you will want to implement IReflectable interface if you want to provide serialization
	 * as that interface properly handles versioning, nested objects, pointer handling and more.
	 *
	 * @note
	 * This class is useful for types you can easily serialize using a memcpy (built-in types like int/float/etc), or
	 * types you cannot modify so they implement IReflectable interface (like std::string or std::vector).
	 *
	 * @see		RTTITypeBase
	 * @see		RTTIField
	 */
	template<class T>
	struct RTTIPlainType
	{
		static_assert(std::is_pod<T>::value,
			"Provided type isn't plain-old-data. You need to specialize RTTIPlainType template in order to serialize this type. "\
			" (Or call BS_ALLOW_MEMCPY_SERIALIZATION(type) macro if you are sure the type can be properly serialized using just memcpy.)");

		enum { id = 0 /**< Unique id for the serializable type. */ };
		enum { hasDynamicSize = 0 /**< 0 (Object has static size less than 255 bytes, for example int) or 1 (Dynamic size with no size restriction, for example string) */ };

		/** Serializes the provided object into the provided pre-allocated memory buffer. */
		static void toMemory(const T& data, char* memory)
		{
			memcpy(memory, &data, sizeof(T));
		}

		/**
		 *  Deserializes a previously allocated object from the provided memory buffer. Return the number of bytes read
		 *  from the memory buffer.
		 */
		static UINT32 fromMemory(T& data, char* memory)
		{
			memcpy(&data, memory, sizeof(T));
			return sizeof(T);
		}

		/** Returns the size of the provided object. (Works for both static and dynamic size types) */
		static UINT32 getDynamicSize(const T& data)
		{
			return sizeof(T);
		}
	};

	/**
	 * Helper method when serializing known data types that have valid
	 * RTTIPlainType specialization.
	 *
	 * Returns the size of the element. If elements serializable type is
	 * specialized with hasDynamicSize == true, the dynamic size is calculated,
	 * otherwise sizeof() is used.
	 */
	template<class ElemType>
	UINT32 rttiGetElemSize(const ElemType& data)
	{
		if(RTTIPlainType<ElemType>::hasDynamicSize == 1)
			return RTTIPlainType<ElemType>::getDynamicSize(data);
		else
			return sizeof(ElemType);
	}

	/**
	 * Helper method when serializing known data types that have valid
	 * RTTIPlainType specialization.
	 *
	 * Writes the specified data into memory, advances the memory pointer by the
	 * bytes written and returns pointer to new memory.
	 */
	template<class ElemType>
	char* rttiWriteElem(const ElemType& data, char* memory)
	{
		RTTIPlainType<ElemType>::toMemory(data, memory);

		return memory + rttiGetElemSize(data);
	}

	/**
	 * Helper method when serializing known data types that have valid
	 * RTTIPlainType specialization.
	 *
	 * Writes the specified data into memory, advances the memory pointer by the
	 * bytes written and returns pointer to new memory. Also increases the size
	 * value by the size of the written element.
	 */
	template<class ElemType>
	char* rttiWriteElem(const ElemType& data, char* memory, UINT32& size)
	{
		RTTIPlainType<ElemType>::toMemory(data, memory);

		UINT32 elemSize = rttiGetElemSize(data);
		size += elemSize;

		return memory + elemSize;
	}

	/**
	 * Helper method when serializing known data types that have valid
	 * RTTIPlainType specialization.
	 *
	 * Reads the specified data into memory, advances the memory pointer by the
	 * bytes read and returns pointer to new memory.
	 */
	template<class ElemType>
	char* rttiReadElem(ElemType& data, char* memory)
	{
		UINT32 size = RTTIPlainType<ElemType>::fromMemory(data, memory);

		return memory + size;
	}

	/**
	 * Helper method when serializing known data types that have valid
	 * RTTIPlainType specialization.
	 *
	 * Reads the specified data into memory, advances the memory pointer by the
	 * bytes read and returns pointer to new memory. Also increases the size
	 * value by the size of the read element.
	 */
	template<class ElemType>
	char* rttiReadElem(ElemType& data, char* memory, UINT32& size)
	{
		UINT32 elemSize = RTTIPlainType<ElemType>::fromMemory(data, memory);

		size += elemSize;
		return memory + elemSize;
	}

	/** Helper for checking for existance of rttiEnumFields method on a class. */
	template <class T>
	struct has_rttiEnumFields
	{
		struct dummy {};

		template <typename C, typename P>
		static auto test(P* p) -> decltype(std::declval<C>().rttiEnumFields(*p), std::true_type());

		template <typename, typename>
		static std::false_type test(...);

		typedef decltype(test<T, dummy>(nullptr)) type;
		static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
	};

	/**
	 * Notify the RTTI system that the specified type may be serialized just by using a memcpy.
	 *
	 * @note	Internally this creates a basic RTTIPlainType<T> specialization for the type.
	 *
	 * @see		RTTIPlainType<T>
	 */
#define BS_ALLOW_MEMCPY_SERIALIZATION(type)								\
	static_assert (std::is_trivially_copyable<type>()==true,			\
						#type " is not trivially copyable");			\
	template<> struct RTTIPlainType<type>								\
	{	enum { id=0 }; enum { hasDynamicSize = 0 };						\
		static void toMemory(const type& data, char* memory)			\
		{ memcpy(memory, &data, sizeof(type)); }						\
		static UINT32 fromMemory(type& data, char* memory)				\
		{ memcpy(&data, memory, sizeof(type)); return sizeof(type); }	\
		static UINT32 getDynamicSize(const type& data)					\
		{ return sizeof(type); }										\
	};

	/** @} */
	/** @} */
}
