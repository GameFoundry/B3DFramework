//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"

namespace b3d
{
	struct RTTIFieldInfo;

	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	/**
	 * RTTIPlainType for std::vector.
	 *
	 * @see		RTTIPlainType
	 */
	template <class T>
	struct RTTIPlainType<std::vector<T, StdAlloc<T>>>
	{
		enum
		{
			id = TID_Vector
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const std::vector<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
					size += B3DRTTIWrite(item, stream);

				return size; });
		}

		static BitLength FromMemory(std::vector<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			data.clear();
			for(uint32_t i = 0; i < numElements; i++)
			{
				T element;
				B3DRTTIRead(element, stream);

				data.push_back(element);
			}

			return size;
		}

		static BitLength GetSize(const std::vector<T, StdAlloc<T>>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
				dataSize += B3DRTTISize(item);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::array.
	 *
	 * @see		RTTIPlainType
	 */
	template <class T, u32 N>
	struct RTTIPlainType<std::array<T, N>>
	{
		enum
		{
			id = TID_Array
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const std::array<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				size += B3DRTTIWrite(N, stream);

				for (const auto& item : data)
					size += B3DRTTIWrite(item, stream);

				return size; });
		}

		static BitLength FromMemory(std::array<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			numElements = std::min(numElements, N);

			for(uint32_t i = 0; i < numElements; i++)
			{
				T element;
				B3DRTTIRead(element, stream);

				data[i] = element;
			}

			return size;
		}

		static BitLength GetSize(const std::array<T, N>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
				dataSize += B3DRTTISize(item);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::list.
	 *
	 * @see		RTTIPlainType
	 */
	template <class T>
	struct RTTIPlainType<std::list<T, StdAlloc<T>>>
	{
		enum
		{
			id = TID_List
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const std::list<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
					size += B3DRTTIWrite(item, stream);

				return size; });
		}

		static BitLength FromMemory(std::list<T, StdAlloc<T>>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size = 0;
			B3DRTTIRead(size, stream);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			for(uint32_t i = 0; i < numElements; i++)
			{
				T element;
				B3DRTTIRead(element, stream);

				data.push_back(element);
			}

			return size;
		}

		static BitLength GetSize(const std::list<T, StdAlloc<T>>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
				dataSize += B3DRTTISize(item);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::set.
	 *
	 * @see		RTTIPlainType
	 */
	template <class T>
	struct RTTIPlainType<std::set<T, std::less<T>, StdAlloc<T>>>
	{
		enum
		{
			id = TID_Set
		};

		enum
		{
			hasDynamicSize = 1
		};

		typedef std::set<T, std::less<T>, StdAlloc<T>> SetType;

		static BitLength ToMemory(const SetType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
					size += B3DRTTIWrite(item, stream);

				return size; });
		}

		static BitLength FromMemory(SetType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			for(uint32_t i = 0; i < numElements; i++)
			{
				T element;
				B3DRTTIRead(element, stream);
				data.insert(element);
			}

			return size;
		}

		static BitLength GetSize(const SetType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
				dataSize += B3DRTTISize(item);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::map.
	 *
	 * @see		RTTIPlainType
	 */
	template <class Key, class Value>
	struct RTTIPlainType<std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>>>
	{
		enum
		{
			id = TID_Map
		};

		enum
		{
			hasDynamicSize = 1
		};

		typedef std::map<Key, Value, std::less<Key>, StdAlloc<std::pair<const Key, Value>>> MapType;

		static BitLength ToMemory(const MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
				{
					size += B3DRTTIWrite(item.first, stream);
					size += B3DRTTIWrite(item.second, stream);
				}

				return size; });
		}

		static BitLength FromMemory(MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			for(uint32_t i = 0; i < numElements; i++)
			{
				Key key;
				B3DRTTIRead(key, stream);

				Value value;
				B3DRTTIRead(value, stream);

				data[key] = value;
			}

			return size;
		}

		static BitLength GetSize(const MapType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
			{
				dataSize += B3DRTTISize(item.first);
				dataSize += B3DRTTISize(item.second);
			}

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::unordered_map.
	 *
	 * @see		RTTIPlainType
	 */
	template <class Key, class Value>
	struct RTTIPlainType<std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, StdAlloc<std::pair<const Key, Value>>>>
	{
		enum
		{
			id = TID_UnorderedMap
		};

		enum
		{
			hasDynamicSize = 1
		};

		typedef std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, StdAlloc<std::pair<const Key, Value>>> MapType;

		static BitLength ToMemory(const MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
				{
					size += B3DRTTIWrite(item.first, stream);
					size += B3DRTTIWrite(item.second, stream);
				}

				return size; });
		}

		static BitLength FromMemory(MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			for(uint32_t i = 0; i < numElements; i++)
			{
				Key key;
				B3DRTTIRead(key, stream);

				Value value;
				B3DRTTIRead(value, stream);

				data[key] = value;
			}

			return size;
		}

		static BitLength GetSize(const MapType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
			{
				dataSize += B3DRTTISize(item.first);
				dataSize += B3DRTTISize(item.second);
			}

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::unordered_set.
	 *
	 * @see		RTTIPlainType
	 */
	template <class Key>
	struct RTTIPlainType<std::unordered_set<Key, std::hash<Key>, std::equal_to<Key>, StdAlloc<Key>>>
	{
		enum
		{
			id = TID_UnorderedSet
		};

		enum
		{
			hasDynamicSize = 1
		};

		typedef std::unordered_set<Key, std::hash<Key>, std::equal_to<Key>, StdAlloc<Key>> MapType;

		static BitLength ToMemory(const MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
					size += B3DRTTIWrite(item, stream);

				return size; });
		}

		static BitLength FromMemory(MapType& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			for(uint32_t i = 0; i < numElements; i++)
			{
				Key key;
				B3DRTTIRead(key, stream);

				data.insert(key);
			}

			return size;
		}

		static BitLength GetSize(const MapType& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
				dataSize += B3DRTTISize(item);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::pair.
	 *
	 * @see		RTTIPlainType
	 */
	template <class A, class B>
	struct RTTIPlainType<std::pair<A, B>>
	{
		enum
		{
			id = TID_Pair
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const std::pair<A, B>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.first, stream);
				size += B3DRTTIWrite(data.second, stream);

				return size; });
		}

		static BitLength FromMemory(std::pair<A, B>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			B3DRTTIRead(data.first, stream);
			B3DRTTIRead(data.second, stream);

			return size;
		}

		static BitLength GetSize(const std::pair<A, B>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;
			dataSize += B3DRTTISize(data.first);
			dataSize += B3DRTTISize(data.second);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/**
	 * RTTIPlainType for std::optional.
	 *
	 * @see		RTTIPlainType
	 */
	template <class T>
	struct RTTIPlainType<std::optional<T>>
	{
		enum
		{
			id = TID_Optional
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const std::optional<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
			{
				const u8 hasValue = (u8)data.has_value();

				BitLength size = 0;
				size += B3DRTTIWrite(hasValue, stream);

				if(hasValue)
					size += B3DRTTIWrite(data.value(), stream);

				return size; });
		}

		static BitLength FromMemory(std::optional<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			u8 hasValue;
			B3DRTTIRead(hasValue, stream);

			if(hasValue)
			{
				T value;
				B3DRTTIRead(value, stream);

				data = value;
			}
			else
			{
				data.reset();
			}

			return size;
		}

		static BitLength GetSize(const std::optional<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(u8) + sizeof(T);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};


	/** @} */
	/** @endcond */
} // namespace b3d
