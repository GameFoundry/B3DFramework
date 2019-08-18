//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	*  @{
	*/

	/**
	 * RTTIPlainType for std::vector.
	 *
	 * @see		RTTIPlainType
	 */
	template<class T> struct RTTIPlainType<std::vector<T, StdAlloc<T>>>
	{
		enum { id = TID_Vector }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const std::vector<T, StdAlloc<T>>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.size();
			memcpy(memory, &numElements, sizeof(UINT32));
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			for (const auto& item : data)
			{
				UINT32 elementSize = rttiGetElemSize(item);
				RTTIPlainType<T>::toMemory(item, memory);

				memory += elementSize;
				size += elementSize;
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(std::vector<T, StdAlloc<T>>& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 numElements;
			memcpy(&numElements, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			data.clear();
			for (UINT32 i = 0; i < numElements; i++)
			{
				T element;
				UINT32 elementSize = RTTIPlainType<T>::fromMemory(element, memory);
				data.push_back(element);

				memory += elementSize;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const std::vector<T, StdAlloc<T>>& data)
		{
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (const auto& item : data)
				dataSize += rttiGetElemSize(item);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::list.
	 *
	 * @see		RTTIPlainType
	 */
	template<class T> struct RTTIPlainType<std::list<T, StdAlloc<T>>>
	{
		enum { id = TID_List }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const std::list<T, StdAlloc<T>>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.size();
			memcpy(memory, &numElements, sizeof(UINT32));
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			for (const auto& item : data)
			{
				UINT32 elementSize = rttiGetElemSize(item);
				RTTIPlainType<T>::toMemory(item, memory);

				memory += elementSize;
				size += elementSize;
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(std::list<T, StdAlloc<T>>& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 numElements;
			memcpy(&numElements, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			for (UINT32 i = 0; i < numElements; i++)
			{
				T element;
				UINT32 elementSize = RTTIPlainType<T>::fromMemory(element, memory);
				data.push_back(element);

				memory += elementSize;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const std::list<T, StdAlloc<T>>& data)
		{
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (const auto& item : data)
				dataSize += rttiGetElemSize(item);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::set.
	 *
	 * @see		RTTIPlainType
	 */
	template<class T> struct RTTIPlainType<std::set<T, std::less<T>, StdAlloc<T>>>
	{
		enum { id = TID_Set }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const std::set<T, std::less<T>, StdAlloc<T>>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.size();
			memcpy(memory, &numElements, sizeof(UINT32));
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			for (const auto& item : data)
			{
				UINT32 elementSize = rttiGetElemSize(item);
				RTTIPlainType<T>::toMemory(item, memory);

				memory += elementSize;
				size += elementSize;
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(std::set<T, std::less<T>, StdAlloc<T>>& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 numElements;
			memcpy(&numElements, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			for (UINT32 i = 0; i < numElements; i++)
			{
				T element;
				UINT32 elementSize = RTTIPlainType<T>::fromMemory(element, memory);
				data.insert(element);

				memory += elementSize;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const std::set<T, std::less<T>, StdAlloc<T>>& data)
		{
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (const auto& item : data)
				dataSize += rttiGetElemSize(item);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::map.
	 *
	 * @see		RTTIPlainType
	 */
	template<class Key, class Value> struct RTTIPlainType<std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>>>
	{
		enum { id = TID_Map }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.size();
			memcpy(memory, &numElements, sizeof(UINT32));
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			for (const auto& item : data)
			{
				UINT32 keySize = rttiGetElemSize(item.first);
				RTTIPlainType<Key>::toMemory(item.first, memory);

				memory += keySize;
				size += keySize;

				UINT32 valueSize = rttiGetElemSize(item.second);
				RTTIPlainType<Value>::toMemory(item.second, memory);

				memory += valueSize;
				size += valueSize;
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>>& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 numElements;
			memcpy(&numElements, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			for (UINT32 i = 0; i < numElements; i++)
			{
				Key key;
				UINT32 keySize = RTTIPlainType<Key>::fromMemory(key, memory);
				memory += keySize;

				Value value;
				UINT32 valueSize = RTTIPlainType<Value>::fromMemory(value, memory);
				memory += valueSize;

				data[key] = value;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>>& data)
		{
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (const auto& item : data)
			{
				dataSize += rttiGetElemSize(item.first);
				dataSize += rttiGetElemSize(item.second);
			}

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::unordered_map.
	 *
	 * @see		RTTIPlainType
	 */
	template<class Key, class Value>
	struct RTTIPlainType<std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, StdAlloc<std::pair<const Key, Value>>>>
	{
		enum { id = TID_UnorderedMap }; enum { hasDynamicSize = 1 };

		typedef std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, StdAlloc<std::pair<const Key, Value>>> MapType;

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const MapType& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.size();
			memcpy(memory, &numElements, sizeof(UINT32));
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			for (const auto& item : data)
			{
				UINT32 keySize = rttiGetElemSize(item.first);
				RTTIPlainType<Key>::toMemory(item.first, memory);

				memory += keySize;
				size += keySize;

				UINT32 valueSize = rttiGetElemSize(item.second);
				RTTIPlainType<Value>::toMemory(item.second, memory);

				memory += valueSize;
				size += valueSize;
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(MapType& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 numElements;
			memcpy(&numElements, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			for (UINT32 i = 0; i < numElements; i++)
			{
				Key key;
				UINT32 keySize = RTTIPlainType<Key>::fromMemory(key, memory);
				memory += keySize;

				Value value;
				UINT32 valueSize = RTTIPlainType<Value>::fromMemory(value, memory);
				memory += valueSize;

				data[key] = value;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const MapType& data)
		{
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (const auto& item : data)
			{
				dataSize += RTTIPlainType<Key>::getDynamicSize(item.first);
				dataSize += RTTIPlainType<Value>::getDynamicSize(item.second);
			}

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::unordered_set.
	 *
	 * @see		RTTIPlainType
	 */
	template<class Key>
	struct RTTIPlainType<std::unordered_set<Key, std::hash<Key>, std::equal_to<Key>, StdAlloc<Key>>>
	{
		enum { id = TID_UnorderedSet }; enum { hasDynamicSize = 1 };

		typedef std::unordered_set<Key, std::hash<Key>, std::equal_to<Key>, StdAlloc<Key>> MapType;

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const MapType& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.size();
			memcpy(memory, &numElements, sizeof(UINT32));
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			for (const auto& item : data)
			{
				UINT32 keySize = rttiGetElemSize(item);
				RTTIPlainType<Key>::toMemory(item, memory);

				memory += keySize;
				size += keySize;
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(MapType& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 numElements;
			memcpy(&numElements, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			for (UINT32 i = 0; i < numElements; i++)
			{
				Key key;
				UINT32 keySize = RTTIPlainType<Key>::fromMemory(key, memory);
				memory += keySize;

				data.insert(key);
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const MapType& data)
		{
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (const auto& item : data)
			{
				dataSize += rttiGetElemSize(item);
			}

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::pair.
	 *
	 * @see		RTTIPlainType
	 */
	template<class A, class B> struct RTTIPlainType<std::pair<A, B>>
	{
		enum { id = TID_Pair }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const std::pair<A, B>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 firstSize = rttiGetElemSize(data.first);
			RTTIPlainType<A>::toMemory(data.first, memory);

			memory += firstSize;
			size += firstSize;

			UINT32 secondSize = rttiGetElemSize(data.second);
			RTTIPlainType<B>::toMemory(data.second, memory);

			memory += secondSize;
			size += secondSize;

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(std::pair<A, B>& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 firstSize = RTTIPlainType<A>::fromMemory(data.first, memory);
			memory += firstSize;

			UINT32 secondSize = RTTIPlainType<B>::fromMemory(data.second, memory);
			memory += secondSize;

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const std::pair<A, B>& data)
		{
			UINT64 dataSize = sizeof(UINT32);
			dataSize += rttiGetElemSize(data.first);
			dataSize += rttiGetElemSize(data.second);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
